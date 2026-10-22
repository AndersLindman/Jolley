// src/panels/HeaderPanel.js

import { PARAM_IDS } from '../core/paramIds.js'
import { GridContainer } from '../ui/GridContainer.js';
import { AlignContainer } from '../ui/AlignContainer.js';
import { PresetDropdown } from '../ui/PresetDropdown.js';
import { Theme } from "../theme.js";
import { Label } from '../ui/Label.js';
import { Knob } from '../ui/Knob.js';
import { createKnob } from '../ui/createKnob.js';
//import { Button } from '../ui/Button.js';

export class HeaderPanel extends GridContainer {
    constructor() {
        console.log('[HeaderPanel] Constructor started');
        super({
            cols: 4,
            rows: 1,
            padding: 0,
            gap: 0,
            weightsX: [0.34, 0.24, 0.26]
        });

        // Left-aligned label container
        const alignedLabel = new AlignContainer({ alignX: 'left' });
        alignedLabel.add(new Label('Jolley',
            {
                align: 'left',
                size: Theme.typography.headerSize,
                color: Theme.colors.textHeading}));
        this.add(alignedLabel);

        // Presets
        this.presetDropdown = new PresetDropdown(1, 'Preset');
        
        this.add(this.presetDropdown);

        // Spacer
        this.add(new GridContainer());
        // Temporary export button
        // const exportButton = new Button('Export Current State', () => {
        //     const currentState = presetManager.exportState();
        //    
        //    // Print formatted JSON directly to the browser console
        //    const jsonString = JSON.stringify(currentState, null, 2);
        //    console.log('[Debug Export]', jsonString);
        // });
        // this.add(exportButton);

        // Buttons container
        const buttonsGrid = new GridContainer({ cols: 2, rows: 1, weightsX: [0.5, 0.5] });
        this.add(buttonsGrid);

        // Tube drive knob
        const tubeGrid = new GridContainer(
            { cols: 3, rows: 1, weightsX: [0.6, 0.2]});
        const tubeLabel = new Label(
            'Tube drive', {
                align: 'right', size: Theme.typography.smallSize});
        tubeGrid.add(tubeLabel);
        const tubeKnob = createKnob(PARAM_IDS.TUBE_DRIVE, '', {size: Knob.SIZES.small});
        tubeGrid.add(tubeKnob);
        buttonsGrid.add(tubeGrid);

        // Volume knob
        const volumeGrid = new GridContainer(
            { cols: 3, rows: 1, weightsX: [0.6, 0.2]});
        const volumeLabel = new Label(
            'Volume', {
                align: 'right', size: Theme.typography.smallSize});
        volumeGrid.add(volumeLabel);
        const volumeKnob = createKnob(PARAM_IDS.MASTER_VOLUME, '', {size: Knob.SIZES.small});
        volumeGrid.add(volumeKnob);
        buttonsGrid.add(volumeGrid);
    }
}