// ui/AdsrPanel.js
import { PARAM_IDS } from '../core/paramIds.js';
import { GridContainer } from '../ui/GridContainer.js';
import { ADSREnvelope } from '../ui/ADSREnvelope.js';
import { Knob } from '../ui/Knob.js';
import { Checkbox } from '../ui/Checkbox.js';
import { TextDisplay } from '../ui/TextDisplay.js';
import { createKnob } from '../ui/createKnob.js';
import { paramStore } from '../core/ParameterStore.js';
import { bus } from '../eventBus.js';

export class AdsrPanel extends GridContainer {
    constructor() {
        super({
            cols: 1,
            rows: 2,
            padding: 0,
            gap: 0,
            weightsY: [0.8]
        });

        const envelopeGrid = new GridContainer({ cols: 4, rows: 1 });
        const checkboxGrid = new GridContainer({ cols: 4, rows: 1 });
        this.add(envelopeGrid);
        this.add(checkboxGrid);

        // Helper to bind an ADSR UI component to its parameter store entries
        const bindAdsrComponent = (envComponent, prefix) => {
            const mappings = {
                attack: PARAM_IDS[`${prefix}_ATTACK`],
                attackSlope: PARAM_IDS[`${prefix}_ATTACK_SLOPE`],
                decay: PARAM_IDS[`${prefix}_DECAY`],
                decaySlope: PARAM_IDS[`${prefix}_DECAY_SLOPE`],
                sustain: PARAM_IDS[`${prefix}_SUSTAIN`],
                release: PARAM_IDS[`${prefix}_RELEASE`],
                releaseSlope: PARAM_IDS[`${prefix}_RELEASE_SLOPE`]
            };

            // 1. Pull current initial values from store into UI
            const initialValues = {};
            Object.entries(mappings).forEach(([prop, paramId]) => {
                if (paramId) {
                    initialValues[prop] = paramStore.getValue(paramId);
                }
            });
            envComponent.setValue(initialValues);

            // 2. When user drags UI nodes/curves, send changes to ParameterStore
            envComponent.onChange((values) => {
                Object.entries(mappings).forEach(([prop, paramId]) => {
                    if (paramId && values[prop] !== undefined) {
                        paramStore.setValue(paramId, values[prop], 'ui');
                    }
                });
            });

            // 3. Sync UI when store changes externally (presets, automation, etc.)
            Object.entries(mappings).forEach(([prop, paramId]) => {
                if (paramId) {
                    bus.on(`param:${paramId}`, ({ value }) => {
                        const currentVal = envComponent.getValue()[prop];
                        if (Math.abs(currentVal - value) > 1e-4) {
                            envComponent.setValue({ [prop]: value });
                            envComponent.requestRender();
                        }
                    });
                }
            });
        };

        // Envelopes
        const ampEnv = new ADSREnvelope(1, 'Amp');
        const filterEnv = new ADSREnvelope(2, 'Filter');
        const pitchEnv = new ADSREnvelope(3, 'Pitch');
        const modEnv = new ADSREnvelope(4, 'Mod');

        bindAdsrComponent(ampEnv, 'AMP_ENV');
        bindAdsrComponent(filterEnv, 'FILTER_ENV');
        bindAdsrComponent(pitchEnv, 'PITCH_ENV');
        bindAdsrComponent(modEnv, 'MOD_ENV');

        envelopeGrid.add(ampEnv);
        envelopeGrid.add(filterEnv);
        envelopeGrid.add(pitchEnv);
        envelopeGrid.add(modEnv);

        const createCheckboxGroup = (items) => {
            const group = new GridContainer({ cols: items.length, rows: 1 });
            items.forEach(({ id, label }) => {
                if (!id) {
                    // Empty placeholder if ID is missing
                    group.add(new GridContainer({ cols: 1, rows: 1 }));
                    return;
                }
                const checkbox = new Checkbox(id, label);
                group.add(checkbox);
            });
            return group;
        };

        // Col 1: Amp text display
        const textDisplay = new TextDisplay('', { onChange: (text) => textDisplay.setText(text) });
        paramStore.subscribe((id, value) => {
            let stringValue = paramStore.getFormattedValue(id);
            if (id === PARAM_IDS.FM_OSC2_RATIO && stringValue === '0.00') {
                stringValue = '0.50';
            }
            if (id === PARAM_IDS.FM_OSC3_RATIO && stringValue === '0.00') {
                stringValue = '0.50';
            }
            textDisplay.setText(paramStore.getName(id) + ': ' + stringValue); 
        });
        checkboxGrid.add(textDisplay);

        // Col 2: Filter targets
        const filterGrid = new GridContainer({ cols: 3, rows: 1, weightsX: [0.06, 0.77] });
        filterGrid.add(new GridContainer());
        filterGrid.add(createCheckboxGroup([
            { id: PARAM_IDS.FILTER_ENV_OSC1, label: 'Osc 1' },
            { id: PARAM_IDS.FILTER_ENV_OSC2, label: 'Osc 2' },
            { id: PARAM_IDS.FILTER_ENV_OSC3, label: 'Osc 3' },
        ]));
        checkboxGrid.add(filterGrid);

        // Col 3: Pitch targets
        const pitchGrid = new GridContainer({ cols: 4, rows: 1, weightsX: [0.06, 0.76, 0.11] });
        pitchGrid.add(new GridContainer());
        pitchGrid.add(createCheckboxGroup([
            { id: PARAM_IDS.PITCH_ENV_OSC1, label: 'Osc 1' },
            { id: PARAM_IDS.PITCH_ENV_OSC2, label: 'Osc 2' },
            { id: PARAM_IDS.PITCH_ENV_OSC3, label: 'Osc 3' },
        ]));
        const pitchAmountKnob = createKnob(PARAM_IDS.PITCH_ENV_AMOUNT, '', {size: Knob.SIZES.small});
        pitchGrid.add(pitchAmountKnob);
        checkboxGrid.add(pitchGrid);

        // Col 4: Mod targets
        const modGrid = new GridContainer(
            { cols: 3, rows: 1, weightsX: [0.05, 0.86] });
        modGrid.add(new GridContainer());
        modGrid.add(createCheckboxGroup([
            { id: PARAM_IDS.MOD_ENV_SHAPE1, label: 'Sh 1' },
            { id: PARAM_IDS.MOD_ENV_SHAPE2, label: 'Sh 2' },
            { id: PARAM_IDS.MOD_ENV_SHAPE3, label: 'Sh 3' },
            { id: PARAM_IDS.MOD_ENV_FM,     label: 'FM' }
        ]));
        checkboxGrid.add(modGrid);
    }
}