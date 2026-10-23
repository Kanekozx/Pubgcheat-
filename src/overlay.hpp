#pragma once
#include "common.hpp"

class Overlay {
public:
    bool  init();
    void  shutdown();
    void  begin();
    void  end();
    void  box(float x, float y, float w, float h, u32 argb, float thick = 1.5f);
    void  text(float x, float y, const char* s, u32 argb, float scale = 1.0f);
    int   width()  const;
    int   height() const;
};
