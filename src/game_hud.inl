    Color hud_mix(Color a, Color b, int t, int denom) const {
        if (denom <= 0) return b;
        t = std::clamp(t, 0, denom);
        const int inv = denom - t;
        return {
            static_cast<std::uint8_t>((a.r * inv + b.r * t) / denom),
            static_cast<std::uint8_t>((a.g * inv + b.g * t) / denom),
            static_cast<std::uint8_t>((a.b * inv + b.b * t) / denom)
        };
    }

    void hud_gradient(Rect r, Color top, Color bottom) {
        const int h = std::max(1, r.h - 1);
        for (int y = 0; y < r.h; ++y) {
            fb_.fill_span(r.y + y, r.x, r.x + r.w - 1, hud_mix(top, bottom, y, h));
        }
    }

    int hud_text_width(const std::string& value, int scale) const {
        if (value.empty()) return 0;
        return static_cast<int>(value.size()) * 6 * scale - scale;
    }

    void hud_text(int x, int y, const std::string& value, Color color, int scale = 2) {
        // The base font is deliberately tiny, but the normal renderer used to
        // scale each source pixel into a hard square.  For the HUD we feather
        // each occupied cell into neighbouring pixels and add a dark engraved
        // shadow.  It keeps the engine dependency-free while avoiding the
        // giant 1980s bitmap-block appearance of the old header.
        const Color shadow{47, 24, 17};
        int cursor = x;
        for (char raw : value) {
            const char ch = raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : raw;
            const auto it = FONT.find(ch);
            const Glyph& glyph = it == FONT.end() ? FONT.at(' ') : it->second;
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (((glyph[row] >> (4 - col)) & 1U) == 0U) continue;
                    const int px = cursor + col * scale;
                    const int py = y + row * scale;
                    fb_.blend_rect({px, py + 1, scale + 1, scale + 1}, shadow, 150);
                    if (scale >= 2) {
                        fb_.blend_rect({px - 1, py, 1, scale}, color, 82);
                        fb_.blend_rect({px + scale, py, 1, scale}, color, 82);
                        fb_.blend_rect({px, py - 1, scale, 1}, color, 68);
                        fb_.blend_rect({px, py + scale, scale, 1}, color, 68);
                    }
                    fb_.fill_rect({px, py, scale, scale}, color);
                }
            }
            cursor += 6 * scale;
        }
    }

    void hud_text_centered(Rect r, const std::string& value, Color color, int scale = 2) {
        hud_text(r.x + (r.w - hud_text_width(value, scale)) / 2,
                 r.y + (r.h - 7 * scale) / 2,
                 value, color, scale);
    }

    void fill_hud_disc(int cx, int cy, int radius, Color color) {
        for (int yy = -radius; yy <= radius; ++yy) {
            const int xx = static_cast<int>(std::sqrt(std::max(0, radius * radius - yy * yy)));
            fb_.fill_span(cy + yy, cx - xx, cx + xx, color);
        }
    }

    void hud_disc_ring(int cx, int cy, int radius, Color outer, Color inner) {
        fill_hud_disc(cx, cy, radius, outer);
        if (radius > 1) fill_hud_disc(cx, cy, radius - 2, inner);
    }

    void draw_hud_hieroglyph_texture(int x0, int x1, int y0, int y1) {
        const Color mark{91, 53, 31};
        const Color mark_hi{116, 70, 38};
        for (int x = x0; x < x1; x += 22) {
            const int seed = (x / 22) & 3;
            if (seed == 0) {
                fb_.line(x + 5, y0 + 6, x + 5, y1 - 6, mark);
                fb_.line(x + 2, y0 + 12, x + 8, y0 + 12, mark_hi);
                fb_.fill_rect({x + 3, y0 + 20, 5, 3}, mark);
            } else if (seed == 1) {
                fb_.line(x + 3, y0 + 8, x + 10, y0 + 16, mark_hi);
                fb_.line(x + 10, y0 + 16, x + 3, y0 + 24, mark);
                fb_.fill_rect({x + 5, y0 + 29, 4, 4}, mark);
            } else if (seed == 2) {
                fb_.fill_rect({x + 3, y0 + 7, 8, 2}, mark_hi);
                fb_.line(x + 7, y0 + 9, x + 7, y0 + 24, mark);
                fb_.line(x + 2, y0 + 28, x + 12, y0 + 28, mark_hi);
            } else {
                fb_.line(x + 2, y0 + 10, x + 12, y0 + 10, mark);
                fb_.line(x + 4, y0 + 16, x + 10, y0 + 16, mark_hi);
                fb_.line(x + 6, y0 + 22, x + 8, y0 + 30, mark);
            }
        }
    }

    void draw_hud_bevel(Rect r, Color face, bool active = false) {
        const Color outer{42, 22, 16};
        const Color gold_hi = active ? Color{241, 201, 116} : Color{194, 139, 69};
        const Color gold_lo{107, 61, 31};
        fb_.fill_rect(r, outer);
        fb_.fill_rect({r.x + 2, r.y + 2, r.w - 4, r.h - 4}, gold_lo);
        hud_gradient({r.x + 3, r.y + 3, r.w - 6, r.h - 6},
                     hud_mix(face, {146, 83, 43}, 1, 5),
                     hud_mix(face, {39, 22, 17}, 1, 4));
        fb_.line(r.x + 3, r.y + 3, r.x + r.w - 4, r.y + 3, gold_hi);
        fb_.line(r.x + 3, r.y + 3, r.x + 3, r.y + r.h - 4, gold_hi);
        fb_.line(r.x + 3, r.y + r.h - 4, r.x + r.w - 4, r.y + r.h - 4, {75, 39, 24});
        fb_.line(r.x + r.w - 4, r.y + 3, r.x + r.w - 4, r.y + r.h - 4, {75, 39, 24});
    }

    void draw_hud_corner_gem(int x, int y, bool mirror) {
        const Color dark{64, 32, 22};
        const Color bronze{156, 99, 49};
        const Color bright{224, 175, 87};
        const int s = mirror ? -1 : 1;
        fb_.line(x, y, x + s * 12, y, bright);
        fb_.line(x, y, x, y + 12, bright);
        fb_.line(x + s * 3, y + 3, x + s * 9, y + 3, bronze);
        fb_.line(x + s * 3, y + 3, x + s * 3, y + 9, bronze);
        fb_.triangle(x + s * 2, y + 2, x + s * 8, y + 2, x + s * 2, y + 8, dark);
    }

    void draw_speed_medallion(Rect r, int arrows, bool pause_icon, bool active) {
        const int cx = r.x + r.w / 2;
        const int cy = r.y + r.h / 2;
        const int radius = std::min(r.w, r.h) / 2;
        const Color deep{45, 23, 17};
        const Color bronze{120, 70, 34};
        const Color rim = active ? Color{242, 200, 109} : Color{203, 149, 73};
        const Color face_top = active ? Color{99, 53, 29} : Color{76, 41, 27};
        const Color icon{245, 217, 159};

        fill_hud_disc(cx + 1, cy + 2, radius, deep);
        hud_disc_ring(cx, cy, radius, rim, bronze);
        fill_hud_disc(cx, cy, radius - 4, deep);
        fill_hud_disc(cx, cy - 1, radius - 6, face_top);
        fb_.line(cx - radius / 2, cy - radius + 3, cx + radius / 2, cy - radius + 3, {244, 195, 100});
        fb_.pixel(cx - radius + 3, cy, {226, 173, 83});
        fb_.pixel(cx + radius - 3, cy, {91, 49, 27});

        if (pause_icon) {
            fb_.fill_rect({cx - 5, cy - 7, 3, 14}, {60, 31, 21});
            fb_.fill_rect({cx + 2, cy - 7, 3, 14}, {60, 31, 21});
            fb_.fill_rect({cx - 5, cy - 8, 2, 14}, icon);
            fb_.fill_rect({cx + 2, cy - 8, 2, 14}, icon);
            return;
        }

        const int count = std::clamp(arrows, 1, 3);
        const int tri_w = 6;
        const int gap = 1;
        const int total = count * tri_w + (count - 1) * gap;
        int sx = cx - total / 2;
        for (int i = 0; i < count; ++i) {
            fb_.triangle(sx + 1, cy - 6, sx + 1, cy + 6, sx + tri_w + 1, cy, {58, 30, 20});
            fb_.triangle(sx, cy - 7, sx, cy + 5, sx + tri_w, cy - 1, icon);
            sx += tri_w + gap;
        }
    }

    void draw_hud_wing(int root_x, int root_y, int direction) {
        const Color gold1{191, 135, 62};
        const Color gold2{115, 67, 35};
        for (int i = 0; i < 5; ++i) {
            const int y = root_y + i * 3;
            const int len = 42 - i * 6;
            fb_.line(root_x, root_y + 9, root_x + direction * len, y, gold1);
            fb_.line(root_x, root_y + 10, root_x + direction * (len - 2), y + 2, gold2);
        }
        fb_.triangle(root_x, root_y + 5,
                     root_x + direction * 15, root_y + 1,
                     root_x + direction * 12, root_y + 15,
                     {123, 70, 34});
    }

    void draw_hud_lotus(int cx, int cy) {
        const Color bronze{167, 105, 49};
        const Color bright{222, 164, 77};
        fb_.line(cx, cy + 10, cx, cy - 10, bronze);
        fb_.triangle(cx, cy - 10, cx - 7, cy - 1, cx, cy - 3, bright);
        fb_.triangle(cx, cy - 10, cx + 7, cy - 1, cx, cy - 3, bright);
        fb_.triangle(cx, cy - 7, cx - 10, cy + 3, cx - 1, cy, bronze);
        fb_.triangle(cx, cy - 7, cx + 10, cy + 3, cx + 1, cy, bronze);
        fb_.line(cx - 7, cy + 7, cx + 7, cy + 7, bronze);
    }

    void draw_top_hud() {
        constexpr int hud_h = 72;
        const Color top{91, 49, 30};
        const Color bottom{57, 29, 22};
        const Color dark{45, 23, 17};
        const Color bronze{128, 75, 37};
        const Color warm_gold{213, 160, 78};
        const Color ink{242, 216, 163};

        // Main carved timber/stone band.
        hud_gradient({0, 0, fb_.width(), hud_h}, top, bottom);
        fb_.fill_rect({0, 0, fb_.width(), 3}, {48, 25, 18});
        fb_.fill_rect({0, hud_h - 5, fb_.width(), 5}, dark);
        fb_.fill_rect({0, hud_h - 5, fb_.width(), 1}, warm_gold);
        fb_.fill_rect({0, hud_h - 2, fb_.width(), 1}, bronze);
        draw_hud_hieroglyph_texture(170, std::max(170, fb_.width() - 220), 4, 44);

        // Left control housing is a shaped wedge, not a generic rectangle.
        const IsoPoint l0{0, 2}, l1{170, 2}, l2{153, 49}, l3{0, 49};
        fb_.quad(l0, l1, l2, l3, {61, 31, 22});
        fb_.line(0, 49, 153, 49, {212, 153, 70});
        fb_.line(170, 2, 153, 49, {212, 153, 70});
        fb_.line(167, 4, 150, 47, {88, 49, 27});

        const auto sr = speed_rects();
        draw_speed_medallion(sr[0], 0, true, paused_);
        draw_speed_medallion(sr[1], 1, false, !paused_ && simulation_speed_ == 1);
        draw_speed_medallion(sr[2], 2, false, !paused_ && simulation_speed_ == 2);
        draw_speed_medallion(sr[3], 3, false, !paused_ && simulation_speed_ == 4);
        hud_text(143, 16, "X" + std::to_string(simulation_speed_), ink, 1);

        // Central date cartouche with winged ornament, matching the reference's
        // visual hierarchy rather than floating text on a black strip.
        const int plaque_w = 274;
        const int plaque_x = fb_.width() / 2 - plaque_w / 2;
        const Rect date_box{plaque_x, 5, plaque_w, 39};
        draw_hud_bevel(date_box, {91, 47, 28}, false);
        draw_hud_wing(date_box.x + 14, date_box.y + 7, -1);
        draw_hud_wing(date_box.x + date_box.w - 14, date_box.y + 7, 1);
        const std::string date = std::string(world_.month_name()) + " " + std::to_string(world_.year_bc()) + " BC";
        hud_text_centered({date_box.x + 40, date_box.y + 4, date_box.w - 80, date_box.h - 7}, date, ink, 3);

        // Narrow engraved status band under the date, as in the reference HUD.
        const std::string stats = "POP " + std::to_string(world_.population()) +
            "   IMM " + std::to_string(world_.immigrants_in_transit()) +
            "   EMP " + std::to_string(world_.employed_population()) +
            "   FOOD " + std::to_string(world_.total_food()) +
            "   POT " + std::to_string(world_.total_pottery()) +
            "   TREASURY " + std::to_string(world_.treasury());
        const int stats_w = std::min(fb_.width() - 390, std::max(360, hud_text_width(stats, 1) + 24));
        const Rect stats_box{fb_.width() / 2 - stats_w / 2, 48, stats_w, 18};
        fb_.blend_rect(stats_box, {48, 25, 18}, 165);
        fb_.line(stats_box.x, stats_box.y, stats_box.x + stats_box.w, stats_box.y, bronze);
        hud_text_centered(stats_box, stats, {225, 190, 127}, 1);

        // Right-hand menu plaque uses the same material stack as the rest of
        // the header so it no longer looks like a debug button pasted on top.
        const Rect menu = main_menu_rect();
        draw_hud_lotus(menu.x - 22, menu.y + menu.h / 2);
        draw_hud_bevel(menu, {78, 40, 26}, main_menu_rect().contains(mx_, my_));
        draw_hud_corner_gem(menu.x + 4, menu.y + 4, false);
        draw_hud_corner_gem(menu.x + menu.w - 5, menu.y + 4, true);
        hud_text_centered(menu, "MAIN MENU", ink, 2);

        if (flat_mode_) {
            hud_text(fb_.width() - 390, 53, "FLAT DIAGNOSTIC VIEW", {230, 179, 91}, 1);
        }
    }
