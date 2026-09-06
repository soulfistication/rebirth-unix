#pragma once

#include "engine.hpp"

#if defined(__has_include)
#    if __has_include(<SDL.h>)
#        include <SDL.h>
#    else
#        include <SDL2/SDL.h>
#    endif
#else
#    include <SDL.h>
#endif
#include <vector>

namespace rb {

class UI {
public:
    UI(Engine& engine, SDL_Window* window, SDL_Renderer* renderer);
    bool handleEvent(const SDL_Event& e);
    void draw();

private:
    enum Kind {
        KNone,
        KKnob,
        KPlay,
        KStop,
        KPat,
        KWave,
        KAcidStep,
        KDrumPad,
        KDrumStep,
        KMute,
        KFxToggle,
        KPcfStep,
        KHelp,
        KTempo
    };

    struct Hit {
        SDL_Rect r{};
        int kind = KNone;
        int a = 0;
        int b = 0;
        int c = 0;
        float* val = nullptr;
    };

    void color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void fill(int x, int y, int w, int h);
    void box(int x, int y, int w, int h);
    void fillCircle(int cx, int cy, int rad);
    void text(int x, int y, const char* s, int scale = 1);
    void textC(int x, int y, const char* s, int scale = 1);
    void panel(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, const char* title);
    void knob(int cx, int cy, int rad, float* v, const char* label);
    void button(int x, int y, int w, int h, const char* label, bool on, int kind, int a = 0, int b = 0,
                int c = 0);
    void led(int x, int y, int s, bool on, uint8_t r, uint8_t g, uint8_t b);

    const Hit* hitTest(int x, int y) const;
    void onMouseDown(int x, int y, uint8_t button, uint16_t mod);
    void onMouseUp();
    void onMouseMove(int x, int y);
    void onWheel(int x, int y, int dy);
    void onKey(const SDL_KeyboardEvent& k);
    void drawAcid(int idx, int x, int y, const AppParams& snap, bool playing, int step);
    void drawKit(bool is808, int x, int y, int w, const AppParams& snap, bool playing, int step);

    Engine& eng_;
    SDL_Renderer* ren_;
    std::vector<Hit> hits_;
    int dragKind_ = KNone;
    float* dragVal_ = nullptr;
    int dragY_ = 0;
    float dragStart_ = 0;
    int sel808_ = 0;
    int sel909_ = 0;
    bool help_ = false;
};

int runApp();

} // namespace rb
