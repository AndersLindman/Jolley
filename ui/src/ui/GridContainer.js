// src/ui/GridContainer.js

import { UIComponent } from './UIComponent.js';

export class GridContainer extends UIComponent {
    constructor({
                    cols = 1,
                    rows = 1,
                    padding = 0,
                    gap = 0,
                    weightsX = null,
                    weightsY = null
                } = {}) {
        super();

        this.cols = cols;
        this.rows = rows;
        this.padding = padding;
        this.gap = gap;
        this.weightsX = weightsX;
        this.weightsY = weightsY;
    }

    computeLayout(x, y, width, height, rootWidth = width) {
        this.layout = {
            x,
            y,
            width,
            height
        };

        const {
            cols,
            rows,
            weightsX,
            weightsY,
            padding: rawPadding,
            gap: rawGap
        } = this;

        // Scale gaps and padding proportionally based on a base design width of 1000px
        const scale = rootWidth / 1000;
        const padding = rawPadding * scale;
        const gap = rawGap * scale;

        const innerWidth = width - padding * 2;
        const innerHeight = height - padding * 2;

        const totalGapX = gap * (cols - 1);
        const totalGapY = gap * (rows - 1);

        const availableWidth = innerWidth - totalGapX;
        const availableHeight = innerHeight - totalGapY;

        // --- Process X Weights ---
        let colWeights = weightsX && weightsX.length > 0 ? [...weightsX] : [];
        if (colWeights.length > 0) {
            while (colWeights.length < cols) {
                const sum = colWeights.reduce((acc, val) => acc + val, 0);
                const remaining = Math.max(0, 1 - sum);
                colWeights.push(remaining / (cols - colWeights.length));
            }
        } else {
            colWeights = Array(cols).fill(1 / cols);
        }
        const colWidths = colWeights.map(w => w * availableWidth);

        // --- Process Y Weights ---
        let rowWeights = weightsY && weightsY.length > 0 ? [...weightsY] : [];
        if (rowWeights.length > 0) {
            while (rowWeights.length < rows) {
                const sum = rowWeights.reduce((acc, val) => acc + val, 0);
                const remaining = Math.max(0, 1 - sum);
                rowWeights.push(remaining / (rows - rowWeights.length));
            }
        } else {
            rowWeights = Array(rows).fill(1 / rows);
        }
        const rowHeights = rowWeights.map(w => w * availableHeight);

        // --- Layout Children ---
        for (let i = 0; i < this.children.length; i++) {
            const child = this.children[i];

            const col = i % cols;
            const row = Math.floor(i / cols);

            let childX = padding;
            for (let c = 0; c < col; c++) {
                childX += colWidths[c] + gap;
            }

            let childY = padding;
            for (let r = 0; r < row; r++) {
                childY += rowHeights[r] + gap;
            }

            const cellWidth = colWidths[col];
            const cellHeight = rowHeights[row];

            child.computeLayout(
                childX,
                childY,
                cellWidth,
                cellHeight,
                rootWidth
            );
        }
    }
}