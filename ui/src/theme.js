// src/theme.js

export const Theme = {
    colors: {
        // Faceplate & Panels
        background: '#181818',
        panelBackground: '#181818',
        panelBorder: '#282828',

        // Typography
        textPrimary: '#c0c0c0',
        textSecondary: '#777777',
        textHighlight: '#ff9900',
        textHeading: '#666666',
        textMuted: '#888888',

        // Hardware Elements (Knobs, Sliders, LEDs)
        knobBody: '#282828',
        knobBodyDark: '#181818',
        knobBorder: '#404040',
        knobBorderdark: '#282828',

        knobBody2: '#707070',
        knobBodyDark2: '#606060',
        knobBorder2: '#909090',
        knobBorderdark2: '#686868',
        knobIndicator: '#c0c0c0',
        knobIndicator2: '#e8e8e8',

        buttonText: '#00FFCC',

        ledOff: '#2a1a00',
        ledOn: '#f09000',

        waveform: '#ff9900',
        adsr: '#ff9000',

        // Shadows & Depth
        shadowColor: 'rgba(0, 0, 0, 0.4)'
    },

    typography: {
        fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif',
        smallSize: 11,
        labelSize: 14,
        titleSize: 16,
        headerSize: 18,
        adsrLabelSize: 14,
        weightMedium: '500',
        weightBold: '700'
    },

    metrics: {
        panelBorderRadius: 4,
        knobShadowOffsetY: 20
    }
};