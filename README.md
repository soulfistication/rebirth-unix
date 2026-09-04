# Acid Rack for UNIX
------------------

A UNIX C++ groovebox inspired by Propellerhead **ReBirth RB-338**: two TB-303-style bass synths, analog-modelled TR-808 and TR-909 kits, a 16-step sequencer, and a small FX section.

This is an original software instrument. It is not affiliated with Roland, Reason Studios, or Propellerhead.

<img width="1289" height="800" alt="rebirth" src="https://github.com/user-attachments/assets/2804e912-8ebf-4956-8a8d-42a4d72ff9c2" />

## Layout

| Section | What it does |
| --- | --- |
| **303 A / 303 B** | Saw or square oscillator, diode-ladder filter, accent, slide, tune / cutoff / res / env mod / decay |
| **808** | BD, SD, toms, rim, clap, cowbell, cymbal, hats, maracas, claves |
| **909** | BD, SD, toms, rim, clap, hats, crash, ride |
| **Sequencer** | 16 steps, 8 patterns, shuffle, tempo 60–200 BPM |
| **FX** | Distortion, tempo-sync delay, pattern-controlled filter (PCF), compressor |

Drum and bass voices are synthesized (no sample packs). Pattern 1 is a ready-to-play acid loop.

## Build (Linux, macOS, BSD)

Needs a C++17 compiler and [SDL2](https://www.libsdl.org/).

```bash
# Debian / Ubuntu / Mint
sudo apt install build-essential cmake libsdl2-dev

# Fedora
sudo dnf install gcc-c++ cmake SDL2-devel

# Arch
sudo pacman -S base-devel cmake sdl2

# macOS
brew install cmake sdl2

# FreeBSD
sudo pkg install cmake sdl2
```

CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/rebirth
```

Or Make:

```bash
make
./rebirth
```

## Controls

| Input | Action |
| --- | --- |
| **Space** | Play / stop |
| **1–8** | Select pattern |
| **[ ]** or **− / +** | Tempo |
| **H** | Help overlay |
| **Esc** | Close help, or quit |
| Drag knobs vertically | Adjust (mouse wheel for fine control) |
| 303 step click | Toggle gate |
| Shift + click | Accent |
| Alt or right-click | Slide |
| Wheel on 303 step | Change pitch (C1–C4) |
| 808 / 909 pads | Audition and select a voice |
| Drum steps | Toggle hits for the selected voice |
| PCF bars | Drag a 16-step cutoff sequence |

Start with pattern 1, raise **RES** and **ENV MOD** on 303 A, drop **CUTOFF**, and press Play.

## Architecture

```
src/
  tb303.cpp      PolyBLEP oscillator + 4-pole ladder + accent/slide
  drums.cpp      Analog 808 / 909 voice models
  sequencer.cpp  16th-note clock, shuffle, demo patterns
  fx.cpp         Distortion, delay, PCF, compressor
  engine.cpp     Mix, limiter, audio callback
  ui.cpp         SDL2 hardware-panel GUI
```

Audio is stereo float32 at 44.1 kHz through SDL2 (CoreAudio, PulseAudio, ALSA, or PipeWire depending on the OS).
