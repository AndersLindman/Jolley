// ui/ADSREnvelope.js
import { UIComponent } from './UIComponent.js';
import { Theme } from '../theme.js';

export class ADSREnvelope extends UIComponent {
    static MIN_X_GAP = 0.02;

    constructor(
        id,
        label,
        initialValue = {
            attack: 0.2,
            attackSlope: 3,
            decay: 0.2,
            decaySlope: 3,
            sustain: 0.5,
            release: 0.3,
            releaseSlope: 3
        }
    ) {
        super();

        this.id = id;
        this.label = label;
        this.changeCallback = null;

        this.attackX  = 0.2;
        this.decayX   = 0.4;
        this.sustainY = 0.5;
        this.releaseX = 0.7;

        this.attackCurve  = 3;
        this.decayCurve   = 3;
        this.releaseCurve = 3;

        this.setValue(initialValue);
        this.activePoint = null;
    }

    onChange(cb) {
        this.changeCallback = cb;
    }

    _notifyChange() {
        if (typeof this.changeCallback === 'function') {
            // Pass the entire computed value object (attack, decay, sustain, release, slopes)
            this.changeCallback(this.getValue());
        }
    }

    computeLayout(x, y, width, height, rootWidth) {
        super.computeLayout(x, y, width, height, rootWidth);
        this.width = width;
        this.height = height;
    }

    getAttack()       { return this.attackX; }
    getAttackSlope()  { return this.attackCurve; }
    getDecay()        { return this.decayX - this.attackX; }
    getDecaySlope()   { return this.decayCurve; }
    getSustain()      { return this.sustainY; }
    getRelease()      { return 1.0 - this.releaseX; }
    getReleaseSlope() { return this.releaseCurve; }

    getValue() {
        return {
            attack:       this.getAttack(),
            attackSlope:  this.getAttackSlope(),
            decay:        this.getDecay(),
            decaySlope:   this.getDecaySlope(),
            sustain:      this.getSustain(),
            release:      this.getRelease(),
            releaseSlope: this.getReleaseSlope()
        };
    }

    setValue(params = {}) {
        const {
            attack, decay, sustain, release,
            attackSlope, decaySlope, releaseSlope,
            attackCurve, decayCurve, releaseCurve, curve
        } = params;

        const MIN_GAP = ADSREnvelope.MIN_X_GAP;

        let a = Math.max(MIN_GAP, Math.min(1, attack ?? this.getAttack()));
        let d = Math.max(MIN_GAP, Math.min(1, decay ?? this.getDecay()));
        let s = Math.max(0, Math.min(1, sustain ?? this.getSustain()));
        let r = Math.max(MIN_GAP, Math.min(1, release ?? this.getRelease()));

        const maxSum = 1.0 - MIN_GAP;
        const sum = a + d + r;

        if (sum > maxSum) {
            const excess = sum - maxSum;
            const flexA = a - MIN_GAP;
            const flexD = d - MIN_GAP;
            const flexR = r - MIN_GAP;
            const totalFlex = flexA + flexD + flexR;

            if (totalFlex > 0) {
                const scale = (totalFlex - excess) / totalFlex;
                a = MIN_GAP + flexA * Math.max(0, scale);
                d = MIN_GAP + flexD * Math.max(0, scale);
                r = MIN_GAP + flexR * Math.max(0, scale);
            } else {
                a = MIN_GAP; d = MIN_GAP; r = MIN_GAP;
            }
        }

        this.attackX  = a;
        this.decayX   = a + d;
        this.sustainY = s;
        this.releaseX = 1.0 - r;

        const clamp = v => Math.max(-20, Math.min(20, v));

        if (curve !== undefined) {
            const c = clamp(curve);
            this.attackCurve = c; this.decayCurve = c; this.releaseCurve = c;
        }

        const aSlope = attackSlope ?? attackCurve;
        const dSlope = decaySlope ?? decayCurve;
        const rSlope = releaseSlope ?? releaseCurve;

        if (aSlope !== undefined) this.attackCurve  = clamp(aSlope);
        if (dSlope !== undefined) this.decayCurve   = clamp(dSlope);
        if (rSlope !== undefined) this.releaseCurve = clamp(rSlope);

        this.isDirty = true;
    }

    setAttackX(v)  {
        this.attackX  = Math.max(ADSREnvelope.MIN_X_GAP, Math.min(this.decayX - ADSREnvelope.MIN_X_GAP, v));
        this.isDirty = true;
    }
    setDecayX(v)   {
        this.decayX   = Math.max(this.attackX + ADSREnvelope.MIN_X_GAP, Math.min(this.releaseX - ADSREnvelope.MIN_X_GAP, v));
        this.isDirty = true;
    }
    setSustainY(v) {
        this.sustainY = Math.max(0, Math.min(1, v));
        this.isDirty = true;
    }
    setReleaseX(v) {
        this.releaseX = Math.max(this.decayX + ADSREnvelope.MIN_X_GAP, Math.min(1.0 - ADSREnvelope.MIN_X_GAP, v));
        this.isDirty = true;
    }

    _getCurveY(x, p) {
        if (Math.abs(p) < 1e-4) return 1 - x;
        return (Math.exp(-p * x) - Math.exp(-p)) / (1 - Math.exp(-p));
    }

    _curveFromMidpoint(target) {
        let lo = -20, hi = 20;
        for (let i = 0; i < 40; i++) {
            const mid = (lo + hi) * 0.5;
            if (this._getCurveY(0.5, mid) > target) lo = mid;
            else hi = mid;
        }
        return (lo + hi) * 0.5;
    }

    handlePointerDown(canvasX, canvasY, event) {
        const { localX, localY } = this.canvasToLocal(canvasX, canvasY);
        const sc = px => this.scale(px);

        const padding     = sc(16);
        const graphTop    = sc(24);
        const graphBottom = (this.height ?? this.layout.height) - padding;
        const graphLeft   = padding;
        const graphRight  = (this.width ?? this.layout.width) - padding;
        const gw = graphRight - graphLeft;
        const gh = graphBottom - graphTop;
        if (gw <= 0 || gh <= 0) return;

        const p1x = graphLeft + this.attackX  * gw, p1y = graphTop;
        const p2x = graphLeft + this.decayX   * gw, p2y = graphBottom - this.sustainY * gh;
        const p3x = graphLeft + this.releaseX * gw, p3y = p2y;

        const dots = this._curveDotPositions(
            p1x, p1y, p2x, p2y, p3x, p3y,
            graphBottom, gh, graphLeft, graphRight
        );

        const hitRadius = sc(14);
        this.activePoint = null;

        let minDist = Infinity;
        const checkHit = (name, x, y) => {
            const dist = Math.hypot(localX - x, localY - y);
            if (dist <= hitRadius && dist < minDist) {
                minDist = dist;
                this.activePoint = name;
            }
        };

        checkHit('attackCurve', dots.attack.x, dots.attack.y);
        checkHit('decayCurve', dots.decay.x, dots.decay.y);
        checkHit('releaseCurve', dots.release.x, dots.release.y);
        checkHit('attack', p1x, p1y);
        checkHit('decay', p2x, p2y);
        checkHit('release', p3x, p3y);

        if (this.activePoint) {
            this.startAttackX  = this.attackX;
            this.startDecayX   = this.decayX;
            this.startSustainY = this.sustainY;
            this.startReleaseX = this.releaseX;
            this.startLocalX   = localX;
            this.startLocalY   = localY;
        }
    }

    handlePointerMove(canvasX, canvasY, event) {
        if (!this.activePoint) return;
        const { localX, localY } = this.canvasToLocal(canvasX, canvasY);
        const sc = px => this.scale(px);

        const padding     = sc(16);
        const graphTop    = sc(24);
        const graphBottom = (this.height ?? this.layout.height) - padding;
        const graphLeft   = padding;
        const graphRight  = (this.width ?? this.layout.width) - padding;
        const gw = graphRight - graphLeft;
        const gh = graphBottom - graphTop;
        if (gw <= 0 || gh <= 0) return;

        if (this.activePoint === 'attackCurve' ||
            this.activePoint === 'decayCurve'  ||
            this.activePoint === 'releaseCurve') {

            const amp = Math.max(0, Math.min(1, (graphBottom - localY) / gh));
            let target;

            if (this.activePoint === 'attackCurve') {
                target = 1 - amp;
            } else if (this.activePoint === 'decayCurve') {
                target = (1 - this.sustainY > 0.001)
                    ? (amp - this.sustainY) / (1 - this.sustainY)
                    : 0.5;
            } else {
                target = (this.sustainY > 0.001)
                    ? amp / this.sustainY
                    : 0.5;
            }

            target = Math.max(0.01, Math.min(0.99, target));
            const p = this._curveFromMidpoint(target);

            if (this.activePoint === 'attackCurve')  this.attackCurve  = p;
            if (this.activePoint === 'decayCurve')   this.decayCurve   = p;
            if (this.activePoint === 'releaseCurve') this.releaseCurve = p;
            this.isDirty = true;
            this.requestRender();
            this._notifyChange();
            return;
        }

        const dNx =  (localX - this.startLocalX) / gw;
        const dNy = -(localY - this.startLocalY) / gh;

        if (this.activePoint === 'attack')       this.setAttackX(this.startAttackX + dNx);
        else if (this.activePoint === 'decay') { this.setDecayX(this.startDecayX + dNx); this.setSustainY(this.startSustainY + dNy); }
        else if (this.activePoint === 'release') this.setReleaseX(this.startReleaseX + dNx);

        this.requestRender();
        this._notifyChange();
    }

    handlePointerUp() {
        this.activePoint = null;
    }

    _curveDotPositions(p1x, p1y, p2x, p2y, p3x, p3y, graphBottom, gh, graphLeft, graphRight) {
        const t = 0.5;
        const aAmp = 1 - this._getCurveY(t, this.attackCurve);
        const dAmp = this.sustainY + (1 - this.sustainY) * this._getCurveY(t, this.decayCurve);
        const rAmp = this.sustainY * this._getCurveY(t, this.releaseCurve);

        return {
            attack:  { x: graphLeft + (p1x - graphLeft) * t, y: graphBottom - aAmp * gh },
            decay:   { x: p1x + (p2x - p1x) * t,             y: graphBottom - dAmp * gh },
            release: { x: p3x + (graphRight - p3x) * t,      y: graphBottom - rAmp * gh }
        };
    }

    drawSelf(ctx) {
        const sc = px => this.scale(px);
        const w = this.width ?? this.layout.width;
        const h = this.height ?? this.layout.height;

        ctx.fillStyle = Theme.colors.panelBackground;
        ctx.fillRect(0, 0, w, h);
        ctx.strokeStyle = Theme.colors.panelBorder;
        ctx.lineWidth = sc(1);
        ctx.strokeRect(0, 0, w, h);

        ctx.fillStyle = Theme.colors.textSecondary;
        ctx.font = `${Theme.typography.weightMedium} ${sc(Theme.typography.adsrLabelSize)}px ${Theme.typography.fontFamily}`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(this.label, w / 2, sc(12));

        const pad = sc(16);
        const gTop = sc(24), gBot = h - pad;
        const gL   = pad, gR = w - pad;
        const gw = gR - gL, gh = gBot - gTop;

        const p0x = gL, p0y = gBot;
        const p1x = gL + this.attackX  * gw, p1y = gTop;
        const p2x = gL + this.decayX   * gw, p2y = gBot - this.sustainY * gh;
        const p3x = gL + this.releaseX * gw, p3y = p2y;
        const p4x = gR, p4y = gBot;

        const path = new Path2D();
        path.moveTo(p0x, p0y);

        const seg = Math.round(sc(64));

        for (let i = 1; i <= seg; i++) {
            const t = i / seg;
            const amp = 1 - this._getCurveY(t, this.attackCurve);
            path.lineTo(p0x + (p1x - p0x) * t, gBot - amp * gh);
        }
        for (let i = 1; i <= seg; i++) {
            const t = i / seg;
            const amp = this.sustainY + (1 - this.sustainY) * this._getCurveY(t, this.decayCurve);
            path.lineTo(p1x + (p2x - p1x) * t, gBot - amp * gh);
        }
        path.lineTo(p3x, p3y);
        for (let i = 1; i <= seg; i++) {
            const t = i / seg;
            const amp = this.sustainY * this._getCurveY(t, this.releaseCurve);
            path.lineTo(p3x + (p4x - p3x) * t, gBot - amp * gh);
        }
        path.lineTo(p0x, p0y);

        ctx.save();
        ctx.globalAlpha = 0.15;
        ctx.fillStyle = Theme.colors.adsr;
        ctx.fill(path);
        ctx.restore();

        ctx.strokeStyle = Theme.colors.adsr;
        ctx.lineWidth = sc(1.7);
        ctx.stroke(path);

        const mainDots = [{ x: p1x, y: p1y }, { x: p2x, y: p2y }, { x: p3x, y: p3y }];
        ctx.fillStyle = Theme.colors.knobBody;
        ctx.strokeStyle = Theme.colors.knobBorder;
        ctx.lineWidth = sc(1.5);
        for (const d of mainDots) {
            ctx.beginPath();
            ctx.arc(d.x, d.y, sc(5), 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
        }

        const cd = this._curveDotPositions(p1x, p1y, p2x, p2y, p3x, p3y, gBot, gh, gL, gR);
        ctx.fillStyle = Theme.colors.adsr;
        ctx.strokeStyle = Theme.colors.panelBackground;
        ctx.lineWidth = sc(1.5);
        for (const d of [cd.attack, cd.decay, cd.release]) {
            ctx.beginPath();
            ctx.arc(d.x, d.y, sc(4), 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
        }
    }
}