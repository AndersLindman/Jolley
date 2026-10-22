import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { bus } from '../eventBus.js';

export class Knob extends UIComponent {
    static SIZES = {
        small: 10,
        medium: 14,
        large: 18
    };
    static MIN_ANGLE = -Math.PI * 1.25;
    static MAX_ANGLE = Math.PI * 0.25;

    constructor(
        id,
        label,
        value = 0.5,
        onChange = null,
        {
            steps = 2,
            size = Knob.SIZES.medium,
            background = Theme.colors.panelBackground,
            theme = 'dark'
        } = {}
    ) {
        super();

        this.id = id;
        this.label = label;
        // Keep internal value safely clamped between 0 and 1
        this.value = Math.max(0, Math.min(1, value ?? 0.5));
        this.onChange = onChange;
        this.steps = steps;
        this.size = size;
        this.background = background;
        this.theme = theme;

        this.isDragging = false;
        this.startY = 0;
        this.startValue = this.value;

        // Automatically listen to ParameterStore updates for this.id
        if (this.id) {
            bus.on(`param:${this.id}`, (data) => {
                // Fallback if data is passed directly or wrapped in an object
                const normVal = typeof data === 'object' ? data.normalized : data;
                const origin = data?.origin;
                if (origin === this || this.isDragging) return;

                if (typeof normVal === 'number' && !isNaN(normVal)) {
                    this.setValue(normVal, true);
                }
            });
        }
    }

    setValue(value, silent = false) {
        const newValue = Math.max(0, Math.min(1, value));

        let finalValue = newValue;
        if (this.steps > 2) {
            finalValue = Math.round(newValue * (this.steps - 1)) / (this.steps - 1);
        }

        const valueChanged = finalValue !== this.value;

        if (valueChanged || silent) {
            this.value = finalValue;

            // Force repaint on external updates (copy, presets)
            this.isDirty = true;
            this.requestRender();

            if (!silent && valueChanged && this.onChange) {
                this.onChange(this.value);
            }
        }
    }

    handlePointerDown(x, y, event) {
        this.isDragging = true;
        this.startY = event.clientY;
        this.startValue = this.value;
        this.requestRender();
    }

    handlePointerMove(x, y, event) {
        if (!this.isDragging) return;

        const sensitivity = 150;
        const deltaY = this.startY - event.clientY;

        let newValue = this.startValue + (deltaY / sensitivity);

        if (this.steps > 2) {
            newValue = Math.round(newValue * (this.steps - 1)) / (this.steps - 1);
        }

        this.setValue(newValue);
    }

    handlePointerUp(x, y, event) {
        this.isDragging = false;
        if (this.steps > 2) {
            this.setValue(Math.round(this.value * (this.steps - 1)) / (this.steps - 1));
        }
    }

    computeLayout(x, y, width, height, rootWidth) {
        super.computeLayout(x, y, width, height, rootWidth);
        const scale = (px) => this.scale(px);
        this.width = width;
        this.height = height;
        this.centerX = width / 2;
        this.centerY = height / 2;
        this.radius = scale(this.size);
        this.labelOffsetY = -this.radius * 1.9;
        this.shadowOffsetY = scale(Theme.metrics.knobShadowOffsetY * this.size / 40);
        this.borderWidth = scale(this.size / 12);
        this.indicatorLength = this.radius * 0.5;
        this.indicatorWidth = scale(this.size / 8);
    }

    drawSelf(ctx) {
        const {
            width, height, centerX, centerY, radius, labelOffsetY, shadowOffsetY, borderWidth,
            indicatorLength, indicatorWidth, theme
        } = this;

        // Erase old Knob background
        ctx.fillStyle = this.background;
        ctx.fillRect(0, 0, width, height);

        // Label
        this.drawText(ctx, this.label, centerX, centerY + labelOffsetY,
            { size: this.size / 1.2 });

        // Ticks for stepped knobs
        ctx.lineWidth = indicatorWidth * 0.6;
        ctx.strokeStyle = this.theme == 'light' ? Theme.colors.knobIndicator2 : Theme.colors.knobIndicator;

        for (let i = 0; i < this.steps; i++) {
            const stepVal = i / (this.steps - 1);
            const angle = Knob.MIN_ANGLE + stepVal * (Knob.MAX_ANGLE - Knob.MIN_ANGLE);

            const isNearTop = angle > -Math.PI * 0.6 && angle < -Math.PI * 0.4;
            const startMultiplier = isNearTop ? 1.05 : 1.1;
            const endMultiplier = isNearTop ? 1.25 : 1.45;

            const startX = centerX + Math.cos(angle) * radius * startMultiplier;
            const startY = centerY + Math.sin(angle) * radius * startMultiplier;
            const endX = centerX + Math.cos(angle) * radius * endMultiplier;
            const endY = centerY + Math.sin(angle) * radius * endMultiplier;

            ctx.beginPath();
            ctx.moveTo(startX, startY);
            ctx.lineTo(endX, endY);
            ctx.stroke();
        }

        // Calculate angle from current normalized [0, 1] value
        const normVal = Math.max(0, Math.min(1, this.value ?? 0.5));
        const angle = Knob.MIN_ANGLE + normVal * (Knob.MAX_ANGLE - Knob.MIN_ANGLE);

        // Knob Shadow
        ctx.fillStyle = Theme.colors.shadowColor;
        ctx.beginPath();
        ctx.arc(centerX, centerY + shadowOffsetY, radius, 0, Math.PI * 2);
        ctx.fill();

        // Knob Upper Half Body
        ctx.fillStyle = this.theme == 'light' ? Theme.colors.knobBody2 : Theme.colors.knobBody;
        ctx.strokeStyle = this.theme == 'light' ? Theme.colors.knobBorder2 : Theme.colors.knobBorder;
        ctx.lineWidth = borderWidth;
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();

        // Knob Lower Half Shade
        ctx.fillStyle = this.theme == 'light' ? Theme.colors.knobBodyDark2 : Theme.colors.knobBodyDark;
        ctx.strokeStyle = this.theme == 'light' ? Theme.colors.knobBorderdark2 : Theme.colors.knobBorderdark;
        ctx.lineWidth = borderWidth;
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI);
        ctx.fill();
        ctx.stroke();

        // Indicator Line
        const indicatorDistance = radius - indicatorLength;
        const indicatorStartX = centerX + Math.cos(angle) * indicatorDistance;
        const indicatorStartY = centerY + Math.sin(angle) * indicatorDistance;
        const indicatorEndX = centerX + Math.cos(angle) * radius * 0.9;
        const indicatorEndY = centerY + Math.sin(angle) * radius * 0.9;

        ctx.strokeStyle = Theme.colors.textPrimary;
        ctx.lineWidth = indicatorWidth;
        ctx.beginPath();
        ctx.moveTo(indicatorStartX, indicatorStartY);
        ctx.lineTo(indicatorEndX, indicatorEndY);
        ctx.stroke();
    }
}