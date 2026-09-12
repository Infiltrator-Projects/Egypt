    // Player-facing UI artwork is held as graphical surfaces. The runtime
    // selects/blits pixels from these surfaces; it does not construct the HUD
    // or tool ribbon from font cells, rectangles, circles or widget primitives.
    common::IndexedRgbaImage hud_chrome_;
    common::IndexedRgbaImage hud_glyphs_;
    common::IndexedRgbaImage tool_icons_;
    bool hud_graphics_attempted_ = false;
