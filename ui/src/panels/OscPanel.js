// src/panels/OscPanel.js
import { GridContainer } from '../ui/GridContainer.js';
import { Oscillator1 } from '../ui/Oscillator.js';
import { Oscillator2 } from '../ui/Oscillator.js';
import { Oscillator3 } from '../ui/Oscillator.js';
import { FmMatrix } from "../ui/FmMatrix.js";
import { bus } from '../eventBus.js';

export class OscPanel extends GridContainer {
    constructor() {
        super({ cols: 4, rows: 1, padding: 0, gap: 6 });

        this.add(new Oscillator1());
        this.add(new Oscillator2());
        this.add(new Oscillator3());
        this.add(new FmMatrix());
    }

    destroy() {
        bus.off('param:osc1:shape', this.updateWaveform);
        bus.off('param:osc1:rolloff', this.updateWaveform);
    }
}