// src/main.js
import { SynthApp } from './ui/SynthApp.js';

function bootstrap() {

    // Locate DOM targets
    const canvas = document.getElementById('pluginCanvas');
    const wrapper = document.getElementById('synth-wrapper');

    if (!canvas || !wrapper) {
        console.error('Synth mounting failed: #pluginCanvas or #synth-wrapper missing in DOM');
        return;
    }

    // Mount UI App
    const app = new SynthApp(canvas, wrapper);

    // (Optional) Expose app globally for debugging in development
    // window.__SYNTH_APP__ = app;
}

// Ensure DOM is ready before bootstrapping
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', bootstrap);
} else {
    bootstrap();
}