#include "fx.hpp"

#include <algorithm>

namespace rb {

void FXChain::setSampleRate(float sr) {
    sr_ = sr;
    delayL_.assign(static_cast<size_t>(sr * 2.5f) + 8, 0.0f);
    delayR_.assign(delayL_.size(), 0.0f);
    delayPos_ = 0;
    pcf_.reset();
}

void FXChain::setTempoDelaySamples(float samplesPerQuarter) {
    const float mul[] = {0.25f, 0.5f, 0.75f, 1.0f, 2.0f};
    const float t = clampf(p_.delayTime, 0.0f, 1.0f);
    const float idx = t * 4.0f;
    const int i = std::min(3, static_cast<int>(idx));
    const float frac = idx - static_cast<float>(i);
    const float beats = lerp(mul[i], mul[i + 1], frac);
    delayLen_ = std::max(2, static_cast<int>(samplesPerQuarter * beats));
    delayLen_ = std::min(delayLen_, static_cast<int>(delayL_.size()) - 2);
}

void FXChain::process(float& l, float& r) {
    float mono = 0.5f * (l + r);

    if (p_.distOn) {
        const float drive = 1.0f + p_.distDrive * 18.0f;
        float d = fastTanh(mono * drive) / fastTanh(drive * 0.7f);
        const float toneC = 0.05f + p_.distTone * 0.45f;
        d = distTone_.low(d, toneC);
        mono = lerp(mono, d, p_.distMix);
    }

    if (p_.pcfOn) {
        const float cutNorm = clampf(lerp(p_.pcfCutoff, pcfStep_, p_.pcfAmount), 0.0f, 1.0f);
        const float hz = 80.0f * std::pow(120.0f, cutNorm);
        mono = pcf_.low(mono, hz, p_.pcfRes, sr_);
    }

    l = r = mono;

    if (p_.delayOn && !delayL_.empty()) {
        const int n = static_cast<int>(delayL_.size());
        int read = delayPos_ - delayLen_;
        while (read < 0) read += n;
        const float dl = delayL_[static_cast<size_t>(read)];
        const float dr = delayR_[static_cast<size_t>(read)];
        const float fb = p_.delayFb * 0.92f;
        delayL_[static_cast<size_t>(delayPos_)] = l + dr * fb;
        delayR_[static_cast<size_t>(delayPos_)] = r + dl * fb * 0.98f;
        delayPos_ = (delayPos_ + 1) % n;
        l += dl * p_.delayMix;
        r += dr * p_.delayMix;
    }

    if (p_.compOn) {
        const float peak = std::max(std::fabs(l), std::fabs(r));
        const float thresh = lerp(0.15f, 0.85f, p_.compThresh);
        const float ratio = lerp(1.5f, 8.0f, p_.compRatio);
        const float atk = 0.01f;
        const float rel = 0.0008f;
        if (peak > env_) env_ += atk * (peak - env_);
        else env_ += rel * (peak - env_);
        float g = 1.0f;
        if (env_ > thresh) {
            const float over = env_ / thresh;
            g = thresh * (1.0f + (over - 1.0f) / ratio) / std::max(env_, 1.0e-6f);
        }
        const float makeup = lerp(1.0f, 3.2f, p_.compGain);
        l *= g * makeup;
        r *= g * makeup;
    }
}

} // namespace rb
