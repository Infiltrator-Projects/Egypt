    // Player-facing HUD artwork is held as graphical surfaces. The runtime
    // selects/blits pixels from these surfaces; it does not construct the HUD
    // from font cells, rectangles, circles or other widget primitives.
    common::IndexedRgbaImage hud_chrome_;
    common::IndexedRgbaImage hud_glyphs_;
    bool hud_graphics_attempted_ = false;
