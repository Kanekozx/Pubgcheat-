#include "overlay.hpp"
static bool g_on = false;
bool Overlay::init()     { g_on = true; return true; }
void Overlay::shutdown() { g_on = false; }
void Overlay::begin()    {}
void Overlay::end()      {}
void Overlay::box(float, float, float, float, u32, float) {}
void Overlay::text(float, float, const char*, u32, float) {}
int Overlay::width()  const { return 1080; }
int Overlay::height() const { return 2400; }
