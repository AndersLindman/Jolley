// src/ui/PresetDropdown.js

import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { presetManager } from '../core/PresetManager.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from "../core/paramIds.js";
import { bus } from '../eventBus.js';
import { popupManager } from '../core/PopupManager.js';

export class PresetDropdown extends UIComponent {
    constructor(id, label = 'Preset') {
        super();

        this.id = id;
        this.label = label;
        this.factoryNames = presetManager.getFactoryNames();
        this.isOpen = false;
        this.hoveredIndex = -1;

        // Initialize the cached normalized value from ParameterStore.
        try {
            this.value = paramStore.getNormalized(PARAM_IDS.PRESET_INDEX);
        } catch (e) {
            this.value = 0;
        }

        this.value = this.clampValue(this.value);
        this.updatePresetName();

        // Listen to parameter changes from the DAW, host, or UI.
        this.unsubParam = paramStore.subscribe((id, normValue) => {

            if (id !== PARAM_IDS.PRESET_INDEX) return;

            const previousName = this.currentPresetName;

            this.value = this.clampValue(normValue);
            this.updatePresetName();

            // Load a factory preset only when the selected preset changes.
            if (
                this.currentPresetName !== previousName &&
                this.factoryNames.includes(this.currentPresetName)
            ) {
                presetManager.loadFactory(this.currentPresetName);
            }

            this.isDirty = true;
            this.requestRender();
        });

        // Synchronize when a preset is loaded externally.
        this.unsubBus = bus.on('preset:loaded', (presetData) => {
            const presetName =
                typeof presetData === 'string'
                    ? presetData
                    : presetData?.name;

            if (!presetName) return;

            const index = this.factoryNames.indexOf(presetName);

            // Ignore names that are not factory presets.
            if (index === -1) return;

            // Update the cached value and derived display name.
            this.value = this.factoryNames.length > 1
                ? index / (this.factoryNames.length - 1)
                : 0;

            this.updatePresetName();

            // Synchronize ParameterStore with the loaded preset.
            // This also allows the DAW to retain the selected index.
            paramStore.setNormalized(
                PARAM_IDS.PRESET_INDEX,
                this.value,
                'ui',
                true
            );

            this.isDirty = true;
            this.requestRender();
        });
    }

    // Clamp a normalized parameter value to [0, 1].
    clampValue(value) {
        const numericValue = Number(value);

        if (!Number.isFinite(numericValue)) {
            return 0;
        }

        return Math.max(0, Math.min(1, numericValue));
    }

    // Derive the displayed preset name from the normalized value.
    updatePresetName() {
        const index = Math.round(this.value * (this.factoryNames.length - 1));
        this.currentPresetName = this.factoryNames[index];
    }

    // Update the selected preset and synchronize ParameterStore.
    selectPreset(index) {
        if (index < 0 || index >= this.factoryNames.length) {
            return;
        }

        const selectedName = this.factoryNames[index];

        const norm = this.factoryNames.length > 1
            ? index / (this.factoryNames.length - 1)
            : 0;

        // Update local UI state immediately.
        this.value = norm;
        this.updatePresetName();

        // Update the host-backed parameter.
        paramStore.beginEdit(PARAM_IDS.PRESET_INDEX);

        try {
            paramStore.setNormalized(
                PARAM_IDS.PRESET_INDEX,
                this.value,
                'ui',
                true
            );
        } finally {
            paramStore.endEdit(PARAM_IDS.PRESET_INDEX);
        }

        // Explicitly load the selected factory preset.
        presetManager.loadFactory(selectedName);

        this.isDirty = true;
        this.requestRender();
    }

    // Forces all underlying panels to repaint when popup opens/closes.
    invalidateFullTree() {
        let curr = this;

        while (curr.parent) {
            curr = curr.parent;
        }

        if (curr.markDirty) {
            curr.markDirty();
        } else {
            curr.isDirty = true;
        }

        this.requestRender();
    }

    // Computes responsive metrics.
    getMetrics(isOverlay = false) {
        let x, y, w, h;

        if (isOverlay) {
            const bounds = this.getAbsoluteBounds();
            x = bounds.x;
            y = bounds.y;
            w = bounds.w;
            h = bounds.h;
        } else {
            x = 0;
            y = 0;
            w = this.layout.width;
            h = this.layout.height;
        }

        const fontSize = Math.max(9, h * 0.38);
        const itemHeight = Math.max(20, h * 0.85);
        const paddingX = Math.max(6, h * 0.25);
        const chevronOffset = Math.max(10, h * 0.45);
        const chevronSize = Math.max(3, h * 0.12);
        const menuWidth = Math.max(w, h * 4.5);
        const menuHeight = this.factoryNames.length * itemHeight;

        return {
            x, y, w, h,
            fontSize,
            itemHeight,
            paddingX,
            chevronOffset,
            chevronSize,
            menuWidth,
            menuHeight
        };
    }

    handlePointerDown(canvasX, canvasY, event) {
        const { localX, localY } =
            this.canvasToLocal(canvasX, canvasY);

        const { w, h, itemHeight, menuWidth, menuHeight } =
            this.getMetrics(false);

        if (!this.isOpen) {
            // Clicked header box: open the menu.
            if (
                localX >= 0 && localX <= w &&
                localY >= 0 && localY <= h
            ) {
                this.isOpen = true;
                popupManager.setPopup(this);
                this.isDirty = true;
                this.invalidateFullTree();
                event.stopPropagation?.();
            }
        } else {
            // Clicked header box again: close the menu.
            if (
                localX >= 0 && localX <= w &&
                localY >= 0 && localY <= h
            ) {
                this.isOpen = false;
                popupManager.close();
                this.isDirty = true;
                this.invalidateFullTree();
                return;
            }

            // Clicked a menu item.
            if (
                localX >= 0 && localX <= menuWidth &&
                localY >= h && localY <= h + menuHeight
            ) {
                const index = Math.floor(
                    (localY - h) / itemHeight
                );

                this.selectPreset(index);

                this.isOpen = false;
                popupManager.close();
                this.isDirty = true;
                this.invalidateFullTree();
            } else {
                // Clicked outside: close the menu.
                this.isOpen = false;
                popupManager.close();
                this.isDirty = true;
                this.invalidateFullTree();
            }
        }
    }

    handlePointerMove(canvasX, canvasY, event) {
        if (!this.isOpen) return;

        const { localX, localY } =
            this.canvasToLocal(canvasX, canvasY);

        const { h, itemHeight, menuWidth, menuHeight } =
            this.getMetrics(false);

        if (
            localX >= 0 && localX <= menuWidth &&
            localY >= h && localY <= h + menuHeight
        ) {
            const newHover = Math.floor(
                (localY - h) / itemHeight
            );

            if (
                newHover !== this.hoveredIndex &&
                newHover >= 0 &&
                newHover < this.factoryNames.length
            ) {
                this.hoveredIndex = newHover;
                this.isDirty = true;
                this.requestRender();
            }
        } else if (this.hoveredIndex !== -1) {
            this.hoveredIndex = -1;
            this.isDirty = true;
            this.requestRender();
        }
    }

    drawSelf(ctx) {
        const {
            x, y, w, h, fontSize, paddingX,
            chevronOffset, chevronSize
        } = this.getMetrics(false);

        // Header box.
        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(x, y, w, h);

        ctx.strokeStyle = Theme.colors.panelBorder;
        ctx.lineWidth = 1;
        ctx.strokeRect(x, y, w, h);

        // Label.
        ctx.fillStyle = Theme.colors.textSecondary;
        ctx.font =
            `${Theme.typography.weightMedium} ` +
            `${this.scale(fontSize)}px ` +
            `${Theme.typography.fontFamily}`;

        ctx.textAlign = 'left';
        ctx.textBaseline = 'middle';

        ctx.fillText(
            this.label,
            x + paddingX,
            y + h / 2
        );

        ctx.fillStyle = Theme.colors.textPrimary;
        ctx.textAlign = 'right';

        ctx.fillText(
            this.currentPresetName,
            x + w - chevronOffset - paddingX / 2,
            y + h / 2
        );

        // Chevron.
        ctx.strokeStyle = Theme.colors.textSecondary;
        ctx.lineWidth = Math.max(1, h * 0.04);
        ctx.beginPath();

        const cx = x + w - chevronOffset / 2;
        const cy = y + h / 2;

        if (this.isOpen) {
            ctx.moveTo(
                cx - chevronSize,
                cy + chevronSize / 2
            );
            ctx.lineTo(
                cx,
                cy - chevronSize / 2
            );
            ctx.lineTo(
                cx + chevronSize,
                cy + chevronSize / 2
            );
        } else {
            ctx.moveTo(
                cx - chevronSize,
                cy - chevronSize / 2
            );
            ctx.lineTo(
                cx,
                cy + chevronSize / 2
            );
            ctx.lineTo(
                cx + chevronSize,
                cy - chevronSize / 2
            );
        }

        ctx.stroke();
    }

    paintOverlay(ctx) {
        if (!this.isOpen) return;

        const {
            x, y, fontSize, itemHeight, paddingX,
            menuWidth, menuHeight, h
        } = this.getMetrics(true);

        const menuX = x;
        const menuY = y + h;

        ctx.save();

        // Menu background.
        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(menuX, menuY, menuWidth, menuHeight);

        ctx.strokeStyle = Theme.colors.panelBorder;
        ctx.lineWidth = 1;
        ctx.strokeRect(menuX, menuY, menuWidth, menuHeight);

        // Menu items.
        ctx.font =
            `${Theme.typography.weightMedium} ` +
            `${this.scale(fontSize)}px ` +
            `${Theme.typography.fontFamily}`;

        for (let i = 0; i < this.factoryNames.length; i++) {
            const name = this.factoryNames[i];
            const itemY = menuY + i * itemHeight;

            if (i === this.hoveredIndex) {
                ctx.fillStyle = 'rgba(255, 255, 255, 0.2)';
                ctx.fillRect(menuX, itemY, menuWidth, itemHeight);
            } else if (name === this.currentPresetName) {
                ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
                ctx.fillRect(menuX, itemY, menuWidth, itemHeight);
            }

            ctx.fillStyle = Theme.colors.textPrimary || '#ffffff';
            ctx.textAlign = 'left';
            ctx.textBaseline = 'middle';

            ctx.fillText(
                name,
                menuX + paddingX,
                itemY + itemHeight / 2
            );

            if (i < this.factoryNames.length - 1) {
                ctx.strokeStyle = Theme.colors.panelBorder;
                ctx.lineWidth = 0.5;
                ctx.beginPath();
                ctx.moveTo(menuX, itemY + itemHeight);
                ctx.lineTo(menuX + menuWidth, itemY + itemHeight);
                ctx.stroke();
            }
        }

        ctx.restore();
    }

    destroy() {
        if (this.unsubParam) this.unsubParam();
        if (this.unsubBus) this.unsubBus();
    }
}