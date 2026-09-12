    static constexpr const char* kHudGlyphCharacters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 :-/,.?+";

    void ensure_hud_graphics() {
        if (hud_graphics_attempted_) return;
        hud_graphics_attempted_ = true;
        hud_chrome_ = common::load_e8pa("build/assets/hud_chrome.e8p");
        hud_glyphs_ = common::load_e8pa("build/assets/hud_glyphs.e8p");
        if (!hud_chrome_.valid() || !hud_glyphs_.valid()) {
            std::cerr << "Egypt: graphical HUD assets failed to load\n";
        }
    }

    void blit_hud_region(const common::IndexedRgbaImage& image,
                         int sx, int sy, int sw, int sh, int dx, int dy) {
        if (!image.valid() || sw <= 0 || sh <= 0) return;
        for (int y = 0; y < sh; ++y) {
            const int src_y = sy + y;
            if (src_y < 0 || src_y >= image.height) continue;
            for (int x = 0; x < sw; ++x) {
                const int src_x = sx + x;
                if (src_x < 0 || src_x >= image.width) continue;
                const std::uint8_t palette_index =
                    image.indices[static_cast<std::size_t>(src_y * image.width + src_x)];
                const std::uint8_t alpha = image.alpha[palette_index];
                if (alpha == 0) continue;
                const Color color = image.palette[palette_index];
                if (alpha == 255) fb_.pixel(dx + x, dy + y, color);
                else fb_.blend_rect({dx + x, dy + y, 1, 1}, color, alpha);
            }
        }
    }

    int hud_glyph_index(char raw) const {
        char ch = raw;
        if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
        const std::string glyphs(kHudGlyphCharacters);
        const auto pos = glyphs.find(ch);
        const auto fallback = glyphs.find('?');
        return static_cast<int>(pos == std::string::npos ? fallback : pos);
    }

    int hud_glyph_run_width(const std::string& value, bool large) const {
        const int cell_w = large ? 18 : 10;
        return static_cast<int>(value.size()) * cell_w;
    }

    void blit_hud_glyph_run(int x, int y, const std::string& value, bool large) {
        if (!hud_glyphs_.valid()) return;
        const int cell_w = large ? 18 : 10;
        const int cell_h = large ? 26 : 15;
        const int source_y = large ? 0 : 26;

        int cursor = x;
        for (char ch : value) {
            const int index = hud_glyph_index(ch);
            blit_hud_region(hud_glyphs_, index * cell_w, source_y,
                            cell_w, cell_h, cursor, y);
            cursor += cell_w;
        }
    }

    void blit_hud_glyph_run_centered(Rect area, const std::string& value, bool large) {
        const int cell_h = large ? 26 : 15;
        const int width = hud_glyph_run_width(value, large);
        blit_hud_glyph_run(area.x + (area.w - width) / 2,
                           area.y + (area.h - cell_h) / 2,
                           value, large);
    }

    void draw_top_hud() {
        ensure_hud_graphics();
        if (!hud_chrome_.valid() || !hud_glyphs_.valid()) return;

        // This is now a graphical screen, not a collection of runtime widgets.
        // The authored 64x72 HUD surface is tiled across the top of the frame.
        for (int x = 0; x < fb_.width(); x += 64) {
            blit_hud_region(hud_chrome_, 0, 0, 64, 72, x, 0);
        }

        // Whole pre-painted control clusters are selected by state. The pause,
        // play and speed symbols are pixels in the asset, not circles/triangles
        // assembled by the game renderer.
        int control_state = 0;
        if (!paused_) {
            if (simulation_speed_ == 1) control_state = 1;
            else if (simulation_speed_ == 2) control_state = 2;
            else control_state = 3;
        }
        blit_hud_region(hud_chrome_, 64 + control_state * 180, 0, 180, 52, 0, 0);
        blit_hud_glyph_run(145, 16, "X" + std::to_string(simulation_speed_), false);

        // The central cartouche, wings and ornament are one graphical surface.
        constexpr int date_w = 320;
        constexpr int date_h = 44;
        const int date_x = fb_.width() / 2 - date_w / 2;
        blit_hud_region(hud_chrome_, 0, 72, date_w, date_h, date_x, 2);

        const std::string date =
            std::string(world_.month_name()) + " " + std::to_string(world_.year_bc()) + " BC";
        blit_hud_glyph_run_centered({date_x + 48, 7, date_w - 96, 34}, date, true);

        // Values remain strings as game data, but visually they are only runs
        // of pre-rasterized glyph graphics copied from the artwork atlas.
        const std::string stats =
            "POP " + std::to_string(world_.population()) +
            "   IMM " + std::to_string(world_.immigrants_in_transit()) +
            "   EMP " + std::to_string(world_.employed_population()) +
            "   FOOD " + std::to_string(world_.total_food()) +
            "   POT " + std::to_string(world_.total_pottery()) +
            "   TREASURY " + std::to_string(world_.treasury());
        blit_hud_glyph_run_centered({190, 49, std::max(0, fb_.width() - 420), 15}, stats, false);

        // Normal and hover menu plaques are separate painted frames in the
        // chrome atlas. The renderer simply chooses which picture to show.
        const Rect menu = main_menu_rect();
        const int menu_source_y = menu.contains(mx_, my_) ? 116 : 72;
        blit_hud_region(hud_chrome_, 800, menu_source_y, 200, 44,
                        menu.x - 5, menu.y - 4);
        blit_hud_glyph_run_centered(menu, "MAIN MENU", false);

        if (flat_mode_) {
            blit_hud_glyph_run(fb_.width() - 320, 53, "FLAT VIEW", false);
        }
    }
