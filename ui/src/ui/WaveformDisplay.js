// src/ui/WaveformDisplay.js

import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';

export class WaveformDisplay extends UIComponent {
    constructor(
        id,
        label,
        initialParams = { a: 0.9, shape: 0.5 }
    ) {
        super();

        this.id = id;
        this.label = label;

        this.resolution = 256;
        this.points = new Float32Array(this.resolution);
        this.fixedN = 60;

        this.minAmp = -1;
        this.maxAmp =  1;

        // Initialize as null so setParams detects the initial values as a change
        this.a = null;
        this.shape = null;

        this.setParams(initialParams);
    }

    setParams({ a, shape }) {
        let changed = false;

        if (a !== undefined) {
            const newA = Math.max(0.01, Math.min(0.999, a));
            if (newA !== this.a) {
                this.a = newA;
                changed = true;
            }
        }

        if (shape !== undefined) {
            const newShape = Math.max(0, Math.min(1, shape));
            if (newShape !== this.shape) {
                this.shape = newShape;
                changed = true;
            }
        }

        if (changed) {
            this._computeWaveform();
        }
    }

    getParams() {
        return { a: this.a, shape: this.shape };
    }

    setRolloff(rolloff) {
        this.a = Math.max(0.01, Math.min(0.999, rolloff));
        this._computeWaveform();
    }

    setShape(shape) {
        this.shape = Math.max(0.0, Math.min(1.0, shape));
        this._computeWaveform();
    }

    _updateNormalization() {
        const a = this.a;
        const N = this.N;

        // Angle where the infinite series peaks
        const cosT = Math.min(1.0, (2 * a) / (1 + a * a));
        const sinT = Math.sqrt(Math.max(0, 1 - cosT * cosT));
        const theta = Math.atan2(sinT, cosT);

        // Evaluate the FINITE DSF at that angle
        const aN1 = Math.pow(a, N + 1);
        const sinN  = Math.sin(N * theta);
        const sinNP1 = Math.sin((N + 1) * theta);
        const num = a * sinT + aN1 * (a * sinN - sinNP1);
        const den = 1 + a * a - 2 * a * cosT;
        const singlePeak = Math.abs(num / Math.max(den, 1e-7));

        // Blend + gain
        const blendPeak = 1 + this.amount;
        let pulseGain = 1.0;
        if (this.shape <= 0.5) {
            pulseGain = Math.min(Math.PI / this.offset, 3.0);
        }

        const totalPeak = singlePeak * blendPeak * pulseGain;
        this.normGain = 1.0 / Math.max(totalPeak, 1e-7);
    }

    _computeWaveform() {
        const N  = this.fixedN;
        const NP = N + 1;
        const morph_a  = this.a;
        const powNP = Math.pow(morph_a, NP);
        const a2    = morph_a * morph_a;

        let offset, amount;
        if (this.shape <= 0.5) {
            const t = this.shape * 2;
            offset = Math.PI * t;
            amount = 1;
        } else {
            const t = (this.shape - 0.5) * 2;
            offset = Math.PI;
            amount = 1 - t;
        }
        offset = Math.max(offset, 0.1);

        const gain = (this.shape <= 0.5)
            ? Math.min(Math.PI / Math.max(offset, 0.1), 3.0)
            : 1.0;

        const TWO_PI = Math.PI * 2;
        let maxAbs = 0;

        // Pass 1: compute raw waveform
        for (let i = 0; i < this.resolution; i++) {
            const B = (i / this.resolution) * TWO_PI;

            const denR = Math.max(1e-7, 1 + a2 - 2 * morph_a * Math.cos(B));
            const r = (morph_a * Math.sin(B)
                + powNP * (morph_a * Math.sin(N * B) - Math.sin(NP * B))) / denR;

            const C = B + offset;
            const denQ = Math.max(1e-7, 1 + a2 - 2 * morph_a * Math.cos(C));
            const q = (morph_a * Math.sin(C)
                + powNP * (morph_a * Math.sin(N * C) - Math.sin(NP * C))) / denQ;

            this.points[i] = (r - q * amount) * gain;

            const abs = Math.abs(this.points[i]);
            if (abs > maxAbs) maxAbs = abs;
        }

        // Pass 2: normalize to [-1, 1] by actual peak
        if (maxAbs > 1e-7) {
            for (let i = 0; i < this.resolution; i++) {
                this.points[i] /= maxAbs;
            }
        }

        this.maxAmp =  1.0;
        this.minAmp = -1.0;

        this.isDirty = true;
        this.requestRender();
    }

    drawSelf(ctx) {
        const width  = this.layout.width;
        const height = this.layout.height;

        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(0, 0, width, height);

        ctx.strokeStyle = Theme.colors.panelBorder;
        ctx.lineWidth = 1;
        ctx.strokeRect(0, 0, width, height);

        // ctx.fillStyle = Theme.colors.textSecondary;
        // ctx.font = `${Theme.typography.weightMedium} ${this.scale(10)}px ${Theme.typography.fontFamily}`;
        // ctx.textAlign = 'center';
        // ctx.textBaseline = 'middle';
        //       ctx.fillText(this.label, width / 2, this.scale(10));

        const pad = this.scale(16);
        const gTop = this.scale(4);
        const gBot = height - pad;
        const gL   = pad;
        const gR   = width - pad;
        const gw   = gR - gL;
        const gh   = gBot - gTop;

        // waveform to clip the bottom of the canvas when using fixed [-1, 1] bounds.
        const midY = gTop + gh / 2;

        ctx.fillStyle = 'rgba(0,0,0,0.15)';
        ctx.fillRect(gL, gTop, gw, gh);

        ctx.strokeStyle = Theme.colors.panelBorder;
        ctx.lineWidth = this.scale(1);
        ctx.beginPath();
        ctx.moveTo(gL, midY);
        ctx.lineTo(gR, midY);
        ctx.stroke();

        const ampSpan = this.maxAmp - this.minAmp;
        const yScale  = (ampSpan > 1e-6) ? (gh / ampSpan) : 0;

        ctx.beginPath();
        ctx.moveTo(gL, midY);
        for (let i = 0; i < this.resolution; i++) {
            const px = gL + (i / (this.resolution - 1)) * gw;
            const py = midY - this.points[i] * yScale;
            ctx.lineTo(px, py);
        }
        ctx.lineTo(gR, midY);
        ctx.closePath();

        ctx.save();
        ctx.globalAlpha = 0.15;
        ctx.fillStyle = Theme.colors.waveform;
        ctx.fill();
        ctx.restore();

        ctx.beginPath();
        for (let i = 0; i < this.resolution; i++) {
            const px = gL + (i / (this.resolution - 1)) * gw;
            let py = midY - this.points[i] * yScale;
            py = py > gBot ? gBot : py;
            py = py < gTop ? gTop : py;
            if (i === 0) ctx.moveTo(px, py);
            else         ctx.lineTo(px, py);
        }
        ctx.strokeStyle = Theme.colors.waveform;
        ctx.lineWidth = this.scale(1.7);
        ctx.stroke();

        let shapeLabel;
        if      (this.shape < 0.15) shapeLabel = 'Pulse';
        else if (this.shape < 0.4)  shapeLabel = 'Pulse→Sq';
        else if (this.shape < 0.6)  shapeLabel = 'Square';
        else if (this.shape < 0.85) shapeLabel = 'Sq→Saw';
        else                        shapeLabel = 'Sawtooth';

        ctx.fillStyle = Theme.colors.textSecondarya;
        ctx.font = `${Theme.typography.weightMedium} ${this.scale(10)}px ${Theme.typography.fontFamily}`;
        ctx.textAlign = 'right';
        ctx.textBaseline = 'top';
        ctx.fillText(shapeLabel, gR, gBot + this.scale(4));
    }
}