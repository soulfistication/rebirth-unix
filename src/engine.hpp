#pragma once

#include "drums.hpp"
#include "fx.hpp"
#include "sequencer.hpp"
#include "tb303.hpp"
#include "types.hpp"

#include <atomic>
#include <mutex>

namespace rb {

class Engine {
public:
    void init(int sampleRate);
    void process(float* interleaved, int frames);
    void togglePlay();
    void stop();
    void panic();
    void preview808(int voice);
    void preview909(int voice);

    std::mutex mutex;
    AppParams params;
    Sequencer seq;
    std::atomic<bool> playing{false};
    std::atomic<int> playStep{0};

private:
    void onStep(int step, const AppParams& p);

    int sr_ = 44100;
    TB303 acid_[2];
    AnalogKit kit808_;
    AnalogKit kit909_;
    FXChain fx_;
    float peak_ = 0.0f;
};

} // namespace rb
