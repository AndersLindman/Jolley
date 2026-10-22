// src/ui/createKnob.js
import { Knob } from './Knob.js';
import { paramStore } from '../core/ParameterStore.js';

export function createKnob(paramId, labelText, options = {}) {
    const param = paramStore.parameters.get(paramId);
    if (param.steps > 0) {
        options.steps = param.steps;
    }
    return new Knob(
        paramId,
        labelText,
        paramStore.getNormalized(paramId),
        (v) => paramStore.setNormalized(paramId, v, 'ui'),
        options
    );
}