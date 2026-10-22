// src/ui/SynthApp.js

import { UIComponent } from './UIComponent.js';
import { GridContainer } from './GridContainer.js';
import { HeaderPanel } from '../panels/HeaderPanel.js';
import { OscPanel } from '../panels/OscPanel.js';
import { FilterPanel } from '../panels/FilterPanel.js';
import { AdsrPanel } from '../panels/AdsrPanel.js';
import { popupManager } from '../core/PopupManager.js';
import { bus } from '../eventBus.js';

export class SynthApp {
    constructor(canvas, wrapper) {
        this.canvas = canvas;
        this.wrapper = wrapper;
        this.ctx = canvas.getContext('2d');

        // Root UI hierarchy
        this.rootSynth = new UIComponent();
        this.rootSynth.isRoot = true;

        // Render Scheduler state
        this.renderPending = false;

        // Drag & Pointer state
        this.activeDragNode = null;
        this.activePointerId = null;

        // Resize state
        this.resizePending = false;
        this.pendingWidth = 0;
        this.pendingHeight = 0;

        this.initLayout();
        this.bindEvents();
        this.setupResizeObserver();

        bus.on('ui:requestRender', () => this.requestRender());
    }

    initLayout() {
        const mainGrid = new GridContainer({
            cols: 1,
            rows: 4,
            padding: 0,
            gap: 0,
            weightsY: [0.055, 0.37, 0.31]
        });

        this.rootSynth.add(mainGrid);
        mainGrid.add(new HeaderPanel());
        mainGrid.add(new OscPanel());
        mainGrid.add(new FilterPanel());
        mainGrid.add(new AdsrPanel());
    }

    requestRender() {
        if (!this.renderPending) {
            this.renderPending = true;
            requestAnimationFrame(() => this.renderNow());
        }
    }

    renderNow() {
        this.renderPending = false; // Reset flag so future renders can be scheduled!

        // 1. Repaint tree nodes into canvas buffer
        this.rootSynth.paint(this.ctx);

        // 2. Paint active overlays
        const dpr = this.rootSynth.canvas?.dpr || window.devicePixelRatio || 1;
        popupManager.paintOverlay(this.ctx, dpr);
    }

    setupResizeObserver() {
        this.resizeObserver = new ResizeObserver((entries) => {
            const { width, height } = entries[0].contentRect;
            if (width === 0 || height === 0) return;

            this.pendingWidth = width;
            this.pendingHeight = height;

            if (this.resizePending) return;
            this.resizePending = true;

            requestAnimationFrame(() => {
                this.resizePending = false;
                this.rootSynth.resize(this.pendingWidth, this.pendingHeight, this.ctx);
                this.renderNow();
            });
        });

        this.resizeObserver.observe(this.wrapper);

        // Initial layout pass
        const initialRect = this.wrapper.getBoundingClientRect();
        if (initialRect.width > 0 && initialRect.height > 0) {
            this.rootSynth.resize(initialRect.width, initialRect.height, this.ctx);
            this.renderNow();
        }
    }

    getCanvasCoords(event) {
        const rect = this.canvas.getBoundingClientRect();
        const scaleX = this.rootSynth.canvas ? this.rootSynth.canvas.width / rect.width : this.canvas.width / rect.width;
        const scaleY = this.rootSynth.canvas ? this.rootSynth.canvas.height / rect.height : this.canvas.height / rect.height;
        return {
            x: (event.clientX - rect.left) * scaleX,
            y: (event.clientY - rect.top) * scaleY
        };
    }

    bindEvents() {
        this.canvas.addEventListener('pointerdown', (event) => {
            const { x, y } = this.getCanvasCoords(event);

            if (popupManager.activePopup) {
                popupManager.handlePointerDown(x, y, event);
                this.requestRender();
                return;
            }

            const node = this.rootSynth.hitTest(x, y);
            if (!node) return;

            this.activeDragNode = node;
            this.activePointerId = event.pointerId;

            this.canvas.setPointerCapture(event.pointerId);
            node.handlePointerDown?.(x, y, event);
            this.requestRender();
        });

        this.canvas.addEventListener('pointermove', (event) => {
            const { x, y } = this.getCanvasCoords(event);

            if (popupManager.activePopup) {
                popupManager.handlePointerMove(x, y, event);
                this.requestRender();
                return;
            }

            if (this.activeDragNode) {
                this.activeDragNode.handlePointerMove?.(x, y, event);
                this.requestRender();
            }
        });

        this.canvas.addEventListener('pointerup', (event) => {
            if (!this.activeDragNode) return;

            const { x, y } = this.getCanvasCoords(event);
            this.activeDragNode.handlePointerUp?.(x, y, event);

            if (this.activePointerId !== null) {
                try {
                    this.canvas.releasePointerCapture(this.activePointerId);
                } catch (e) {
                    // Ignore if already released
                }
            }

            this.activeDragNode = null;
            this.activePointerId = null;
            this.requestRender();
        });

        this.canvas.addEventListener('dblclick', (event) => {
            const { x, y } = this.getCanvasCoords(event);

            if (popupManager.activePopup) {
                // Optional: handle popup dblclick if needed, or skip
                return;
            }

            const node = this.rootSynth.hitTest(x, y);
            if (!node) return;

            node.handleDoubleClick?.(x, y, event);
            this.requestRender();
        });
    }

    destroy() {
        this.resizeObserver?.disconnect();
    }
}