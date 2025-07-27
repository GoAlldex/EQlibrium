# Project Overview

The EQlibrium application was developed for educational purposes and will no longer be supported with extensions.

## Key Features
1. Equlizer visual for each audio channel L/R(filter graph, frequency graph, VU-Meter)
2. Equlizer notch, lowpass and highpass (lowpass + highpass = bandpass)
3. Adjustable gain for each channel L/R
4. Audio recording (with filter and background audio)
5. Audio player with repeat for .wav and .mp3 files and L/R channel waveform graph
6. Disable function for L/R channel

---

## Requierments
1. JUCE Framework (clone JUCE framework to extern/JUCE/)
2. CMake 3.3 or greater

---

## Build System

This project uses CMake as the primary build system. Make sure you execute cmake in admin mode (otherwise VST3 may not compile). To build the project:

1. Clone the repository.
2. Navigate to the project root directory:
   ```bash
   cd EQlibrium
   ```
3. Create a build directory and navigate to it:
   ```bash
   mkdir build && cd build
   ```
4. Configure the project using CMake:
   ```bash
   cmake ..
   ```
5. Build the project:
   ```bash
   cmake --build . --config Release --target EQlibrium_Standalone -j [CPU thread count]
   ```
   or
   ```bash
   cmake --build . --config Release --target EQlibrium_VST3 -j [CPU thread count]
   ```

---

## Dependencies

EQlibrium relies on the following key dependencies:

1. **JUCE Framework**: For audio processing and UI management.

Dependencies are managed using `CMake` for seamless integration. Ensure all dependencies are installed or accessible
during the configuration process.

---

## License

Refer to the `LICENSE` files for more details on licensing terms.