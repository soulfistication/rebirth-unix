#pragma once

#include "types.hpp"

#include <atomic>
#include <cstdint>

namespace rb {

class Sequencer {
public:
    void setSampleRate(float sr) { sr_ = sr; }
    void reset();
    bool tick(float bpm, float shuffle, int& outStep);

    std::atomic<int> step{0};

private:
    float sr_ = 44100.0f;
    double acc_ = 0.0;
    double wait_ = 1.0;
    int step_ = 0;
    bool started_ = false;
};

void loadDemoPatterns(AppParams& p);

} // namespace rb
