// src/ui/Voice.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { GridContainer } from './GridContainer.js';
import { Label } from './Label.js';
import { Knob } from './Knob.js';
import { Button } from './Button.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from '../core/paramIds.js';
import { bus } from '../eventBus.js';
import { createKnob } from './createKnob.js';

export class Voice extends UIComponent {
    constructor() {
        super();

        const mainGrid = new GridContainer({
            cols: 1,
            rows: 3,
            gap: 4,
            weightsY: [0.12, 0.7, 0.18]
        });

        // --- 1. Header ---
        const headerGrid = new GridContainer({ cols: 1, rows: 1 });
        const title = new Label('VOICE', {
            size: Theme.typography.titleSize,
            baseline: 'bottom'
        });
        headerGrid.add(title);
        mainGrid.add(headerGrid);

        // --- 2. 4x2 Knob Matrix Grid ---
        const matrixGrid = new GridContainer({
            cols: 4,
            rows: 2,
            gap: 2
        });

        matrixGrid.add(createKnob(PARAM_IDS.UNISON_VOICES, 'Voices'));
        matrixGrid.add(createKnob(PARAM_IDS.UNISON_DETUNE, 'Detune'));
        matrixGrid.add(createKnob(PARAM_IDS.UNISON_SPREAD, 'Spread'));
        matrixGrid.add(createKnob(PARAM_IDS.VOICE_DRIFT,  'Drift'));

        matrixGrid.add(createKnob(PARAM_IDS.VOICE_GLIDE,      'Glide'));
        matrixGrid.add(createKnob(PARAM_IDS.VOICE_BEND,       'Bend'));
        matrixGrid.add(createKnob(PARAM_IDS.VOICE_VELOCITY,   'Velocity'));
        matrixGrid.add(createKnob(PARAM_IDS.VOICE_MOD_CUTOFF, 'Mod Cutoff'));

        mainGrid.add(matrixGrid);
        this.add(mainGrid);

        // --- 3. Voice Mode Radio Buttons ---
        const modeGrid = new GridContainer({
            cols: 3,
            rows: 1,
            gap: 4
        });

        const modes = [
            { label: 'POLY', value: 0 },
            { label: 'MONO', value: 1 },
            { label: 'LEGATO', value: 2 }
        ];

        this.modeButtons = modes.map(({ label, value }) => {
            return new Button(
                label,
                () => paramStore.setValue(PARAM_IDS.VOICE_MODE, value),
                { activeColor: '#00FFCC' }
            );
        });

        this.modeButtons.forEach(btn => modeGrid.add(btn));
        mainGrid.add(modeGrid);

        // Mode Radio Button State Listener
        bus.on(`param:${PARAM_IDS.VOICE_MODE}`, ({ value }) => {
            this.updateVoiceModeButtons(value);
        });

        const currentMode = paramStore.getValue(PARAM_IDS.VOICE_MODE) ?? 0;
        this.updateVoiceModeButtons(currentMode);
    }

    updateVoiceModeButtons(activeValue) {
        this.modeButtons.forEach((btn, index) => {
            btn.setActive(index === activeValue);
        });
    }
}