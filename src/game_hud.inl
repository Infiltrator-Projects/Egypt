    static constexpr const char* kHudGlyphCharacters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 :-/,.?+";

    void ensure_hud_graphics() {
        if (hud_graphics_attempted_) return;
        hud_graphics_attempted_ = true;
        hud_chrome_ = common::load_e8pa(common::runtime_asset_path("hud_chrome.e8p"));
        hud_glyphs_ = common::load_e8pa(common::runtime_asset_path("hud_glyphs.e8p"));
        tool_icons_ = common::load_e8pa(common::runtime_asset_path("tool_icons.e8p"));
        if (!hud_chrome_.valid() || !hud_glyphs_.valid() || !tool_icons_.valid()) {
            std::cerr << "Egypt: authored graphical UI surfaces failed to load\n";
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

    int hud_glyph_advance(char ch, bool large, int* left_out = nullptr, int* width_out = nullptr) const {
        const int cell_w = large ? 20 : 12;
        const int cell_h = large ? 30 : 18;
        const int source_y = large ? 0 : 30;
        const int space = large ? 9 : 6;
        const int spacing = large ? 2 : 1;
        if (ch == ' ') {
            if (left_out) *left_out = 0;
            if (width_out) *width_out = 0;
            return space;
        }

        const int index = hud_glyph_index(ch);
        int left = cell_w;
        int right = -1;
        for (int y = 0; y < cell_h; ++y) {
            for (int x = 0; x < cell_w; ++x) {
                const int src_x = index * cell_w + x;
                if (src_x < 0 || src_x >= hud_glyphs_.width || source_y + y >= hud_glyphs_.height) continue;
                const auto palette_index = hud_glyphs_.indices[
                    static_cast<std::size_t>((source_y + y) * hud_glyphs_.width + src_x)];
                if (hud_glyphs_.alpha[palette_index] == 0) continue;
                left = std::min(left, x);
                right = std::max(right, x);
            }
        }
        if (right < left) {
            left = 0;
            right = std::max(0, cell_w / 2 - 1);
        }
        const int width = right - left + 1;
        if (left_out) *left_out = left;
        if (width_out) *width_out = width;
        return width + spacing;
    }

    int hud_glyph_run_width(const std::string& value, bool large) const {
        if (!hud_glyphs_.valid() || value.empty()) return 0;
        int width = 0;
        for (char ch : value) width += hud_glyph_advance(ch, large);
        return std::max(0, width - (large ? 2 : 1));
    }

    void blit_hud_glyph_run(int x, int y, const std::string& value, bool large) {
        if (!hud_glyphs_.valid()) return;
        const int cell_w = large ? 20 : 12;
        const int cell_h = large ? 30 : 18;
        const int source_y = large ? 0 : 30;
        int cursor = x;
        for (char ch : value) {
            int left = 0;
            int width = 0;
            const int advance = hud_glyph_advance(ch, large, &left, &width);
            if (ch != ' ' && width > 0) {
                const int index = hud_glyph_index(ch);
                blit_hud_region(hud_glyphs_, index * cell_w + left, source_y,
                                width, cell_h, cursor, y);
            }
            cursor += advance;
        }
    }

    void blit_hud_glyph_run_centered(Rect area, const std::string& value, bool large) {
        const int cell_h = large ? 30 : 18;
        const int width = hud_glyph_run_width(value, large);
        blit_hud_glyph_run(area.x + (area.w - width) / 2,
                           area.y + (area.h - cell_h) / 2,
                           value, large);
    }

    void draw_hud_stat(int x, int icon, int value) {
        blit_hud_region(hud_chrome_, 780 + icon * 32, 72, 32, 32, x, 40);
        blit_hud_glyph_run(x + 31, 47, std::to_string(value), false);
    }

    void draw_top_hud() {
        ensure_hud_graphics();
        if (!hud_chrome_.valid() || !hud_glyphs_.valid()) return;

        // The top of the game is one graphical screen surface.  We only copy
        // authored pixels here; there are no runtime HUD boxes, lines, circles
        // or font cells constructing its appearance.
        for (int x = 0; x < fb_.width(); x += 256) {
            blit_hud_region(hud_chrome_, 0, 0, 256, 72, x, 0);
        }

        int speed_state = 1;
        if (paused_) speed_state = 0;
        else if (simulation_speed_ >= 4) speed_state = 3;
        else if (simulation_speed_ >= 2) speed_state = 2;
        blit_hud_region(hud_chrome_, 256 + speed_state * 180, 0, 180, 52, 0, 0);
        blit_hud_glyph_run_centered({140, 10, 26, 26},
                                    "X" + std::to_string(simulation_speed_), false);

        const Rect date_box{fb_.width() / 2 - 180, 1, 360, 48};
        blit_hud_region(hud_chrome_, 0, 72, 360, 48, date_box.x, date_box.y);
        const std::string date = std::string(world_.month_name()) + " " +
            std::to_string(world_.year_bc()) + " BC";
        blit_hud_glyph_run_centered(date_box, date, true);

        const int left = std::max(184, date_box.x - 224);
        const int right = date_box.x + date_box.w + 8;
        draw_hud_stat(left + 0 * 72, 0, world_.population());
        draw_hud_stat(left + 1 * 72, 1, world_.immigrants_in_transit());
        draw_hud_stat(left + 2 * 72, 2, world_.employed_population());
        draw_hud_stat(right + 0 * 72, 3, world_.total_food());
        draw_hud_stat(right + 1 * 72, 4, world_.total_pottery());
        draw_hud_stat(right + 2 * 72, 5, world_.treasury());

        const Rect menu = main_menu_rect();
        const bool hot = menu.contains(mx_, my_);
        blit_hud_region(hud_chrome_, hot ? 570 : 360, 72, 210, 48, menu.x, menu.y);
        blit_hud_glyph_run_centered(menu, "MAIN MENU", false);
    }

    void draw_tool_ribbon() {
        ensure_hud_graphics();
        if (!tool_icons_.valid()) return;
        const auto tools = tool_buttons();
        for (int i = 0; i < static_cast<int>(tools.size()); ++i) {
            const bool selected = static_cast<int>(tool_) == i;
            const bool hot = tools[i].contains(mx_, my_);
            const int source_y = (selected || hot) ? 48 : 0;
            blit_hud_region(tool_icons_, i * 48, source_y, 48, 48,
                            tools[i].x, tools[i].y);
        }
    }
