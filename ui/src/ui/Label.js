// src/ui/Label.js

import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';

export class Label extends UIComponent {
    constructor(
        text,
        {
            align = 'center',
            baseline = 'middle',
            size = Theme.typography.labelSize,
            weight = Theme.typography.weightMedium,
            color = Theme.colors.textPrimary
        } = {}
    ) {
        super();

        this.text = text;
        this.align = align;
        this.baseline = baseline;
        this.size = size;
        this.weight = weight;
        this.color = color;
    }

    drawSelf(ctx) {
        const {
            x,
            y,
            width,
            height
        } = this.layout;

        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(x, y, width, height);
        ctx.fillStyle = this.color;
        ctx.textBaseline = this.baseline;

        let textX = x + width / 2;

        if (this.align === 'left') {
            textX = x + 8; // Optional padding
        } else if (this.align === 'right') {
            textX = x + width - 8;
        }

        const textY = y + height / 2;

        this.drawText(
            ctx,
            this.text,
            textX,
            textY,
            {
                size: this.size,
                align: this.align,
                baseline: this.baseline,
                color: this.color
            }
        );
    }

    setText(text) {
        this.text = text;
        this.isDirty = true;
    }
}