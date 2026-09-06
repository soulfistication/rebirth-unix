#pragma once

#include "types.hpp"

#include <cmath>
#include <cstdint>

namespace rb {

inline float whiteNoise() {
    static uint32_t s = 0xC0FFEEU;
    s = s * 1664525u + 1013904223u;
    return static_cast<float>(static_cast<int32_t>(s)) * (1.0f / 2147483648.0f);
}

inline float polyblep(float t, float dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

struct BlepOsc {
    float phase = 0.0f;

    float process(float hz, float sr, bool square) {
        const float dt = clampf(hz / sr, 1.0e-6f, 0.49f);
        float t = phase;
        float saw = 2.0f * t - 1.0f - polyblep(t, dt);
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;
        if (!square) return saw;

        float sq = t < 0.5f ? 1.0f : -1.0f;
        sq += polyblep(t, dt);
        float t2 = t + 0.5f;
        if (t2 >= 1.0f) t2 -= 1.0f;
        sq -= polyblep(t2, dt);
        return sq * 0.85f;
    }
};

struct DecayEnv {
    float value = 0.0f;
    float coeff = 0.0f;
    float attackCoeff = 0.0f;
    bool attacking = false;

    void trigger(float decaySec, float sr, float attackSec = 0.0008f) {
        attacking = true;
        value = 0.0f;
        attackCoeff = 1.0f - std::exp(-1.0f / std::max(1.0f, attackSec * sr));
        coeff = std::exp(-1.0f / std::max(1.0f, decaySec * sr));
    }

    float process() {
        if (attacking) {
            value += (1.0f - value) * attackCoeff;
            if (value > 0.995f) {
                value = 1.0f;
                attacking = false;
            }
        } else {
            value *= coeff;
        }
        return value;
    }
};

struct OnePole {
    float z = 0.0f;

    float low(float in, float coeff) {
        z += coeff * (in - z);
        return z;
    }

    float high(float in, float coeff) { return in - low(in, coeff); }
};

// 4-pole diode-ish ladder, 2x oversampled. Cutoff in Hz, res 0..1.
struct DiodeLadder {
    float z[4]{};
    float delay = 0.0f;

    void reset() {
        z[0] = z[1] = z[2] = z[3] = delay = 0.0f;
    }

    float tick(float in, float g, float k) {
        const float x = fastTanh(in - k * z[3]);
        z[0] = z[0] + g * (x - fastTanh(z[0]));
        z[1] = z[1] + g * (fastTanh(z[0]) - fastTanh(z[1]));
        z[2] = z[2] + g * (fastTanh(z[1]) - fastTanh(z[2]));
        z[3] = z[3] + g * (fastTanh(z[2]) - fastTanh(z[3]));
        return z[3];
    }

    float process(float in, float cutoffHz, float res, float sr) {
        const float fc = clampf(cutoffHz, 20.0f, sr * 0.22f);
        const float g = 1.0f - std::exp(-kTwoPi * fc / (sr * 2.0f));
        const float k = res * res * 9.0f;
        const float a = tick((in + delay) * 0.5f, g, k);
        const float b = tick(in, g, k);
        delay = in;
        return (a + b) * 0.5f;
    }
};

struct Svf {
    float lp = 0.0f, bp = 0.0f;

    void reset() { lp = bp = 0.0f; }

    float low(float in, float cutoffHz, float res, float sr) {
        const float f = 2.0f * std::sin(kPi * clampf(cutoffHz / sr, 0.0001f, 0.25f));
        const float q = 1.0f - clampf(res, 0.0f, 0.95f);
        lp += f * bp;
        const float hp = in - lp - q * bp;
        bp += f * hp;
        return lp;
    }
};

inline float hardSoft(float x) {
    if (x > 1.0f) return 1.0f;
    if (x < -1.0f) return -1.0f;
    return 1.5f * x - 0.5f * x * x * x;
}

} // namespace rb
