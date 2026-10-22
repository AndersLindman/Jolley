// src/ui/Button.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';

export class Button extends UIComponent {
    constructor(label, onClick = null, options = {}) {
        super();
        this.label = label;
        this.onClick = onClick;
        this.isPressed = false;

        // Active State & Color Overrides
        this.isActive = options.isActive ?? options.active ?? false;
        this.activeColor = options.activeColor ?? options.accentColor ?? Theme.colors.buttonText;
        this.inactiveColor = options.inactiveColor ?? Theme.colors.textPrimary;
        this.activeBg = options.activeBg ?? Theme.colors.knobBodyDark;
        this.inactiveBg = options.inactiveBg ?? Theme.colors.knobBody;
        this.borderColor = options.borderColor ?? Theme.colors.knobBorder;
    }

    setActive(active) {
        if (this.isActive !== active) {
            this.isActive = active;
            this.isDirty = true;
            this.requestRender();
        }
    }

    setColor(color) {
        if (this.activeColor !== color) {
            this.activeColor = color;
            this.isDirty = true;
            this.requestRender();
        }
    }

    setLabel(label) {
        if (this.label !== label) {
            this.label = label;
            this.isDirty = true;
            this.requestRender();
        }
    }

    handlePointerDown(x, y, event) {
        this.isPressed = true;
        this.isDirty = true;
        this.requestRender();

        if (this.onClick) {
            this.onClick();
        } else {
            console.warn(`[Button] "${this.label}" has no onClick handler!`);
        }
    }

    handlePointerUp(x, y, event) {
        if (this.isPressed) {
            this.isPressed = false;
            this.isDirty = true;
            this.requestRender();
        }
    }

    computeLayout(x, y, width, height, rootWidth) {
        super.computeLayout(x, y, width, height, rootWidth);
        this.width = width;
        this.height = height;
    }

    drawSelf(ctx) {
        const { width, height } = this;
        const scale = this.scale(1);

        // Clear cell background
        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(0, 0, width, height);

        const padding = 2 * scale;
        const btnWidth = width - padding * 2;
        const btnHeight = height - padding * 2;

        const isHighlighted = this.isPressed || this.isActive;

        // Button body background
        ctx.fillStyle = isHighlighted ? this.activeBg : this.inactiveBg;
        ctx.fillRect(padding, padding, btnWidth, btnHeight);

        // Border / Bezel (highlights with active color when selected)
        ctx.strokeStyle = isHighlighted ? this.activeColor : this.borderColor;
        ctx.lineWidth = this.scale(1);
        ctx.strokeRect(padding, padding, btnWidth, btnHeight);

        // Label Text
        ctx.font = `${Math.round(12 * scale)}px sans-serif`;
        ctx.fillStyle = isHighlighted ? this.activeColor : this.inactiveColor;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(this.label, width / 2, height / 2);
    }
}