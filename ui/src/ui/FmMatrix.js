// src/ui/FmPanel.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { GridContainer } from './GridContainer.js';
import { Label } from './Label.js';
import { Knob } from './Knob.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from '../core/paramIds.js';
import { bus } from '../eventBus.js';
import { createKnob } from './createKnob.js';
import { Checkbox } from "./Checkbox.js";

export class FmMatrix extends UIComponent {
    constructor() {
        super();

        const mainGrid = new GridContainer({
            cols: 1,
            rows: 2,
            gap: 0,
            weightsY: [0.2]
        });

        // --- Header ---
        const headerGrid = new GridContainer({
            cols: 4,
            rows: 1,
            weightsX: [0.05, 0.12, 0.41]
        });
        
        const activeCheckbox = new Checkbox(PARAM_IDS.FM_ACTIVE, 'Active');
        
        const title = new Label('FM MATRIX', {
            size: Theme.typography.titleSize,
            baseline: 'middle',
        });

        headerGrid.add(new GridContainer());
        headerGrid.add(activeCheckbox);
        headerGrid.add(title);
        mainGrid.add(headerGrid);

        // --- 3x2 Matrix Grid ---
        const matrixGrid = new GridContainer({
            cols: 5,
            rows: 2,
            gap: 0,
            weightsX: [0.04, 0.31, 0.31, 0.31, 0.03]
        });

        // Row 1: Oscillator 3 Modulation Chain
        const osc3RatioKnob = createKnob(PARAM_IDS.FM_OSC3_RATIO, 'Osc 3 Ratio');
        const depth3to2Knob = createKnob(PARAM_IDS.FM_DEPTH_3TO2, '3 → 2 Depth');
        const osc3FbKnob    = createKnob(PARAM_IDS.FM_OSC3_FEEDBACK, 'Osc 3 FB');

        // Row 2: Oscillator 2 Modulation Chain & Master Control
        const osc2RatioKnob = createKnob(PARAM_IDS.FM_OSC2_RATIO, 'Osc 2 Ratio');
        const depth2to1Knob = createKnob(PARAM_IDS.FM_DEPTH_2TO1, '2 → 1 Depth');
        const masterFmKnob  = createKnob(PARAM_IDS.FM_MASTER_AMOUNT, 'FM Amount');

        matrixGrid.add(new GridContainer({ cols: 1, rows: 1 }));
        matrixGrid.add(osc3RatioKnob);
        matrixGrid.add(depth3to2Knob);
        matrixGrid.add(osc3FbKnob);
        matrixGrid.add(new GridContainer({ cols: 1, rows: 1 }));

        matrixGrid.add(new GridContainer({ cols: 1, rows: 1 }));
        matrixGrid.add(osc2RatioKnob);
        matrixGrid.add(depth2to1Knob);
        matrixGrid.add(masterFmKnob);
        matrixGrid.add(new GridContainer({ cols: 1, rows: 1 }));

        mainGrid.add(matrixGrid);

        this.add(mainGrid);
    }
}