#pragma once

#include "dsp.hpp"
#include "types.hpp"

namespace rb {

class AnalogKit {
public:
    enum class Kind { Kit808, Kit909 };

    void setSampleRate(float sr);
    void setKind(Kind k) { kind_ = k; }
    void setParams(const DrumKitParams& p) { params_ = p; }
    void trigger808(int voice, float velocity = 1.0f);
    void trigger909(int voice, float velocity = 1.0f);
    float process();

private:
    struct Voice {
        int mode = 0;
        float phase[6]{};
        float freq[6]{};
        float amp = 0, ampD = 0;
        float amp2 = 0, amp2D = 0;
        float pitch = 0, pitchD = 0;
        float baseF = 0;
        float lp = 0, hp = 0, bp = 0;
        float click = 0, clickD = 0;
        int clapN = 0, clapT = 0;
        float gain = 0;
        float tone = 0.5f;
        int metallic = 0;
        float last = 0;
    };

    void trigSineKick(Voice& v, float startHz, float endHz, float decay, float click, float gain);
    void trigTom(Voice& v, float hz, float decay, float gain);
    void trigSnare(Voice& v, float toneHz, float decay, float snappy, float gain, bool nine);
    void trigHat(Voice& v, float decay, float gain, bool open, bool nine);
    void trigClap(Voice& v, float decay, float gain);
    void trigCowbell(Voice& v, float decay, float gain);
    void trigRim(Voice& v, float gain, bool nine);
    void trigCym(Voice& v, float decay, float gain, bool ride);
    void trigNoisePerc(Voice& v, float decay, float hp, float gain);
    void trigClave(Voice& v, float gain);

    float processVoice(Voice& v);

    float sr_ = 44100.0f;
    Kind kind_ = Kind::Kit808;
    DrumKitParams params_{};
    Voice voices_[16];
};

} // namespace rb
