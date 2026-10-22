// src/ui/TextDisplay.js
import { Label } from './Label.js';
import { Theme } from '../theme.js';

export class TextDisplay extends Label {
    constructor(text, options = {}) {
        super(text, {
            color: Theme.colors.buttonText,
            size: Theme.typography.labelSize,},
        );
        this.onChange = options.onChange || null;
    }

    setText(text) {
        if (this.text !== text) {
            this.text = text;
            this.isDirty = true;
            this.requestRender();
            if (this.onChange) {
                this.onChange(text);
            }
        }
    }
}