#include "overlay.hpp"

static bool  g_on = false;
static int   g_w = 1080, g_h = 2400; // Default phone resolution

bool Overlay::init()     { g_on = true; LOG("overlay: null backend initialized"); return true; }
void Overlay::shutdown() { g_on = false; }
void Overlay::begin()    {}
void Overlay::end()      {}
void Overlay::box(float x, float y, float w, float h, u32, float) {
    if (g_on) LOG("box %.1f %.1f %.1f %.1f", x, y, w, h);
}
void Overlay::text(float x, float y, const char* s, u32, float) {
    if (g_on) LOG("text %.1f %.1f '%s'", x, y, s);
}
int Overlay::width()  const { return g_w; }
int Overlay::height() const { return g_h; }