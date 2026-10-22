// src/ui/Oscillator.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';
import { GridContainer } from './GridContainer.js';
import { Label } from './Label.js';
import { WaveformDisplay } from './WaveformDisplay.js';
import { Knob } from './Knob.js';
import { Checkbox } from './Checkbox.js';
import { Button } from './Button.js';
import { paramStore } from '../core/ParameterStore.js';
import { PARAM_IDS } from '../core/paramIds.js';
import { bus } from '../eventBus.js';

function getOscillatorGrid(oscNr, label, extraPanel) {
    const mainGrid = new GridContainer({
        cols: 1,
        rows: 2,
        gap: 0,
        weightsY: [0.45]
    });

    const displayGrid = new GridContainer({
        cols: 2,
        rows: 1,
        weightsX: [0.35]
    });
    mainGrid.add(displayGrid);

    const knobGrid = new GridContainer({
        cols: 1,
        rows: 2,
        gap: 0
    });
    mainGrid.add(knobGrid);

    const upperKnobs = new GridContainer({
        cols: 4,
        rows: 1
    });
    knobGrid.add(upperKnobs);

    const lowerKnobs = new GridContainer({
        cols: 2,
        rows: 1
    });
    knobGrid.add(lowerKnobs);

    // --- Header & Shape Control (Left) ---
    const shapeGrid = new GridContainer({
        cols: 1,
        rows: 2,
        weightsY: [0.28]
    });
    const title = new Label(label,
        { size: Theme.typography.titleSize, baseline: 'bottom' });
    shapeGrid.add(title);

    const shapeId = PARAM_IDS[`OSC${oscNr}_SHAPE`];
    const shapeKnob = new Knob(
        shapeId,
        'Shape',
        paramStore.getNormalized(shapeId),
        (v) => paramStore.setNormalized(shapeId, v, shapeKnob),
        {theme: 'light'}
    );
    shapeGrid.add(shapeKnob);
    displayGrid.add(shapeGrid);

    // --- Waveform Visualizer (Right) ---
    const rolloffId = PARAM_IDS[`OSC${oscNr}_ROLLOFF`];
    const waveformDisplay = new WaveformDisplay(
        `waveform-osc-${oscNr}`,
        'Waveform',
        {
            shape: paramStore.getValue(shapeId),
            a: paramStore.getValue(rolloffId)
        }
    );
    displayGrid.add(waveformDisplay);

    // Waveform reactivity
    bus.on(`param:${shapeId}`, ({ normalized }) => waveformDisplay.setShape(normalized));
    bus.on(`param:${rolloffId}`, ({ normalized }) => waveformDisplay.setRolloff(normalized));

    // --- Upper Pitch/Timbre Knobs ---
    const octaveId = PARAM_IDS[`OSC${oscNr}_OCTAVE`];
    const octaveKnob = new Knob(
        octaveId,
        'Octave',
        paramStore.getNormalized(octaveId),
        (v) => paramStore.setNormalized(octaveId, v, octaveKnob),
        { steps: paramStore.parameters.get(octaveId).steps }
    );

    const semitoneId = PARAM_IDS[`OSC${oscNr}_SEMITONE`];
    const semitoneKnob = new Knob(
        semitoneId,
        'Semitone',
        paramStore.getNormalized(semitoneId),
        (v) => paramStore.setNormalized(semitoneId, v, semitoneKnob),
        { steps: paramStore.parameters.get(semitoneId).steps }
    );

    const fineId = PARAM_IDS[`OSC${oscNr}_FINE`];
    const finetuneKnob = new Knob(
        fineId,
        'Fine-tune',
        paramStore.getNormalized(fineId),
        (v) => paramStore.setNormalized(fineId, v, finetuneKnob)
    );

    const aKnob = new Knob(
        rolloffId,
        'Roll-off',
        paramStore.getNormalized(rolloffId),
        (v) => paramStore.setNormalized(rolloffId, v, aKnob)
    );

    upperKnobs.add(aKnob);
    upperKnobs.add(octaveKnob);
    upperKnobs.add(semitoneKnob);
    upperKnobs.add(finetuneKnob);

    // --- Lower Output Knobs ---
    const lowerKnobsGrid = new GridContainer({
        cols: 2,
        rows: 1
    });

    const panId = PARAM_IDS[`OSC${oscNr}_PAN`];
    const panKnob = new Knob(
        panId,
        'Pan',
        paramStore.getNormalized(panId),
        (v) => paramStore.setNormalized(panId, v, panKnob)
    );

    const levelId = PARAM_IDS[`OSC${oscNr}_LEVEL`];
    const volumeKnob = new Knob(
        levelId,
        'Level',
        paramStore.getNormalized(levelId),
        (v) => paramStore.setNormalized(levelId, v, volumeKnob)
    );

    lowerKnobsGrid.add(panKnob);
    lowerKnobsGrid.add(volumeKnob);

    lowerKnobs.add(lowerKnobsGrid);
    lowerKnobs.add(extraPanel);

    return mainGrid;
}

export class Oscillator1 extends UIComponent {
    constructor() {
        super();

        const syncPanel = new GridContainer({
            cols: 2,
            rows: 1,
            gap: 4
        });

        const sync2Checkbox = new Checkbox(PARAM_IDS.OSC2_SYNC_TO_OSC1, 'Sync 2');
        const sync3Checkbox = new Checkbox(PARAM_IDS.OSC3_SYNC_TO_OSC1, 'Sync 3');
        syncPanel.add(sync2Checkbox);
        syncPanel.add(sync3Checkbox);
        this.add(getOscillatorGrid(1, '| OSC 1', syncPanel));
    }
}

export class Oscillator2 extends UIComponent {
    constructor() {
        super();

        const copyPanel = new GridContainer({
            cols: 2,
            rows: 1,
            gap: 4
        });

        // "🠈 Copy Osc 1 -> Osc 2"
        const copyFromOsc1Btn = new Button('1→2', () => {
            paramStore.copyOscillator(1, 2);
        });

        // "🠊 Copy Osc 2 -> Osc 3"
        const copyToOsc3Btn = new Button('2→1', () => {
            paramStore.copyOscillator(2, 1);
        });

        copyPanel.add(copyFromOsc1Btn);
        copyPanel.add(copyToOsc3Btn);

        this.add(getOscillatorGrid(2, '| OSC 2', copyPanel));
    }
}

export class Oscillator3 extends UIComponent {
    constructor() {
        super();

        const copyPanel = new GridContainer({
            cols: 2,
            rows: 1,
            gap: 4
        });

        // "🠈 Copy Osc 1 -> Osc 2"
        const copyFromOsc1Btn = new Button('2→3', () => {
            paramStore.copyOscillator(2, 3);
        });

        // "🠊 Copy Osc 2 -> Osc 3"
        const copyToOsc3Btn = new Button('3→2', () => {
            paramStore.copyOscillator(3, 2);
        });

        copyPanel.add(copyFromOsc1Btn);
        copyPanel.add(copyToOsc3Btn);

        this.add(getOscillatorGrid(3, '| OSC 3', copyPanel));
    }
}