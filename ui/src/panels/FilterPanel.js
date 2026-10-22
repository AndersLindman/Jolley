// src/panels/FilterPanel.js

import { GridContainer } from '../ui/GridContainer.js';
import { LFO } from '../ui/LFO.js';
import {Filter} from '../ui/Filter.js';
import {Voice} from '../ui/Voice.js';

export class FilterPanel extends GridContainer {
    constructor() {
        super({
            cols: 3,
            rows: 1,
            padding: 10,
            gap: 0
        });

        this.add(new LFO());
        this.add(new Filter());
        this.add(new Voice());
    }
}