#include "drums.hpp"

#include <algorithm>

namespace rb {

void AnalogKit::setSampleRate(float sr) { sr_ = sr; }

void AnalogKit::trigSineKick(Voice& v, float startHz, float endHz, float decay, float click, float gain) {
    v.mode = 1;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (decay * sr_));
    v.baseF = endHz;
    v.pitch = startHz / endHz;
    v.pitchD = std::exp(-1.0f / (0.045f * sr_));
    v.phase[0] = 0.0f;
    v.click = click;
    v.clickD = std::exp(-1.0f / (0.0045f * sr_));
    v.gain = gain;
    v.lp = 0;
}

void AnalogKit::trigTom(Voice& v, float hz, float decay, float gain) {
    v.mode = 1;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (decay * sr_));
    v.baseF = hz;
    v.pitch = 1.8f;
    v.pitchD = std::exp(-1.0f / (0.06f * sr_));
    v.phase[0] = 0.0f;
    v.click = 0.25f;
    v.clickD = std::exp(-1.0f / (0.003f * sr_));
    v.gain = gain;
}

void AnalogKit::trigSnare(Voice& v, float toneHz, float decay, float snappy, float gain, bool nine) {
    v.mode = 2;
    v.amp = 0.85f;
    v.ampD = std::exp(-1.0f / ((nine ? 0.18f : 0.22f) * decay * sr_));
    v.amp2 = snappy;
    v.amp2D = std::exp(-1.0f / ((nine ? 0.08f : 0.12f) * decay * sr_));
    v.baseF = toneHz;
    v.pitch = nine ? 1.15f : 1.4f;
    v.pitchD = std::exp(-1.0f / (0.03f * sr_));
    v.phase[0] = 0;
    v.phase[1] = 0.25f;
    v.gain = gain;
    v.tone = snappy;
    v.lp = 0;
    v.hp = 0;
}

void AnalogKit::trigHat(Voice& v, float decay, float gain, bool open, bool nine) {
    v.mode = 3;
    v.metallic = nine ? 1 : 0;
    v.amp = 1.0f;
    const float d = open ? lerp(0.12f, 0.55f, decay) : lerp(0.025f, 0.07f, decay);
    v.ampD = std::exp(-1.0f / (d * sr_));
    static const float f808[6] = {205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.8f};
    static const float f909[6] = {317.0f, 420.0f, 483.0f, 626.0f, 760.0f, 1060.0f};
    const float* f = nine ? f909 : f808;
    for (int i = 0; i < 6; ++i) {
        v.freq[i] = f[i];
        v.phase[i] = 0.1f * static_cast<float>(i);
    }
    v.gain = gain;
    v.lp = 0;
    v.hp = 0;
    v.bp = 0;
}

void AnalogKit::trigClap(Voice& v, float decay, float gain) {
    v.mode = 4;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (lerp(0.08f, 0.28f, decay) * sr_));
    v.clapN = 0;
    v.clapT = 0;
    v.gain = gain;
    v.lp = 0;
    v.hp = 0;
}

void AnalogKit::trigCowbell(Voice& v, float decay, float gain) {
    v.mode = 5;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (lerp(0.2f, 0.7f, decay) * sr_));
    v.freq[0] = 540.0f;
    v.freq[1] = 800.0f;
    v.phase[0] = 0;
    v.phase[1] = 0;
    v.gain = gain;
    v.bp = 0;
    v.lp = 0;
}

void AnalogKit::trigRim(Voice& v, float gain, bool nine) {
    v.mode = 6;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / ((nine ? 0.035f : 0.028f) * sr_));
    v.freq[0] = nine ? 450.0f : 455.0f;
    v.freq[1] = nine ? 1480.0f : 1660.0f;
    v.phase[0] = 0;
    v.phase[1] = 0.3f;
    v.gain = gain;
}

void AnalogKit::trigCym(Voice& v, float decay, float gain, bool ride) {
    v.mode = 3;
    v.metallic = 1;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / ((ride ? lerp(0.4f, 1.2f, decay) : lerp(0.5f, 1.6f, decay)) * sr_));
    static const float f[6] = {317.f, 420.f, 552.f, 703.f, 986.f, 1210.f};
    for (int i = 0; i < 6; ++i) {
        v.freq[i] = f[i] * (ride ? 0.92f : 1.05f);
        v.phase[i] = 0.07f * static_cast<float>(i + 1);
    }
    v.gain = gain * (ride ? 0.7f : 0.85f);
    v.lp = v.hp = v.bp = 0;
    v.tone = ride ? 0.35f : 0.55f;
}

void AnalogKit::trigNoisePerc(Voice& v, float decay, float hp, float gain) {
    v.mode = 7;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (decay * sr_));
    v.tone = hp;
    v.gain = gain;
    v.hp = 0;
    v.lp = 0;
}

void AnalogKit::trigClave(Voice& v, float gain) {
    v.mode = 8;
    v.amp = 1.0f;
    v.ampD = std::exp(-1.0f / (0.04f * sr_));
    v.baseF = 2500.0f;
    v.phase[0] = 0;
    v.gain = gain;
}

void AnalogKit::trigger808(int voice, float velocity) {
    if (voice < 0 || voice >= kVoice808) return;
    Voice& v = voices_[voice];
    const DrumVoiceParams& p = params_.voice[voice];
    const float g = p.level * velocity;
    const float tune = std::pow(2.0f, (p.tune - 0.5f) * 1.4f);
    switch (static_cast<Voice808>(voice)) {
    case Voice808::BD:
        trigSineKick(v, 145.0f * tune, 52.0f * tune, lerp(0.25f, 2.4f, p.decay), 0.55f, g * 1.3f);
        break;
    case Voice808::SD:
        trigSnare(v, 190.0f * tune, p.decay, lerp(0.35f, 0.95f, p.tone), g * 0.9f, false);
        break;
    case Voice808::LT:
        trigTom(v, 90.0f * tune, lerp(0.2f, 1.1f, p.decay), g);
        break;
    case Voice808::MT:
        trigTom(v, 140.0f * tune, lerp(0.18f, 0.9f, p.decay), g);
        break;
    case Voice808::HT:
        trigTom(v, 210.0f * tune, lerp(0.14f, 0.7f, p.decay), g);
        break;
    case Voice808::RS:
        trigRim(v, g * 0.8f, false);
        break;
    case Voice808::CP:
        trigClap(v, p.decay, g * 0.95f);
        break;
    case Voice808::CB:
        trigCowbell(v, p.decay, g * 0.7f);
        break;
    case Voice808::CY:
        trigCym(v, p.decay, g * 0.75f, false);
        break;
    case Voice808::OH:
        trigHat(v, p.decay, g * 0.55f, true, false);
        break;
    case Voice808::CH:
        trigHat(v, p.decay, g * 0.45f, false, false);
        voices_[static_cast<int>(Voice808::OH)].amp = 0.0f;
        break;
    case Voice808::MA:
        trigNoisePerc(v, 0.045f, 0.25f, g * 0.5f);
        break;
    case Voice808::CL:
        trigClave(v, g * 0.65f);
        break;
    }
}

void AnalogKit::trigger909(int voice, float velocity) {
    if (voice < 0 || voice >= kVoice909) return;
    Voice& v = voices_[voice];
    const DrumVoiceParams& p = params_.voice[voice];
    const float g = p.level * velocity;
    const float tune = std::pow(2.0f, (p.tune - 0.5f) * 1.2f);
    switch (static_cast<Voice909>(voice)) {
    case Voice909::BD:
        trigSineKick(v, 210.0f * tune, 48.0f * tune, lerp(0.18f, 1.15f, p.decay), 1.1f, g * 1.35f);
        break;
    case Voice909::SD:
        trigSnare(v, 220.0f * tune, p.decay, lerp(0.45f, 1.0f, p.tone), g, true);
        break;
    case Voice909::LT:
        trigTom(v, 100.0f * tune, lerp(0.16f, 0.85f, p.decay), g);
        break;
    case Voice909::MT:
        trigTom(v, 155.0f * tune, lerp(0.14f, 0.7f, p.decay), g);
        break;
    case Voice909::HT:
        trigTom(v, 230.0f * tune, lerp(0.12f, 0.55f, p.decay), g);
        break;
    case Voice909::RS:
        trigRim(v, g * 0.75f, true);
        break;
    case Voice909::HC:
        trigClap(v, p.decay, g);
        break;
    case Voice909::CH:
        trigHat(v, p.decay, g * 0.4f, false, true);
        voices_[static_cast<int>(Voice909::OH)].amp = 0.0f;
        break;
    case Voice909::OH:
        trigHat(v, p.decay, g * 0.5f, true, true);
        break;
    case Voice909::CR:
        trigCym(v, p.decay, g * 0.8f, false);
        break;
    case Voice909::RD:
        trigCym(v, p.decay, g * 0.55f, true);
        break;
    }
}

float AnalogKit::processVoice(Voice& v) {
    if (v.amp < 1.0e-5f && v.amp2 < 1.0e-5f && v.click < 1.0e-5f) return 0.0f;
    float out = 0.0f;
    switch (v.mode) {
    case 1: {
        v.pitch = 1.0f + (v.pitch - 1.0f) * v.pitchD;
        const float hz = v.baseF * v.pitch;
        v.phase[0] += hz / sr_;
        if (v.phase[0] >= 1.0f) v.phase[0] -= 1.0f;
        out = std::sin(v.phase[0] * kTwoPi) * v.amp;
        out += v.click * whiteNoise() * 0.35f;
        v.amp *= v.ampD;
        v.click *= v.clickD;
        break;
    }
    case 2: {
        v.pitch = 1.0f + (v.pitch - 1.0f) * v.pitchD;
        v.phase[0] += (v.baseF * v.pitch) / sr_;
        v.phase[1] += (v.baseF * 1.54f * v.pitch) / sr_;
        if (v.phase[0] >= 1.0f) v.phase[0] -= 1.0f;
        if (v.phase[1] >= 1.0f) v.phase[1] -= 1.0f;
        const float body = (std::sin(v.phase[0] * kTwoPi) + 0.4f * std::sin(v.phase[1] * kTwoPi)) * v.amp;
        float nz = whiteNoise();
        v.hp += 0.35f * (nz - v.hp);
        const float noise = (nz - v.hp) * v.amp2;
        out = body * 0.55f + noise;
        v.amp *= v.ampD;
        v.amp2 *= v.amp2D;
        break;
    }
    case 3: {
        float mix = 0.0f;
        for (int i = 0; i < 6; ++i) {
            v.phase[i] += v.freq[i] / sr_;
            if (v.phase[i] >= 1.0f) v.phase[i] -= 1.0f;
            mix += v.phase[i] < 0.5f ? 1.0f : -1.0f;
        }
        mix *= (1.0f / 6.0f);
        mix += whiteNoise() * 0.15f;
        const float hpC = v.metallic ? 0.22f : 0.18f;
        v.hp += hpC * (mix - v.hp);
        float hp = mix - v.hp;
        v.bp += 0.08f * (hp - v.bp);
        out = (hp - 0.4f * v.bp) * v.amp;
        v.amp *= v.ampD;
        break;
    }
    case 4: {
        if (v.clapT <= 0 && v.clapN < 4) {
            v.amp = 1.0f - 0.12f * static_cast<float>(v.clapN);
            v.clapN++;
            v.clapT = static_cast<int>(sr_ * (v.clapN == 4 ? 0.0f : 0.012f));
        } else {
            v.clapT--;
        }
        float nz = whiteNoise();
        v.hp += 0.28f * (nz - v.hp);
        out = (nz - v.hp) * v.amp;
        v.amp *= v.ampD;
        break;
    }
    case 5: {
        for (int i = 0; i < 2; ++i) {
            v.phase[i] += v.freq[i] / sr_;
            if (v.phase[i] >= 1.0f) v.phase[i] -= 1.0f;
            out += v.phase[i] < 0.5f ? 0.5f : -0.5f;
        }
        v.bp += 0.12f * (out - v.bp);
        out = (out - 0.5f * v.bp) * v.amp;
        v.amp *= v.ampD;
        break;
    }
    case 6: {
        for (int i = 0; i < 2; ++i) {
            v.phase[i] += v.freq[i] / sr_;
            if (v.phase[i] >= 1.0f) v.phase[i] -= 1.0f;
            out += std::sin(v.phase[i] * kTwoPi) * (i == 0 ? 0.7f : 0.4f);
        }
        out += whiteNoise() * 0.2f;
        out *= v.amp;
        v.amp *= v.ampD;
        break;
    }
    case 7: {
        float nz = whiteNoise();
        v.hp += v.tone * (nz - v.hp);
        out = (nz - v.hp) * v.amp;
        v.amp *= v.ampD;
        break;
    }
    case 8: {
        v.phase[0] += v.baseF / sr_;
        if (v.phase[0] >= 1.0f) v.phase[0] -= 1.0f;
        out = std::sin(v.phase[0] * kTwoPi) * v.amp;
        v.amp *= v.ampD;
        break;
    }
    default:
        break;
    }
    return out * v.gain;
}

float AnalogKit::process() {
    float sum = 0.0f;
    const int n = (kind_ == Kind::Kit808) ? kVoice808 : kVoice909;
    for (int i = 0; i < n; ++i) sum += processVoice(voices_[i]);
    return sum * params_.volume;
}

} // namespace rb
