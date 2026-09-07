#include "sequencer.hpp"

#include <algorithm>
#include <cmath>

namespace rb {

void Sequencer::reset() {
    acc_ = 0.0;
    wait_ = 1.0;
    step_ = 0;
    started_ = false;
    step.store(0);
}

bool Sequencer::tick(float bpm, float shuffle, int& outStep) {
    const double sp16 = static_cast<double>(sr_) * 60.0 / std::max(40.0f, bpm) / 4.0;
    if (!started_) {
        started_ = true;
        wait_ = 0.0;
        acc_ = 0.0;
        step_ = 0;
        outStep = 0;
        step.store(0);
        return true;
    }
    acc_ += 1.0;
    if (acc_ < wait_) return false;

    step_ = (step_ + 1) & 15;
    const double swing = static_cast<double>(clampf(shuffle, 0.0f, 0.7f)) * sp16 * 0.5;
    if (step_ & 1) wait_ = sp16 + swing;
    else wait_ = sp16 - swing;
    acc_ = 0.0;
    outStep = step_;
    step.store(step_);
    return true;
}

static AcidStep ns(int note, bool acc = false, bool slide = false) {
    AcidStep s;
    s.gate = true;
    s.note = static_cast<uint8_t>(note);
    s.accent = acc;
    s.slide = slide;
    return s;
}

void loadDemoPatterns(AppParams& p) {
    for (int b = 0; b < kPatterns; ++b) {
        for (int i = 0; i < kSteps; ++i) p.bank[b].pcf[i] = 0.35f + 0.04f * static_cast<float>(i);
    }

    auto& a0 = p.bank[0].acid[0].steps;
    a0[0] = ns(36, true);
    a0[2] = ns(36);
    a0[4] = ns(39);
    a0[5] = ns(39, false, true);
    a0[6] = ns(43, true);
    a0[8] = ns(48);
    a0[10] = ns(46, false, true);
    a0[11] = ns(43);
    a0[12] = ns(39, true);
    a0[14] = ns(36);
    a0[15] = ns(31, false, true);

    auto& b0 = p.bank[0].acid[1].steps;
    b0[0] = ns(24);
    b0[3] = ns(24);
    b0[6] = ns(27, true);
    b0[8] = ns(31, false, true);
    b0[10] = ns(24);
    b0[12] = ns(27);
    b0[14] = ns(31, true);

    auto& e0 = p.bank[0].d808;
    for (int i = 0; i < 16; ++i) {
        if ((i % 2) == 0) e0.trig[static_cast<int>(Voice808::CH)][i] = true;
        if (i == 0 || i == 3 || i == 8 || i == 10) e0.trig[static_cast<int>(Voice808::BD)][i] = true;
        if (i == 4 || i == 12) e0.trig[static_cast<int>(Voice808::CP)][i] = true;
        if (i == 14) e0.trig[static_cast<int>(Voice808::OH)][i] = true;
        if (i == 6 || i == 14) e0.trig[static_cast<int>(Voice808::MA)][i] = true;
    }

    auto& n0 = p.bank[0].d909;
    n0.trig[static_cast<int>(Voice909::BD)][0] = true;
    n0.trig[static_cast<int>(Voice909::BD)][9] = true;
    n0.trig[static_cast<int>(Voice909::SD)][4] = true;
    n0.trig[static_cast<int>(Voice909::SD)][12] = true;
    for (int i = 0; i < 16; ++i) n0.trig[static_cast<int>(Voice909::CH)][i] = true;
    n0.trig[static_cast<int>(Voice909::OH)][6] = true;
    n0.trig[static_cast<int>(Voice909::OH)][14] = true;
    n0.trig[static_cast<int>(Voice909::HT)][11] = true;

    auto& a1 = p.bank[1].acid[0].steps;
    a1[0] = ns(38, true);
    a1[1] = ns(38, false, true);
    a1[2] = ns(41);
    a1[4] = ns(38);
    a1[7] = ns(45, true, true);
    a1[8] = ns(50);
    a1[11] = ns(41);
    a1[12] = ns(38, true);
    a1[13] = ns(36, false, true);
    a1[15] = ns(33);

    auto& b1 = p.bank[1].acid[1].steps;
    for (int i = 0; i < 16; i += 4) b1[i] = ns(26, i == 0);

    auto& e1 = p.bank[1].d808;
    e1.trig[static_cast<int>(Voice808::BD)][0] = true;
    e1.trig[static_cast<int>(Voice808::BD)][6] = true;
    e1.trig[static_cast<int>(Voice808::BD)][8] = true;
    e1.trig[static_cast<int>(Voice808::SD)][4] = true;
    e1.trig[static_cast<int>(Voice808::SD)][12] = true;
    for (int i = 0; i < 16; ++i) e1.trig[static_cast<int>(Voice808::CH)][i] = true;
    e1.trig[static_cast<int>(Voice808::CB)][3] = true;
    e1.trig[static_cast<int>(Voice808::CB)][11] = true;

    auto& n1 = p.bank[1].d909;
    for (int i : {0, 7, 10}) n1.trig[static_cast<int>(Voice909::BD)][i] = true;
    n1.trig[static_cast<int>(Voice909::SD)][4] = true;
    n1.trig[static_cast<int>(Voice909::SD)][12] = true;
    n1.trig[static_cast<int>(Voice909::CH)][2] = true;
    n1.trig[static_cast<int>(Voice909::CH)][6] = true;
    n1.trig[static_cast<int>(Voice909::CH)][10] = true;
    n1.trig[static_cast<int>(Voice909::CH)][14] = true;
    n1.trig[static_cast<int>(Voice909::CR)][0] = true;

    auto& a2 = p.bank[2].acid[0].steps;
    a2[0] = ns(41, true, true);
    a2[1] = ns(48);
    a2[2] = ns(41);
    a2[4] = ns(36, true);
    a2[6] = ns(39, false, true);
    a2[7] = ns(43);
    a2[8] = ns(48, true);
    a2[10] = ns(43);
    a2[12] = ns(39);
    a2[14] = ns(36, true);

    p.acid[0].cutoff = 0.26f;
    p.acid[0].resonance = 0.78f;
    p.acid[0].envMod = 0.68f;
    p.acid[0].decay = 0.32f;
    p.acid[1].square = true;
    p.acid[1].cutoff = 0.22f;
    p.acid[1].resonance = 0.6f;
    p.acid[1].volume = 0.55f;
    p.acid[1].envMod = 0.4f;

    p.kit808.voice[static_cast<int>(Voice808::BD)].decay = 0.62f;
    p.kit808.voice[static_cast<int>(Voice808::BD)].level = 0.95f;
    p.kit808.voice[static_cast<int>(Voice808::CH)].level = 0.42f;
    p.kit808.voice[static_cast<int>(Voice808::OH)].level = 0.5f;
    p.kit808.voice[static_cast<int>(Voice808::CP)].level = 0.7f;
    p.kit909.voice[static_cast<int>(Voice909::BD)].decay = 0.45f;
    p.kit909.voice[static_cast<int>(Voice909::CH)].level = 0.38f;
    p.kit909.voice[static_cast<int>(Voice909::SD)].level = 0.72f;
    p.kit909.volume = 0.7f;

    for (int i = 0; i < kSteps; ++i) {
        p.bank[0].pcf[i] = 0.25f + 0.5f * (0.5f + 0.5f * std::sin(i * 0.7f));
    }
}

} // namespace rb
