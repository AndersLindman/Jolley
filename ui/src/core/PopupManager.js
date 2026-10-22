// src/core/PopupManager.js

class PopupManager {
    constructor() {
        this.activePopup = null;
    }

    setPopup(component) {
        this.activePopup = component;
    }

    close() {
        if (this.activePopup) {
            this.activePopup.isOpen = false;
            this.activePopup = null;
        }
    }

    handlePointerDown(canvasX, canvasY, event) {
        if (this.activePopup && typeof this.activePopup.handlePointerDown === 'function') {
            this.activePopup.handlePointerDown(canvasX, canvasY, event);
            return true;
        }
        return false;
    }

    handlePointerMove(canvasX, canvasY, event) {
        if (this.activePopup && typeof this.activePopup.handlePointerMove === 'function') {
            this.activePopup.handlePointerMove(canvasX, canvasY, event);
            return true;
        }
        return false;
    }

    paintOverlay(ctx, dpr = 1) {
        if (this.activePopup && typeof this.activePopup.paintOverlay === 'function') {
            ctx.save();
            // Apply DPR scaling so the overlay renders in logical space
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
            this.activePopup.paintOverlay(ctx);
            ctx.restore();
        }
    }
}

export const popupManager = new PopupManager();