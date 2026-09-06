#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rb {

constexpr int kSteps = 16;
constexpr int kPatterns = 8;
constexpr int kVoice808 = 13;
constexpr int kVoice909 = 11;
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;

enum class Voice808 : int {
    BD, SD, LT, MT, HT, RS, CP, CB, CY, OH, CH, MA, CL
};

enum class Voice909 : int {
    BD, SD, LT, MT, HT, RS, HC, CH, OH, CR, RD
};

inline const char* name808(int i) {
    static const char* n[] = {"BD", "SD", "LT", "MT", "HT", "RS", "CP",
                              "CB", "CY", "OH", "CH", "MA", "CL"};
    return n[i];
}

inline const char* name909(int i) {
    static const char* n[] = {"BD", "SD", "LT", "MT", "HT", "RS", "HC",
                              "CH", "OH", "CR", "RD"};
    return n[i];
}

inline float clampf(float x, float lo, float hi) {
    return std::min(hi, std::max(lo, x));
}

inline float midiToHz(float note) {
    return 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline float fastTanh(float x) {
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline int noteNameIndex(int midi, char* out, int cap) {
    static const char* names[] = {"C", "C#", "D", "D#", "E", "F",
                                  "F#", "G", "G#", "A", "A#", "B"};
    int n = midi;
    if (n < 0) n = 0;
    int oct = n / 12 - 1;
    int pc = n % 12;
    if (cap < 5) return 0;
    const char* s = names[pc];
    int i = 0;
    out[i++] = s[0];
    if (s[1]) out[i++] = s[1];
    if (oct < 0) {
        out[i++] = '-';
        out[i++] = static_cast<char>('0' + -oct);
    } else {
        out[i++] = static_cast<char>('0' + oct);
    }
    out[i] = 0;
    return i;
}

struct AcidStep {
    bool gate = false;
    bool accent = false;
    bool slide = false;
    uint8_t note = 36;
};

struct AcidPattern {
    AcidStep steps[kSteps];
};

struct DrumPattern {
    bool trig[kVoice808][kSteps]{};
};

struct DrumPattern909 {
    bool trig[kVoice909][kSteps]{};
};

struct Pattern {
    AcidPattern acid[2];
    DrumPattern d808;
    DrumPattern909 d909;
    float pcf[kSteps];
};

struct AcidParams {
    float tune = 0.5f;      // 0..1 => -12..+12 semitones
    float cutoff = 0.28f;
    float resonance = 0.72f;
    float envMod = 0.62f;
    float decay = 0.38f;
    float accent = 0.55f;
    float volume = 0.72f;
    bool square = false;
};

struct DrumVoiceParams {
    float level = 0.7f;
    float tune = 0.5f;
    float decay = 0.5f;
    float tone = 0.5f; // snare snappy / hat tone
};

struct DrumKitParams {
    DrumVoiceParams voice[16];
    float volume = 0.85f;
};

struct FXParams {
    bool distOn = true;
    float distDrive = 0.45f;
    float distTone = 0.55f;
    float distMix = 0.35f;

    bool delayOn = true;
    float delayTime = 0.375f; // 0=1/16, 0.25=1/8, 0.5=1/8d, 0.75=1/4, 1=1/2
    float delayFb = 0.38f;
    float delayMix = 0.22f;

    bool pcfOn = false;
    float pcfCutoff = 0.55f;
    float pcfRes = 0.25f;
    float pcfAmount = 0.7f;

    bool compOn = true;
    float compThresh = 0.55f;
    float compRatio = 0.45f;
    float compGain = 0.42f;
};

struct MixParams {
    float acid[2] = {0.85f, 0.7f};
    float d808 = 0.9f;
    float d909 = 0.8f;
    float master = 0.8f;
    bool mute[4] = {false, false, false, false};
};

struct AppParams {
    AcidParams acid[2];
    DrumKitParams kit808;
    DrumKitParams kit909;
    FXParams fx;
    MixParams mix;
    float bpm = 128.0f;
    float shuffle = 0.12f;
    int pattern = 0;
    Pattern bank[kPatterns];
};

} // namespace rb
