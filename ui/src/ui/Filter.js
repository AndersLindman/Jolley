// src/ui/Filter.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { GridContainer } from './GridContainer.js';
import { Label } from './Label.js';
import { Knob } from './Knob.js';
import { Button } from './Button.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from '../core/paramIds.js';
import { bus } from '../eventBus.js';

function createKnob(paramId, labelText, theme = 'dark') {
    const param = paramStore.parameters?.get(paramId);
    const options = param?.steps ? { steps: param.steps, theme: theme } : { theme: theme };

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

export class Filter extends UIComponent {
    constructor() {
        super();

        const mainGrid = new GridContainer({
            cols: 1,
            rows: 2,
            gap: 10,
            weightsY: [0.22, 0.78]
        });

        // --- Header Grid (Title + 3 Mode Radio Buttons) ---
        const headerGrid = new GridContainer({
            cols: 2,
            rows: 1,
            weightsX: [0.35, 0.65]
        });

        const title = new Label('| FILTER', {
            size: Theme.typography.titleSize,
            baseline: 'bottom'
        });
        headerGrid.add(title);

        const typeSelectorGrid = new GridContainer({
            cols: 3,
            rows: 1,
            gap: 2
        });

// Create Radio Buttons with active color options
        this.typeButtons = [
            new Button('ZDF', () => this.setFilterType(0), { activeColor: '#00FFCC' }),
            new Button('Ladder', () => this.setFilterType(1), { activeColor: '#00FFCC' }),
            new Button('Diode',  () => this.setFilterType(2), { activeColor: '#FF9900' }) // Orange accent for 303
        ];

        this.typeButtons.forEach(btn => typeSelectorGrid.add(btn));

        // Sync button states on param updates
        bus.on(`param:${PARAM_IDS.FILTER_TYPE}`, ({ value }) => {
            this.updateButtonStates(value);
        });

        // Initialize state
        this.updateButtonStates(paramStore.getValue(PARAM_IDS.FILTER_TYPE));

        headerGrid.add(typeSelectorGrid);
        mainGrid.add(headerGrid);

        // --- 6-Knob Matrix Grid ---
        const knobGrid = new GridContainer({
            cols: 5,
            rows: 2,
            gap: 4,
            weightsX: [0.09, 0.28, 0.28, 0.28]
        });

        const cutoffKnob   = createKnob(PARAM_IDS.FILTER_CUTOFF, 'Cutoff', 'light');
        const resoKnob     = createKnob(PARAM_IDS.FILTER_RESONANCE, 'Resonance');
        const driveKnob    = createKnob(PARAM_IDS.FILTER_DRIVE, 'Drive');

        const feedbackKnob = createKnob(PARAM_IDS.FILTER_FEEDBACK, 'Feedback');
        const morphKnob    = createKnob(PARAM_IDS.FILTER_MORPH, 'Morph');
        const envAmtKnob   = createKnob(PARAM_IDS.FILTER_ENV_AMOUNT, 'Env Depth');

        // Row 1: Cutoff, Reso, Drive
        knobGrid.add(new GridContainer({ cols: 1, rows: 1 }));
        knobGrid.add(cutoffKnob);
        knobGrid.add(resoKnob);
        knobGrid.add(driveKnob);
        knobGrid.add(new GridContainer({ cols: 1, rows: 1 }));

        // Row 2: Feedback, Morph, Env Depth
        knobGrid.add(new GridContainer({ cols: 1, rows: 1 }));
        knobGrid.add(feedbackKnob);
        knobGrid.add(morphKnob);
        knobGrid.add(envAmtKnob);
        knobGrid.add(new GridContainer({ cols: 1, rows: 1 }));

        mainGrid.add(knobGrid);
        this.add(mainGrid);
    }

    setFilterType(typeIndex) {
        paramStore.setValue(PARAM_IDS.FILTER_TYPE, typeIndex);
    }

    setFilterType(typeIndex) {
        paramStore.setValue(PARAM_IDS.FILTER_TYPE, typeIndex);
    }

    updateButtonStates(selectedIndex) {
        this.typeButtons.forEach((btn, index) => {
            btn.setActive(index === selectedIndex);
        });
    }
}