// src/ui/LfoPanel.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { GridContainer } from './GridContainer.js';
import { Label } from './Label.js';
import { Knob } from './Knob.js';
import { Button } from './Button.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from '../core/paramIds.js';
import { bus } from '../eventBus.js';

function createKnob(paramId, labelText) {
    const param = paramStore.parameters?.get(paramId);
    const options = param?.steps ? { steps: param.steps } : undefined;

    const knob = new Knob(
        paramId,
        labelText,
        paramStore.getNormalized(paramId),
        (v) => paramStore.setNormalized(paramId, v, knob),
        options
    );

    bus.on(`param:${paramId}`, ({ normalized }) => {
        knob.setValue(normalized);
    });

    return knob;
}

export class LFO extends UIComponent {
    constructor() {
        super();

        const mainGrid = new GridContainer({
            cols: 1,
            rows: 2,
            gap: 2,
            weightsY: [0.12, 0.88]
        });

        // --- Header ---
        const headerGrid = new GridContainer({ cols: 1, rows: 1 });
        headerGrid.add(new Label('LFO', { size: Theme.typography.titleSize, baseline: 'bottom' }));
        mainGrid.add(headerGrid);

        // --- Dual LFO Column Setup ---
        const lfoColumnsGrid = new GridContainer({
            cols: 2,
            rows: 1,
            gap: 8
        });

        // Specific destination mappings per LFO
        const lfo1Destinations = [
            { label: 'Pitch 1', paramId: PARAM_IDS.LFO1_DEST_OSC1_PITCH },
            { label: 'Shape 1', paramId: PARAM_IDS.LFO1_DEST_OSC1_SHAPE },
            { label: 'Cutoff',  paramId: PARAM_IDS.LFO1_DEST_CUTOFF }
        ];

        const lfo2Destinations = [
            { label: 'Pitch 2', paramId: PARAM_IDS.LFO2_DEST_OSC2_PITCH },
            { label: 'Shape 2', paramId: PARAM_IDS.LFO2_DEST_OSC2_SHAPE },
            { label: 'FM amt',  paramId: PARAM_IDS.LFO2_DEST_FM_AMOUNT }
        ];

        lfoColumnsGrid.add(this.buildLfoBlock(1, lfo1Destinations));
        lfoColumnsGrid.add(this.buildLfoBlock(2, lfo2Destinations));

        mainGrid.add(lfoColumnsGrid);
        this.add(mainGrid);
    }

    buildLfoBlock(lfoNum, destinations) {
        const blockGrid = new GridContainer({
            cols: 1,
            rows: 3,
            gap: 4,
            weightsY: [0.22, 0.48, 0.30]
        });

        // 1. Waveform Select Header
        const topRowGrid = new GridContainer({ cols: 3, rows: 1, gap: 2 });
        topRowGrid.add(new Label(`LFO ${lfoNum}`, { size: Theme.typography.textSize }));

        const shapeParamId = PARAM_IDS[`LFO${lfoNum}_SHAPE`];
        const sineBtn = new Button('SIN', () => paramStore.setValue(shapeParamId, 0), { activeColor: Theme.colors.buttonText });
        const triBtn  = new Button('TRI', () => paramStore.setValue(shapeParamId, 1), { activeColor: Theme.colors.buttonText });

        topRowGrid.add(sineBtn);
        topRowGrid.add(triBtn);
        blockGrid.add(topRowGrid);

        bus.on(`param:${shapeParamId}`, ({ value }) => {
            sineBtn.setActive(value === 0);
            triBtn.setActive(value === 1);
        });
        const currentShape = paramStore.getValue(shapeParamId) ?? 0;
        sineBtn.setActive(currentShape === 0);
        triBtn.setActive(currentShape === 1);

        // 2. Knobs (Rate & Depth)
        const knobGrid = new GridContainer({ cols: 2, rows: 1, gap: 4 });
        knobGrid.add(createKnob(PARAM_IDS[`LFO${lfoNum}_RATE`], 'Rate'));
        knobGrid.add(createKnob(PARAM_IDS[`LFO${lfoNum}_DEPTH`], 'Depth'));
        blockGrid.add(knobGrid);

        // 3. Target Routing Checkboxes
        const destGrid = new GridContainer({ cols: 3, rows: 1, gap: 2 });

        destinations.forEach(({ label, paramId }) => {
            const toggleBtn = new Button(label, () => {
                const currentVal = paramStore.getValue(paramId) ?? 0;
                paramStore.setValue(paramId, currentVal === 1 ? 0 : 1);
            }, { activeColor: Theme.colors.buttonText });

            bus.on(`param:${paramId}`, ({ value }) => {
                toggleBtn.setActive(value === 1);
            });
            toggleBtn.setActive(paramStore.getValue(paramId) === 1);

            destGrid.add(toggleBtn);
        });

        blockGrid.add(destGrid);
        return blockGrid;
    }
}