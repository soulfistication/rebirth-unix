#include "engine.hpp"

#include <algorithm>
#include <cmath>

namespace rb {

void Engine::init(int sampleRate) {
    sr_ = sampleRate;
    for (auto& a : acid_) a.setSampleRate(static_cast<float>(sr_));
    kit808_.setSampleRate(static_cast<float>(sr_));
    kit909_.setSampleRate(static_cast<float>(sr_));
    kit808_.setKind(AnalogKit::Kind::Kit808);
    kit909_.setKind(AnalogKit::Kind::Kit909);
    fx_.setSampleRate(static_cast<float>(sr_));
    seq.setSampleRate(static_cast<float>(sr_));
    loadDemoPatterns(params);
}

void Engine::togglePlay() {
    bool next = !playing.load();
    if (next) {
        seq.reset();
        playing.store(true);
    } else {
        playing.store(false);
        panic();
    }
}

void Engine::stop() {
    playing.store(false);
    seq.reset();
    playStep.store(0);
}

void Engine::panic() {
    for (auto& a : acid_) a.noteOff();
}

void Engine::preview808(int voice) { kit808_.trigger808(voice); }
void Engine::preview909(int voice) { kit909_.trigger909(voice); }

void Engine::onStep(int step, const AppParams& p) {
    const Pattern& pat = p.bank[p.pattern];
    for (int i = 0; i < 2; ++i) {
        const int prev = (step + kSteps - 1) % kSteps;
        const AcidStep& st = pat.acid[i].steps[step];
        const AcidStep& pv = pat.acid[i].steps[prev];
        if (!st.gate) {
            acid_[i].noteOff();
        } else {
            const bool slideIn = pv.gate && pv.slide;
            acid_[i].noteOn(st.note, st.accent, slideIn);
        }
    }
    for (int v = 0; v < kVoice808; ++v) {
        if (pat.d808.trig[v][step]) kit808_.trigger808(v);
    }
    for (int v = 0; v < kVoice909; ++v) {
        if (pat.d909.trig[v][step]) kit909_.trigger909(v);
    }
    fx_.setPcfStep(pat.pcf[step]);
}

void Engine::process(float* interleaved, int frames) {
    AppParams local;
    {
        std::lock_guard<std::mutex> lock(mutex);
        local = params;
    }

    acid_[0].setParams(local.acid[0]);
    acid_[1].setParams(local.acid[1]);
    kit808_.setParams(local.kit808);
    kit909_.setParams(local.kit909);
    fx_.setParams(local.fx);
    const float spq = static_cast<float>(sr_) * 60.0f / std::max(40.0f, local.bpm);
    fx_.setTempoDelaySamples(spq);

    const bool run = playing.load();
    for (int n = 0; n < frames; ++n) {
        if (run) {
            int st = 0;
            if (seq.tick(local.bpm, local.shuffle, st)) {
                onStep(st, local);
                playStep.store(st);
            }
        }

        float a0 = acid_[0].process();
        float a1 = acid_[1].process();
        float d8 = kit808_.process();
        float d9 = kit909_.process();

        if (local.mix.mute[0]) a0 = 0;
        if (local.mix.mute[1]) a1 = 0;
        if (local.mix.mute[2]) d8 = 0;
        if (local.mix.mute[3]) d9 = 0;

        float l = a0 * local.mix.acid[0] + a1 * local.mix.acid[1] +
                  d8 * local.mix.d808 + d9 * local.mix.d909;
        float r = l;
        fx_.process(l, r);
        l *= local.mix.master;
        r *= local.mix.master;

        const float peak = std::max(std::fabs(l), std::fabs(r));
        peak_ = std::max(peak_ * 0.9995f, peak);
        float lim = 1.0f;
        if (peak_ > 0.95f) lim = 0.95f / peak_;
        l = fastTanh(l * lim);
        r = fastTanh(r * lim);

        interleaved[n * 2] = l;
        interleaved[n * 2 + 1] = r;
    }
}

} // namespace rb
