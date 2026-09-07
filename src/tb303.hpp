#pragma once

#include "dsp.hpp"
#include "types.hpp"

namespace rb {

class TB303 {
public:
    void setSampleRate(float sr);
    void setParams(const AcidParams& p);
    void noteOn(int midi, bool accent, bool slide);
    void noteOff();
    float process();

private:
    float sr_ = 44100.0f;
    AcidParams p_{};
    BlepOsc osc_;
    DiodeLadder vcf_;
    DecayEnv meg_;
    DecayEnv veg_;
    DecayEnv acc_;
    float freq_ = 110.0f;
    float targetFreq_ = 110.0f;
    float slideCoeff_ = 0.0f;
    bool sliding_ = false;
    bool accenting_ = false;
    bool gated_ = false;
};

} // namespace rb
