// src/ui/AlignContainer.js

import { UIComponent } from './UIComponent.js';

export class AlignContainer extends UIComponent {
    constructor({
                    alignX = 'center', // 'left' | 'center' | 'right'
                    alignY = 'middle', // 'top' | 'middle' | 'bottom'
                    width = null,      // Optional fixed width
                    height = null      // Optional fixed height
                } = {}) {
        super();
        this.alignX = alignX;
        this.alignY = alignY;
        this.fixedWidth = width;
        this.fixedHeight = height;
    }

    computeLayout(x, y, width, height, rootWidth = width) {
        this.layout = { x, y, width, height };
        this.synthWidth = rootWidth;

        if (this.children.length === 0) return;
        const child = this.children[0];

        const childW = this.fixedWidth !== null ? this.fixedWidth : width;
        const childH = this.fixedHeight !== null ? this.fixedHeight : height;

        let childX = 0;
        if (this.alignX === 'center') {
            childX = (width - childW) / 2;
        } else if (this.alignX === 'right') {
            childX = width - childW;
        }

        let childY = 0;
        if (this.alignY === 'middle') {
            childY = (height - childH) / 2;
        } else if (this.alignY === 'bottom') {
            childY = height - childH;
        }

        child.computeLayout(childX, childY, childW, childH, rootWidth);
    }
}