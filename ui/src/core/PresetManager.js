import { bus } from '../eventBus.js';
import { FACTORY_PRESETS } from './FactoryPresets.js';
import { paramStore } from './ParameterStore.js'; // <-- Direct import, no constructor injection needed!

export class PresetManager {
    constructor() {
        this.factoryPresets = FACTORY_PRESETS;
    }

    exportState() {
        const parameters = {};

        for (const [id, param] of paramStore.parameters.entries()) {
            parameters[id] = param.value;
        }

        return {
            version: '1.0.0',
            timestamp: Date.now(),
            parameters
        };
    }

    loadState(stateOrJson) {
        try {
            const preset = typeof stateOrJson === 'string'
                ? JSON.parse(stateOrJson)
                : stateOrJson;

            const parameters = preset.parameters || preset;

            for (const [id, value] of Object.entries(parameters)) {
                if (paramStore.parameters.has(id)) {
                    // Use 'ui' as origin so the new preset values are sent to the C++ backend
                    paramStore.setValue(id, value, 'ui');
                }
            }

            bus.emit('preset:loaded', preset);
        } catch (err) {
            console.error('[PresetManager] Failed to load preset:', err);
        }
    }

    loadFactory(name) {
        const index = this.factoryPresets.findIndex(
            preset => preset.name === name
        );

        if (index === -1) {
            console.warn(`[PresetManager] Factory preset "${name}" not found.`);
            return false;
        }

        const preset = this.factoryPresets[index];
        this.loadState(preset.values);

        // Update the preset index UI control if it exists
        if (paramStore.parameters.has('preset_index') && this.factoryPresets.length > 1) {
            const norm = index / (this.factoryPresets.length - 1);
            paramStore.setNormalized('preset_index', norm, 'ui', true);
        }

        return true;
    }

    saveToLocalStorage(slotName) {
        const state = this.exportState();
        localStorage.setItem(`synth_preset_${slotName}`, JSON.stringify(state));
        bus.emit('preset:saved', slotName);
    }

    loadFromLocalStorage(slotName) {
        const raw = localStorage.getItem(`synth_preset_${slotName}`);
        if (raw) {
            this.loadState(raw);
            return true;
        }
        return false;
    }

    getFactoryNames() {
        return this.factoryPresets.map(preset => preset.name);
    }
}

// Export the singleton instance
export const presetManager = new PresetManager();