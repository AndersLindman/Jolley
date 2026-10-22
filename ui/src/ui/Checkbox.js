// src/ui/Checkbox.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { paramStore } from '../core/ParameterStore.js';

export class Checkbox extends UIComponent {
    constructor(id, label = '') {
        super();
        this.id = id;
        this.label = label;
        
        const initialNormalized = paramStore.getNormalized ? paramStore.getNormalized(this.id) : 0;
        this.value = initialNormalized > 0.5;
        
        this.unsubscribe = paramStore.subscribe((id, normalizedValue) => {
            if (id === this.id) {
                const newBoolVal = normalizedValue > 0.5;
                if (this.value !== newBoolVal) {
                    this.value = newBoolVal;
                    this.isDirty = true;
                    this.requestRender();
                }
            }
        });
    }

    setValue(val, silent = false) {
        const boolVal = Boolean(val);

        if (this.value !== boolVal) {
            this.value = boolVal;

            if (!silent) {
                this.isDirty = true;
                this.requestRender();

                if (this.id) {
                    paramStore.setNormalized(this.id, boolVal ? 1 : 0, 'ui');
                }
            }
        }
    }

    handlePointerDown(x, y, event) {
        this.setValue(!this.value);
    }

    destroy() {
        if (this.unsubscribe) {
            this.unsubscribe();
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

        // Erase old bounding box background
        ctx.fillStyle = Theme.colors.panelBackground || '#1A1A1E';
        ctx.fillRect(0, 0, width, height);

        const boxSize = Math.min(width * 0.3, height * 0.4, 14 * scale);
        const padding = 6 * scale;
        const boxX = padding;
        const boxY = (height - boxSize) / 2;

        // Bezel
        ctx.fillStyle = Theme.colors.knobBorder || '#2A2A30';
        ctx.fillRect(boxX - scale, boxY - scale, boxSize + 2 * scale, boxSize + 2 * scale);

        // Active LED status color
        const ledColor = this.value
            ? (Theme.colors.ledOn)
            : (Theme.colors.ledOff);

        ctx.fillStyle = ledColor;
        ctx.fillRect(boxX, boxY, boxSize, boxSize);

        // Glow center accent when active
        if (this.value) {
            ctx.fillStyle = '#cc9966';
            ctx.fillRect(
                boxX + boxSize * 0.25,
                boxY + boxSize * 0.25,
                boxSize * 0.5,
                boxSize * 0.5
            );
        }

        // Label Text
        if (this.label) {
            ctx.fillStyle = this.value
                ? (Theme.colors.textPrimary)
                : (Theme.colors.textMuted);
            ctx.textAlign = 'left';
            ctx.textBaseline = 'middle';

            const textX = boxX + boxSize + padding;
            const textY = height / 2;
            ctx.fillText(this.label, textX, textY);
        }
    }
}