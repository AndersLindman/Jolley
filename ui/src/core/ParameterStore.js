// src/core/ParameterStore.js

import { bus } from '../eventBus.js';
import { PARAM_IDS } from './paramIds.js';
import { registerAllParameters } from './parameters.js';
import { getSliderState } from "../../juce-framework_webview.js";

export class ParameterStore {
    constructor() {
        const globalScope = typeof window !== 'undefined' ? window : typeof self !== 'undefined' ? self : globalThis;
        globalScope._storeInstanceCount = (globalScope._storeInstanceCount || 0) + 1;
        this.parameters = new Map();
        this.listeners = new Set();
        this.juceStates = new Map(); // Store JUCE webview state handlers
        registerAllParameters(this);
    }

    register({
               id,
               name = id,
               defaultValue = 0.5,
               min = 0,
               max = 1,
               steps = 0,
               getValueText = null,
               skew = 1
           }) {
        const initialNormalized = (defaultValue - min) / (max - min);

        this.parameters.set(id, {
            id,
            name,
            min,
            max,
            steps,
            skew,
            defaultValue,
            value: defaultValue,
            normalized: initialNormalized,
            getValueText: getValueText || ((v) => `${v.toFixed(2)}`)
        });

        // Initialize JUCE 8 webview binding for this parameter only if the bridge is ready
        try {
            // Check if JUCE frontend API is available
            if (typeof window.__JUCE__ === 'undefined' && typeof window.Juce === 'undefined') {
                // Fallback for standalone browser development
                console.warn(`[ParameterStore] JUCE webview binding skipped for "${id}": JUCE frontend API not available`);
                return; 
            }

            const sliderState = getSliderState(id);
            
            // 1. Sync from C++ backend / host to JS frontend
            sliderState.valueChangedEvent.addListener(() => {
                const normVal = sliderState.getNormalisedValue();
                if (Math.abs(normVal - this.getNormalized(id)) > 0.0001) {
                    this.setNormalized(id, normVal, 'host', true);
                }
            });

            this.juceStates.set(id, sliderState);
        } catch (e) {
            console.warn(`[ParameterStore] JUCE webview binding deferred or skipped for "${id}":`, e);
        }
    }

    setNormalized(id, normValue, origin = null, forceEmit = false) {
        const param = this.parameters.get(id);
        if (!param) {
            console.error(`[ParameterStore] setNormalized failed: Unknown param ID "${id}"`);
            return;
        }

        const clampedNorm = Math.max(0, Math.min(1, normValue));

        if (param.steps > 0) {
            const stepIndex = Math.round(clampedNorm * (param.steps - 1));
            const stepNorm = stepIndex / (param.steps - 1);
            param.normalized = stepNorm;
            param.value = param.min + stepNorm * (param.max - param.min);
        } else {
            param.normalized = clampedNorm;
            const norm = param.skew && param.skew !== 1
                ? Math.pow(clampedNorm, param.skew)
                : clampedNorm;
            param.value = param.min + norm * (param.max - param.min);
        }

        // If change originated from UI, update the JUCE webview state object
        if (origin === 'ui' || origin !== 'host' || forceEmit) {
            const juceState = this.juceStates.get(id);
            if (juceState && Math.abs(juceState.getNormalisedValue() - param.normalized) > 0.0001) {
                juceState.setNormalisedValue(param.normalized);
            }
        }

        // Only emit to event bus if bus and window exist (avoids AudioWorklet crashes)
        if (typeof bus !== 'undefined' && bus.emit) {
            bus.emit(`param:${id}`, {
                value: param.value,
                normalized: param.normalized,
                origin
            });
        }
        // Notify subscribers strictly with normalized [0,1] value (JUCE APVTS style)
        this.listeners.forEach((callback) => callback(id, param.normalized));
    }

    // Notify backend when user grabs a control for automation recording
    beginEdit(id) {
        const juceState = this.juceStates.get(id);
        if (juceState && typeof juceState.sliderDragStarted === 'function') {
            juceState.sliderDragStarted();
        }
    }

    // Notify backend when user releases a control
    endEdit(id) {
        const juceState = this.juceStates.get(id);
        if (juceState && typeof juceState.sliderDragEnded === 'function') {
            juceState.sliderDragEnded();
        }
    }

    setValue(id, rawValue, origin = null) {
        const param = this.parameters.get(id);
        if (!param) return;

        const clamped = Math.max(param.min, Math.min(param.max, rawValue));
        let norm = (clamped - param.min) / (param.max - param.min);

        if (param.skew !== 1) {
            norm = Math.pow(norm, 1 / param.skew);
        }

        this.setNormalized(id, norm, origin);
    }

    getNormalized(id) {
        const param = this.parameters.get(id);
        if (!param) {
            throw new Error(`ParameterStore: Attempted to access unregistered parameter ID: "${id}"`);
        }
        return param.normalized;
    }

    getValue(id) {
        return this.parameters.get(id)?.value ?? 0;
    }

    getFormattedValue(id) {
        const param = this.parameters.get(id);
        if (!param) return '';
        return param.getValueText(param.value);
    }

    getName(id) {
        const param = this.parameters.get(id);
        return param ? param.name : id;
    }

    getState() {
        const state = {};
        for (const [id, param] of this.parameters.entries()) {
            state[id] = param.normalized;
        }
        return state;
    }

    setState(stateObject) {
        for (const [id, normVal] of Object.entries(stateObject)) {
            if (this.parameters.has(id)) {
                this.setNormalized(id, normVal, 'host');
            }
        }
    }

    resetToDefault(id) {
        const param = this.parameters.get(id);
        if (!param) return;
        this.setValue(id, param.defaultValue, 'ui');
    }

    subscribe(callback) {
        this.listeners.add(callback);
        return () => this.listeners.delete(callback);
    }

    copyOscillator(sourceOscNr, targetOscNr) {
        const paramsToCopy = ['SHAPE', 'OCTAVE', 'SEMITONE', 'FINE', 'ROLLOFF', 'PAN', 'LEVEL'];

        for (const paramKey of paramsToCopy) {
            const sourceId = PARAM_IDS?.[`OSC${sourceOscNr}_${paramKey}`] || `osc${sourceOscNr}_${paramKey.toLowerCase()}`;
            const targetId = PARAM_IDS?.[`OSC${targetOscNr}_${paramKey}`] || `osc${targetOscNr}_${paramKey.toLowerCase()}`;

            const sourceNorm = sourceId ? this.getNormalized(sourceId) : undefined;

            if (sourceId && targetId && sourceNorm !== undefined) {
                this.setNormalized(targetId, sourceNorm, 'ui', true);
            } else {
                console.warn(`[ParameterStore] FAILED to copy ${paramKey}. Missing ID or value.`);
            }
        }
    }
}

export const paramStore = new ParameterStore();