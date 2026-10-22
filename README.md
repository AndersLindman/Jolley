
# Jolley

A software synthesizer and VST3 plugin built with C++ (JUCE) and a web-based UI.
The name Jolley comes from the discrete summation formula by L.B.W. Jolley used in the oscillators.

---

## Features

* **3 oscillators**: Clean oscillators with clear hard sync.
* **Polyphony**: Polyphony with 8 voices.
* **Stereo filter**: Smooth filter with 3 filter types, drive and feedback.
* **Unison**: Up to 8 voices of unison.
* **FM matrix**: Simple and effective FM modulation.
* **Intuitive UI**: User interface with direct access to all settings.

---

## Implementation

* **High-Performance C++ DSP Engine**: Custom voice architecture featuring polyphonic voice allocation, voice stealing, unison layers, and FM processing.
* **Modern Web GUI**: Responsive frontend interface rendered via native webview bindings, supporting live development workflows.
* **Robust Parameter Management**: Fully integrated with JUCE's Audio Processor Value Tree State (APVTS) for seamless DAW automation and preset handling.
* **Multiple Formats**: Compiles as a standard VST3 plugin and a standalone desktop application.

---

## Tech Stack

* **Backend / Audio Engine**: C++, JUCE Framework, CMake
* **Frontend UI**: HTML5, CSS3, JavaScript, Vite
* **Host Compatibility**: Tested with Reaper and SAVIHost

---

## Building from Source

### Prerequisites

* A C++ compiler supporting C++20 (Visual Studio 2022 / MSBuild on Windows, Xcode on macOS, or GCC/Clang on Linux)
* [CMake](https://cmake.org/) (version 3.22 or higher)
* The [JUCE Framework](https://juce.com/)

### Build Instructions

1. Clone the repository with submodules (if applicable):
```bash
git clone https://github.com/AndersLindman/Jolley.git
cd Jolley

```


2. Configure the project with CMake:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release

```


3. Build the binaries:
```bash
cmake --build build --config Release

```



The compiled VST3 bundle and standalone executable will be output in the `build/Jolley_artefacts/Release/` directory.

---

## Development & Testing

* **Fast Testing**: Drop `savihost.exe` into the VST3 bundle contents folder (`Contents/x86_64-win/`), rename it to `Jolley.exe`, and launch it for instant audio/MIDI routing checks.
* **UI Hot-Reloading**: Run your frontend development server (via Vite) alongside your debug build to iterate on UI changes in real-time.

---

## License

This project is open-source and distributed under the terms of the **GNU General Public License v3.0 (GPLv3)**. See the [LICENSE](LICENSE) file for details.
