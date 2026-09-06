#include "ui.hpp"

#include "font.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace rb {

namespace {
constexpr int kWinW = 1280;
constexpr int kWinH = 800;
} // namespace

UI::UI(Engine& engine, SDL_Window*, SDL_Renderer* renderer)
    : eng_(engine), ren_(renderer) {}

void UI::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(ren_, r, g, b, a);
}

void UI::fill(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    SDL_RenderFillRect(ren_, &rc);
}

void UI::box(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    SDL_RenderDrawRect(ren_, &rc);
}

void UI::fillCircle(int cx, int cy, int rad) {
    for (int y = -rad; y <= rad; ++y) {
        const int xx = static_cast<int>(std::sqrt(static_cast<float>(rad * rad - y * y)));
        SDL_RenderDrawLine(ren_, cx - xx, cy + y, cx + xx, cy + y);
    }
}

void UI::text(int x, int y, const char* s, int scale) {
    for (int i = 0; s[i]; ++i) {
        const uint8_t* g = glyph(static_cast<unsigned char>(s[i]));
        for (int row = 0; row < 8; ++row) {
            const uint8_t bits = g[row];
            for (int col = 0; col < 8; ++col) {
                if (bits & (1u << col)) {
                    if (scale <= 1) SDL_RenderDrawPoint(ren_, x + col, y + row);
                    else fill(x + col * scale, y + row * scale, scale, scale);
                }
            }
        }
        x += 8 * scale;
    }
}

void UI::textC(int x, int y, const char* s, int scale) {
    const int w = static_cast<int>(std::strlen(s)) * 8 * scale;
    text(x - w / 2, y, s, scale);
}

void UI::panel(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, const char* title) {
    color(r, g, b);
    fill(x, y, w, h);
    color(static_cast<uint8_t>(std::min(255, r + 28)), static_cast<uint8_t>(std::min(255, g + 28)),
          static_cast<uint8_t>(std::min(255, b + 28)));
    box(x, y, w, h);
    color(12, 12, 14);
    fill(x + 1, y + 1, w - 2, 18);
    color(230, 220, 200);
    text(x + 8, y + 5, title, 1);
}

void UI::led(int x, int y, int s, bool on, uint8_t r, uint8_t g, uint8_t b) {
    if (on) color(r, g, b);
    else color(static_cast<uint8_t>(r / 5), static_cast<uint8_t>(g / 5), static_cast<uint8_t>(b / 5));
    fill(x, y, s, s);
}

void UI::knob(int cx, int cy, int rad, float* v, const char* label) {
    color(28, 28, 32);
    fillCircle(cx, cy, rad);
    color(168, 168, 176);
    fillCircle(cx, cy, rad - 1);
    color(36, 36, 40);
    fillCircle(cx, cy, rad - 3);
    const float a = 0.75f * kPi + clampf(*v, 0.0f, 1.0f) * 1.5f * kPi;
    const int x2 = cx + static_cast<int>(std::cos(a) * (rad - 5));
    const int y2 = cy + static_cast<int>(std::sin(a) * (rad - 5));
    color(255, 140, 50);
    SDL_RenderDrawLine(ren_, cx, cy, x2, y2);
    SDL_RenderDrawLine(ren_, cx + 1, cy, x2 + 1, y2);
    color(210, 208, 200);
    textC(cx, cy + rad + 3, label, 1);
    Hit h;
    h.r = {cx - rad - 2, cy - rad - 2, rad * 2 + 4, rad * 2 + 16};
    h.kind = KKnob;
    h.val = v;
    hits_.push_back(h);
}

void UI::button(int x, int y, int w, int h, const char* label, bool on, int kind, int a, int b, int c) {
    if (on) color(210, 90, 40);
    else color(52, 52, 60);
    fill(x, y, w, h);
    color(on ? 255 : 90, on ? 180 : 90, on ? 80 : 100);
    box(x, y, w, h);
    color(on ? 20 : 220, on ? 20 : 220, on ? 16 : 210);
    textC(x + w / 2, y + (h - 8) / 2, label, 1);
    Hit hit;
    hit.r = {x, y, w, h};
    hit.kind = kind;
    hit.a = a;
    hit.b = b;
    hit.c = c;
    hits_.push_back(hit);
}

const UI::Hit* UI::hitTest(int x, int y) const {
    for (int i = static_cast<int>(hits_.size()) - 1; i >= 0; --i) {
        const SDL_Rect& r = hits_[static_cast<size_t>(i)].r;
        if (x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h) return &hits_[static_cast<size_t>(i)];
    }
    return nullptr;
}

void UI::onMouseDown(int x, int y, uint8_t mouseButton, uint16_t mod) {
    const Hit* h = hitTest(x, y);
    if (!h) return;

    const int kind = h->kind;
    const int a = h->a;
    const int b = h->b;
    const int c = h->c;
    float* val = h->val;

    if (kind == KPlay) {
        eng_.togglePlay();
        return;
    }
    if (kind == KStop) {
        eng_.stop();
        eng_.panic();
        return;
    }
    if (kind == KHelp) {
        help_ = !help_;
        return;
    }
    if (kind == KDrumPad) {
        if (a == 0) {
            sel808_ = b;
            eng_.preview808(b);
        } else {
            sel909_ = b;
            eng_.preview909(b);
        }
        return;
    }
    if (kind == KKnob && val) {
        dragKind_ = KKnob;
        dragVal_ = val;
        dragY_ = y;
        dragStart_ = *val;
        return;
    }

    std::lock_guard<std::mutex> lock(eng_.mutex);
    AppParams& p = eng_.params;
    switch (kind) {
    case KPat:
        p.pattern = a;
        break;
    case KWave:
        p.acid[a].square = !p.acid[a].square;
        break;
    case KAcidStep: {
        AcidStep& st = p.bank[p.pattern].acid[a].steps[b];
        if (mouseButton == SDL_BUTTON_RIGHT || (mod & KMOD_ALT)) {
            st.slide = !st.slide;
            if (st.slide) st.gate = true;
        } else if (mod & KMOD_SHIFT) {
            st.accent = !st.accent;
            if (st.accent) st.gate = true;
        } else {
            st.gate = !st.gate;
        }
        break;
    }
    case KDrumStep:
        if (a == 0) p.bank[p.pattern].d808.trig[b][c] = !p.bank[p.pattern].d808.trig[b][c];
        else p.bank[p.pattern].d909.trig[b][c] = !p.bank[p.pattern].d909.trig[b][c];
        break;
    case KMute:
        p.mix.mute[a] = !p.mix.mute[a];
        break;
    case KFxToggle:
        if (a == 0) p.fx.distOn = !p.fx.distOn;
        if (a == 1) p.fx.delayOn = !p.fx.delayOn;
        if (a == 2) p.fx.pcfOn = !p.fx.pcfOn;
        if (a == 3) p.fx.compOn = !p.fx.compOn;
        break;
    case KPcfStep:
        dragKind_ = KPcfStep;
        dragVal_ = &p.bank[p.pattern].pcf[a];
        dragY_ = y;
        dragStart_ = *dragVal_;
        break;
    case KTempo:
        p.bpm = clampf(p.bpm + static_cast<float>(a), 60.0f, 200.0f);
        break;
    default:
        break;
    }
}

void UI::onMouseUp() {
    dragKind_ = KNone;
    dragVal_ = nullptr;
}

void UI::onMouseMove(int x, int y) {
    (void)x;
    if (dragKind_ == KKnob && dragVal_) {
        const float dv = static_cast<float>(dragY_ - y) / 140.0f;
        std::lock_guard<std::mutex> lock(eng_.mutex);
        *dragVal_ = clampf(dragStart_ + dv, 0.0f, 1.0f);
    } else if (dragKind_ == KPcfStep && dragVal_) {
        std::lock_guard<std::mutex> lock(eng_.mutex);
        *dragVal_ = clampf(dragStart_ + static_cast<float>(dragY_ - y) / 80.0f, 0.0f, 1.0f);
    }
}

void UI::onWheel(int x, int y, int dy) {
    const Hit* h = hitTest(x, y);
    if (!h) return;
    std::lock_guard<std::mutex> lock(eng_.mutex);
    if (h->kind == KKnob && h->val) {
        *h->val = clampf(*h->val + static_cast<float>(dy) * 0.03f, 0.0f, 1.0f);
    } else if (h->kind == KAcidStep) {
        AcidStep& st = eng_.params.bank[eng_.params.pattern].acid[h->a].steps[h->b];
        const int n = static_cast<int>(st.note) + dy;
        st.note = static_cast<uint8_t>(clampf(static_cast<float>(n), 24.0f, 60.0f));
        st.gate = true;
    } else if (h->kind == KPcfStep) {
        float& v = eng_.params.bank[eng_.params.pattern].pcf[h->a];
        v = clampf(v + static_cast<float>(dy) * 0.05f, 0.0f, 1.0f);
    } else if (h->kind == KTempo) {
        eng_.params.bpm = clampf(eng_.params.bpm + static_cast<float>(dy), 60.0f, 200.0f);
    }
}

void UI::onKey(const SDL_KeyboardEvent& k) {
    if (k.repeat) return;
    if (k.keysym.sym == SDLK_ESCAPE) {
        if (help_) help_ = false;
        else {
            SDL_Event q{};
            q.type = SDL_QUIT;
            SDL_PushEvent(&q);
        }
        return;
    }
    if (k.keysym.sym == SDLK_SPACE) {
        eng_.togglePlay();
        return;
    }
    if (k.keysym.sym == SDLK_h) {
        help_ = !help_;
        return;
    }
    std::lock_guard<std::mutex> lock(eng_.mutex);
    if (k.keysym.sym >= SDLK_1 && k.keysym.sym <= SDLK_8)
        eng_.params.pattern = static_cast<int>(k.keysym.sym - SDLK_1);
    else if (k.keysym.sym == SDLK_LEFTBRACKET)
        eng_.params.bpm = clampf(eng_.params.bpm - 1.0f, 60.0f, 200.0f);
    else if (k.keysym.sym == SDLK_RIGHTBRACKET)
        eng_.params.bpm = clampf(eng_.params.bpm + 1.0f, 60.0f, 200.0f);
}

bool UI::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_QUIT) return false;
    if (e.type == SDL_MOUSEBUTTONDOWN)
        onMouseDown(e.button.x, e.button.y, e.button.button, SDL_GetModState());
    if (e.type == SDL_MOUSEBUTTONUP) onMouseUp();
    if (e.type == SDL_MOUSEMOTION) onMouseMove(e.motion.x, e.motion.y);
    if (e.type == SDL_MOUSEWHEEL) {
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);
        onWheel(mx, my, e.wheel.y);
    }
    if (e.type == SDL_KEYDOWN) onKey(e.key);
    return true;
}

void UI::drawAcid(int idx, int x, int y, const AppParams& snap, bool playing, int step) {
    char title[16];
    std::snprintf(title, sizeof(title), "303 %c", idx == 0 ? 'A' : 'B');
    panel(x, y, 628, 226, idx == 0 ? 78 : 70, 36, 40, title);

    AcidParams& live = eng_.params.acid[idx];
    button(x + 90, y + 3, 40, 14, live.square ? "SQR" : "SAW", live.square, KWave, idx);
    button(x + 560, y + 3, 50, 14, snap.mix.mute[idx] ? "MUTED" : "ON", snap.mix.mute[idx], KMute, idx);

    knob(x + 46, y + 58, 18, &live.tune, "TUNE");
    knob(x + 112, y + 58, 18, &live.cutoff, "CUTOFF");
    knob(x + 178, y + 58, 18, &live.resonance, "RES");
    knob(x + 244, y + 58, 18, &live.envMod, "ENV MOD");
    knob(x + 310, y + 58, 18, &live.decay, "DECAY");
    knob(x + 376, y + 58, 18, &live.accent, "ACCENT");
    knob(x + 442, y + 58, 18, &live.volume, "VOL");
    knob(x + 520, y + 58, 18, &eng_.params.mix.acid[idx], "MIX");

    color(200, 190, 180);
    text(x + 12, y + 96, "click gate   shift accent   alt/right slide   wheel pitch", 1);

    for (int s = 0; s < 16; ++s) {
        const AcidStep& st = snap.bank[snap.pattern].acid[idx].steps[s];
        const int sx = x + 10 + s * 38;
        const int sy = y + 114;
        const bool here = playing && step == s;
        if (st.gate) color(180, 70, 40);
        else color(40, 34, 36);
        fill(sx, sy, 34, 46);
        if (here) color(255, 220, 80);
        else color(90, 80, 78);
        box(sx, sy, 34, 46);
        led(sx + 4, sy + 4, 6, st.accent, 255, 60, 40);
        led(sx + 24, sy + 4, 6, st.slide, 255, 200, 60);
        char nm[8];
        if (st.gate) noteNameIndex(st.note, nm, 8);
        else {
            nm[0] = '-';
            nm[1] = 0;
        }
        color(240, 230, 210);
        textC(sx + 17, sy + 20, nm, 1);
        Hit hit;
        hit.r = {sx, sy, 34, 46};
        hit.kind = KAcidStep;
        hit.a = idx;
        hit.b = s;
        hits_.push_back(hit);
        color(120, 110, 108);
        char nbuf[4];
        std::snprintf(nbuf, sizeof(nbuf), "%d", s + 1);
        textC(sx + 17, sy + 50, nbuf, 1);
    }
}

void UI::drawKit(bool is808, int x, int y, int w, const AppParams& snap, bool playing, int step) {
    const int n = is808 ? kVoice808 : kVoice909;
    panel(x, y, w, 168, is808 ? 36 : 34, is808 ? 40 : 58, is808 ? 72 : 42, is808 ? "808" : "909");
    const int muteI = is808 ? 2 : 3;
    button(x + w - 62, y + 3, 50, 14, snap.mix.mute[muteI] ? "MUTED" : "ON", snap.mix.mute[muteI], KMute,
           muteI);

    const int sel = is808 ? sel808_ : sel909_;
    DrumVoiceParams& vp = is808 ? eng_.params.kit808.voice[sel] : eng_.params.kit909.voice[sel];
    knob(x + w - 210, y + 48, 16, &vp.level, "LEVEL");
    knob(x + w - 150, y + 48, 16, &vp.tune, "TUNE");
    knob(x + w - 90, y + 48, 16, &vp.decay, "DECAY");
    knob(x + w - 30, y + 48, 16, is808 ? &eng_.params.mix.d808 : &eng_.params.mix.d909, "MIX");

    for (int v = 0; v < n; ++v) {
        const int px = x + 8 + v * 46;
        button(px, y + 24, 42, 18, is808 ? name808(v) : name909(v), v == sel, KDrumPad, is808 ? 0 : 1, v);
    }

    color(150, 160, 170);
    text(x + 8, y + 76, "Pads audition + select. Steps edit the selected voice.", 1);

    for (int s = 0; s < 16; ++s) {
        const bool trig = is808 ? snap.bank[snap.pattern].d808.trig[sel][s]
                                : snap.bank[snap.pattern].d909.trig[sel][s];
        const int sx = x + 8 + s * 36;
        const int sy = y + 96;
        const bool here = playing && step == s;
        if (trig) color(is808 ? 80 : 70, is808 ? 140 : 170, is808 ? 200 : 90);
        else color(30, 32, 38);
        fill(sx, sy, 32, 40);
        if (here) color(255, 220, 70);
        else color(70, 74, 80);
        box(sx, sy, 32, 40);
        char nbuf[4];
        std::snprintf(nbuf, sizeof(nbuf), "%d", s + 1);
        color(220, 220, 210);
        textC(sx + 16, sy + 16, nbuf, 1);
        Hit hit;
        hit.r = {sx, sy, 32, 40};
        hit.kind = KDrumStep;
        hit.a = is808 ? 0 : 1;
        hit.b = sel;
        hit.c = s;
        hits_.push_back(hit);
    }
}

void UI::draw() {
    hits_.clear();
    AppParams snap;
    const bool playing = eng_.playing.load();
    const int step = eng_.playStep.load();
    {
        std::lock_guard<std::mutex> lock(eng_.mutex);
        snap = eng_.params;
    }

    color(16, 16, 20);
    SDL_RenderClear(ren_);

    panel(6, 6, kWinW - 12, 40, 40, 40, 46, "ACID RACK   2x303 + 808 + 909");
    button(220, 12, 56, 24, "PLAY", playing, KPlay);
    button(282, 12, 56, 24, "STOP", false, KStop);

    char bpmStr[16];
    std::snprintf(bpmStr, sizeof(bpmStr), "%d", static_cast<int>(snap.bpm + 0.5f));
    color(230, 220, 200);
    text(352, 18, "TEMPO", 1);
    button(404, 12, 22, 24, "-", false, KTempo, -1);
    color(255, 200, 80);
    text(432, 18, bpmStr, 1);
    button(470, 12, 22, 24, "+", false, KTempo, 1);

    color(180, 180, 170);
    text(510, 18, "PATTERN", 1);
    for (int i = 0; i < 8; ++i) {
        char lab[2] = {static_cast<char>('1' + i), 0};
        button(578 + i * 28, 14, 24, 22, lab, snap.pattern == i, KPat, i);
    }

    color(180, 180, 170);
    text(820, 10, "SHUFFLE", 1);
    knob(900, 24, 11, &eng_.params.shuffle, "");
    knob(980, 24, 11, &eng_.params.mix.master, "MAST");
    button(1040, 14, 48, 22, "HELP", help_, KHelp);

    drawAcid(0, 6, 52, snap, playing, step);
    drawAcid(1, 646, 52, snap, playing, step);
    drawKit(true, 6, 286, 836, snap, playing, step);
    drawKit(false, 6, 462, 836, snap, playing, step);

    panel(850, 286, 424, 344, 58, 52, 32, "FX");
    FXParams& fx = eng_.params.fx;
    button(860, 310, 50, 16, fx.distOn ? "ON" : "OFF", fx.distOn, KFxToggle, 0);
    color(230, 220, 190);
    text(918, 312, "DISTORTION", 1);
    knob(890, 360, 16, &fx.distDrive, "DRIVE");
    knob(950, 360, 16, &fx.distTone, "TONE");
    knob(1010, 360, 16, &fx.distMix, "MIX");

    button(1080, 310, 50, 16, fx.delayOn ? "ON" : "OFF", fx.delayOn, KFxToggle, 1);
    color(230, 220, 190);
    text(1138, 312, "DELAY", 1);
    knob(1110, 360, 16, &fx.delayTime, "TIME");
    knob(1170, 360, 16, &fx.delayFb, "FDBK");
    knob(1230, 360, 16, &fx.delayMix, "MIX");

    button(860, 410, 50, 16, fx.pcfOn ? "ON" : "OFF", fx.pcfOn, KFxToggle, 2);
    color(230, 220, 190);
    text(918, 412, "PCF  pattern filter", 1);
    knob(890, 458, 16, &fx.pcfCutoff, "CUTOFF");
    knob(950, 458, 16, &fx.pcfRes, "RES");
    knob(1010, 458, 16, &fx.pcfAmount, "AMT");

    for (int s = 0; s < 16; ++s) {
        const int sx = 860 + s * 25;
        const int sy = 500;
        const float v = snap.bank[snap.pattern].pcf[s];
        color(30, 28, 24);
        fill(sx, sy, 22, 52);
        color(220, 160, 50);
        const int hh = 4 + static_cast<int>(v * 44);
        fill(sx + 1, sy + 52 - hh, 20, hh);
        if (playing && step == s) {
            color(255, 230, 80);
            box(sx, sy, 22, 52);
        }
        Hit hit;
        hit.r = {sx, sy, 22, 52};
        hit.kind = KPcfStep;
        hit.a = s;
        hits_.push_back(hit);
    }

    button(1080, 410, 50, 16, fx.compOn ? "ON" : "OFF", fx.compOn, KFxToggle, 3);
    color(230, 220, 190);
    text(1138, 412, "COMP", 1);
    knob(1110, 458, 16, &fx.compThresh, "THRES");
    knob(1170, 458, 16, &fx.compRatio, "RATIO");
    knob(1230, 458, 16, &fx.compGain, "GAIN");

    panel(850, 638, 424, 156, 40, 40, 46, "MIX / NOTES");
    color(210, 205, 195);
    text(860, 664, "Space play/stop   1-8 pattern   [ ] tempo", 1);
    text(860, 680, "303: click gate, Shift accent, Alt slide", 1);
    text(860, 696, "Wheel on a 303 step changes pitch.", 1);
    text(860, 712, "Delay TIME is tempo-synced (16th-1/2).", 1);
    text(860, 728, "Analog models, no drum samples.", 1);
    text(860, 748, "Homage to ReBirth RB-338 — unofficial.", 1);

    panel(6, 638, 836, 156, 40, 40, 46, "TRANSPORT HINTS");
    color(200, 198, 190);
    text(16, 664, "Pattern 1: acid loop (303 A lead, 303 B sub, 808 + 909).", 1);
    text(16, 680, "Pattern 2: harder 909 groove.  Pattern 3: sliding 303 line.", 1);
    text(16, 696, "Turn RES + ENV MOD up on 303 A, drop CUTOFF, hit PLAY.", 1);
    text(16, 712, "Enable PCF and drag the 16 bars for a stepped filter sweep.", 1);
    char stt[80];
    std::snprintf(stt, sizeof(stt), "Playhead %d/16    selected 808=%s  909=%s", step + 1,
                  name808(sel808_), name909(sel909_));
    color(255, 180, 70);
    text(16, 740, stt, 1);

    if (help_) {
        SDL_SetRenderDrawBlendMode(ren_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren_, 0, 0, 0, 210);
        fill(180, 80, 920, 620);
        SDL_SetRenderDrawBlendMode(ren_, SDL_BLENDMODE_NONE);
        color(240, 230, 210);
        text(200, 100, "ACID RACK  --  HELP", 2);
        text(200, 140, "UNIX C++ groovebox inspired by Propellerhead ReBirth:", 1);
        text(200, 160, "  two TB-303 style bass synths (saw/square, ladder, accent/slide)", 1);
        text(200, 176, "  analog-modelled TR-808 and TR-909 kits", 1);
        text(200, 192, "  16-step sequencer, 8 patterns, shuffle", 1);
        text(200, 208, "  distortion, tempo-sync delay, PCF, compressor", 1);
        text(200, 236, "KEYS", 2);
        text(200, 266, "  Space           Play / stop", 1);
        text(200, 282, "  1 .. 8          Select pattern", 1);
        text(200, 298, "  [  ]            Tempo down / up", 1);
        text(200, 314, "  H               Toggle this help", 1);
        text(200, 330, "  Esc             Close help / quit", 1);
        text(200, 362, "MOUSE", 2);
        text(200, 392, "  Drag knobs vertically. Wheel for fine adjust.", 1);
        text(200, 408, "  303 steps: click gate, Shift accent, Alt or right-click slide.", 1);
        text(200, 424, "  Drum pads audition and select. Then click the 16 steps.", 1);
        text(200, 456, "Esc or H to close.", 1);
    }

    SDL_RenderPresent(ren_);
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    auto* engine = static_cast<Engine*>(userdata);
    const int frames = len / static_cast<int>(sizeof(float) * 2);
    engine->process(reinterpret_cast<float*>(stream), frames);
}

int runApp() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Acid Rack — 303 / 808 / 909", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWinW, kWinH,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Engine engine;
    engine.init(44100);

    SDL_AudioSpec want{};
    SDL_AudioSpec have{};
    want.freq = 44100;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 256;
    want.callback = audioCallback;
    want.userdata = &engine;

    const SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (dev == 0) {
        std::fprintf(stderr, "SDL_OpenAudioDevice: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (have.freq != 44100 || have.format != AUDIO_F32SYS || have.channels != 2) {
        std::fprintf(stderr, "audio format mismatch (%d Hz, fmt=%d, ch=%d)\n", have.freq, have.format,
                     have.channels);
    }

    UI ui(engine, window, renderer);
    SDL_PauseAudioDevice(dev, 0);

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (!ui.handleEvent(e)) running = false;
        }
        ui.draw();
    }

    SDL_PauseAudioDevice(dev, 1);
    SDL_CloseAudioDevice(dev);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

} // namespace rb
