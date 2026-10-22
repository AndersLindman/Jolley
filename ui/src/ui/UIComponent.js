import { Theme } from '../theme.js';
import { bus } from '../eventBus.js';
import { paramStore } from '../core/ParameterStore.js';

export class UIComponent {
    constructor() {
        this.children = [];
        this.parent = null;
        this.isRoot = false;

        this.layout = {
            x: 0,
            y: 0,
            width: 0,
            height: 0
        };

        this.canvas = {
            width: 0,
            height: 0,
            dpr: 1
        };

        this.isDirty = true;
    }

    add(child) {
        child.parent = this; // Establish parent link automatically
        this.children.push(child);
        return child;
    }

    // Depth-first tree traversal helper
    traverse(callback) {
        callback(this);
        for (const child of this.children) {
            child.traverse(callback);
        }
    }

    // Mark this component and its ancestors dirty so renders trigger correctly
    markDirty() {
        this.isDirty = true;
        for (const child of this.children) {
            child.markDirty();
        }
    }

    // Mark single component dirty and notify parent chain
    setDirty() {
        this.isDirty = true;
        let p = this.parent;
        while (p) {
            p.isDirty = true;
            p = p.parent;
        }
    }

    resize(width, height, ctx = null) {
        const dpr = window.devicePixelRatio || 1;

        if (ctx) {
            const targetW = Math.round(width * dpr);
            const targetH = Math.round(height * dpr);

            // Only update canvas buffer dimensions if they actually changed
            if (ctx.canvas.width !== targetW || ctx.canvas.height !== targetH) {
                ctx.canvas.width = targetW;
                ctx.canvas.height = targetH;
            }
        }

        this.canvas.width = width;
        this.canvas.height = height;
        this.canvas.dpr = dpr;

        this.computeLayout(0, 0, width, height, width);
        this.markDirty(); // Resizing marks all children dirty
    }

    computeLayout(x, y, width, height, rootWidth) {
        this.layout = {
            x,
            y,
            width,
            height
        };

        this.rootWidth = rootWidth;

        for (const child of this.children) {
            child.computeLayout(
                0,
                0,
                width,
                height,
                rootWidth
            );
        }
    }

    pixels(percentage) {
        return (this.rootWidth * percentage) * 0.01;
    }

    scale(pixels) {
        return (this.rootWidth * 0.001) * pixels;
    }

    drawText(ctx, label, x, y, {
        size = Theme.typography.labelSize,
        color = Theme.colors.textPrimary,
        align = 'center',
        baseline = 'middle'
    } = {}) {
        ctx.fillStyle = color;
        ctx.font = `${Theme.typography.weightMedium} ${this.scale(size)}px ${Theme.typography.fontFamily}`;
        ctx.textAlign = align;
        ctx.textBaseline = baseline;
        ctx.fillText(label, x, y);
    }

    // Calculates absolute canvas coordinates by walking up the parent chain
    getAbsoluteBounds() {
        let x = 0;
        let y = 0;
        let curr = this;

        while (curr && !curr.isRoot) {
            x += curr.layout?.x || 0;
            y += curr.layout?.y || 0;
            curr = curr.parent;
        }

        const w = this.layout.width || 0;
        const h = this.layout.height || 0;

        return {
            x,
            y,
            w,
            h,
            width: w,
            height: h
        };
    }

    requestRender() {
        this.isDirty = true;
        bus.emit('ui:requestRender');
    }

    paint(ctx) {
        ctx.save();

        if (this.isRoot) {
            const { width, height, dpr } = this.canvas;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

            // Fill root background only on full resets (e.g., resize)
            if (this.isDirty) {
                ctx.fillStyle = Theme.colors.background;
                ctx.fillRect(0, 0, width, height);
                this.isDirty = false;
            }
        } else {
            // Translate context to local component space
            ctx.translate(this.layout.x, this.layout.y);

            // Render ONLY this component's graphics if dirty
            if (this.isDirty) {
                this.drawSelf(ctx);
                this.isDirty = false;
            }
        }

        // Always pass rendering down to children
        for (const child of this.children) {
            child.paint(ctx);
        }

        ctx.restore();
    }

    // 2. Component Visual Representation (Override in subclasses like Knob, Panel, etc.)
    drawSelf(ctx) {
        // Base UIComponent renders no visual graphics by default
    }

    // Converts absolute canvas mouse coords into local component space
    canvasToLocal(canvasX, canvasY) {
        const bounds = this.getAbsoluteBounds();
        return {
            localX: canvasX - bounds.x,
            localY: canvasY - bounds.y
        };
    }

    hitTest(x, y) {
        // 1. Check if (x, y) fall within bounds [0, 0, width, height]
        if (x < 0 || x > this.layout.width || y < 0 || y > this.layout.height) {
            return null;
        }

        // 2. Check children in reverse order (topmost / last-drawn first)
        for (let i = this.children.length - 1; i >= 0; i--) {
            const child = this.children[i];

            const childX = x - child.layout.x;
            const childY = y - child.layout.y;

            const hit = child.hitTest(childX, childY);
            if (hit) return hit;
        }

        // 3. If no child matched, return this
        return this;
    }

    handlePointerDown(canvasX, canvasY, event) {}
    handlePointerMove(canvasX, canvasY, event) {}
    handlePointerUp(canvasX, canvasY, event) {}
    handleDoubleClick(x, y, event) {
        if (this.id) {
            paramStore.resetToDefault(this.id);
        }
    }
}