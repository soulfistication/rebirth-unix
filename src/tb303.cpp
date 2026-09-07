#include "tb303.hpp"

namespace rb {

void TB303::setSampleRate(float sr) {
    sr_ = sr;
    slideCoeff_ = 1.0f - std::exp(-1.0f / (0.055f * sr_));
}

void TB303::setParams(const AcidParams& p) { p_ = p; }

void TB303::noteOn(int midi, bool accent, bool slide) {
    const float semis = (p_.tune - 0.5f) * 24.0f;
    targetFreq_ = midiToHz(static_cast<float>(midi) + semis);
    accenting_ = accent;
    if (slide && gated_) {
        sliding_ = true;
    } else {
        sliding_ = false;
        freq_ = targetFreq_;
        const float decay = lerp(0.18f, 1.85f, p_.decay);
        const float megDecay = accent ? 0.16f : decay;
        const float vegDecay = accent ? decay * 0.85f : decay * 0.7f;
        meg_.trigger(megDecay, sr_, 0.0005f);
        veg_.trigger(vegDecay, sr_, 0.0004f);
        if (accent) acc_.trigger(0.18f, sr_, 0.004f);
    }
    gated_ = true;
}

void TB303::noteOff() {
    gated_ = false;
    sliding_ = false;
}

float TB303::process() {
    if (sliding_) freq_ += (targetFreq_ - freq_) * slideCoeff_;

    const float osc = osc_.process(freq_, sr_, p_.square);
    const float meg = meg_.process();
    const float veg = veg_.process();
    const float acc = acc_.process();

    const float envAmt = p_.envMod * meg;
    const float accAmt = p_.accent * acc * (accenting_ ? 1.0f : 0.0f);
    const float cutNorm = clampf(p_.cutoff * 0.85f + envAmt * 0.9f + accAmt * 0.55f, 0.0f, 1.0f);
    const float cutHz = 30.0f * std::pow(280.0f, cutNorm);
    const float res = clampf(p_.resonance + accAmt * 0.12f, 0.0f, 0.97f);

    const float filt = vcf_.process(osc * 0.9f, cutHz, res, sr_);
    const float vca = veg * (1.0f + accAmt * 0.85f);
    return filt * vca * p_.volume * 1.35f;
}

} // namespace rb
