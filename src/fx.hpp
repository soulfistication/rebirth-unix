#pragma once

#include "dsp.hpp"
#include "types.hpp"

#include <vector>

namespace rb {

class FXChain {
public:
    void setSampleRate(float sr);
    void setParams(const FXParams& p) { p_ = p; }
    void setPcfStep(float v) { pcfStep_ = v; }
    void setTempoDelaySamples(float samplesPerQuarter);
    void process(float& l, float& r);

private:
    float sr_ = 44100.0f;
    FXParams p_{};
    float pcfStep_ = 0.5f;
    OnePole distTone_;
    Svf pcf_;
    std::vector<float> delayL_, delayR_;
    int delayPos_ = 0;
    int delayLen_ = 1;
    float env_ = 0.0f;
};

} // namespace rb
