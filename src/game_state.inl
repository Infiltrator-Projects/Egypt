private:
    Framebuffer& fb_;
    ImageAsset menu_;
    World world_;
    IsoCamera cam_;
    Screen screen_ = Screen::Menu;
    Tool tool_ = Tool::Inspect;
    bool running_ = true;
    bool dirty_ = true;
    bool edge_scroll_ = true;
    bool drag_pan_ = true;
    bool dragging_ = false;
    bool resize_pending_ = false;
    bool paused_ = false;
    bool flat_mode_ = false;
    unsigned drag_button_ = 0;
    int mx_ = 0, my_ = 0;
    int hover_x_ = -1, hover_y_ = -1;
    int selected_x_ = -1, selected_y_ = -1;
    int drag_last_x_ = 0, drag_last_y_ = 0;
    int view_w_ = 0, view_h_ = 0;
    int requested_w_ = 1280, requested_h_ = 720, resolution_index_ = 0;
    int simulation_speed_ = 1;
    double scroll_speed_px_ = 620.0;
    double atmosphere_time_ = 0.0;
    double atmosphere_redraw_ = 0.0;
    std::string status_ = "PRE-ALPHA - NATIVE ENGINE";

    const Color gold{217,177,95}, pale{242,215,154}, panel{19,14,12}, hi{55,40,27};
    static constexpr std::array<std::array<int,2>,3> kResolutions{{{{1280,720}},{{1600,900}},{{1920,1080}}}};

    void zoom_by(int delta) { cam_.zoom_percent = std::clamp(cam_.zoom_percent + delta, 50, 180); }

    void reset_camera() {
        cam_.origin_x = fb_.width() / 2 + 80;
        cam_.origin_y = 150;
        cam_.pan_x = -260;
        cam_.pan_y = -30;
        cam_.zoom_percent = 90;
        update_hover();
    }

    void center_camera_on(int tx, int ty) {
        const IsoPoint p = cam_.project(tx, ty);
        cam_.pan_x += fb_.width() / 2 - p.x;
        cam_.pan_y += fb_.height() / 2 - p.y;
        update_hover();
    }

    Rect main_menu_rect() const { return {fb_.width() - 210, 18, 190, 42}; }
    Rect minimap_rect() const { return {12, std::max(84, fb_.height() - 230), 196, 116}; }

    std::array<Rect,4> speed_rects() const {
        return {Rect{12,16,66,34},Rect{84,16,54,34},Rect{144,16,54,34},Rect{204,16,54,34}};
    }

    bool over_game_ui(int x, int y) const {
        if (y < 78 || y >= fb_.height() - 104) return true;
        if (main_menu_rect().contains(x, y) || minimap_rect().contains(x, y)) return true;
        return Rect{fb_.width() - 520, 82, 510, 250}.contains(x, y);
    }

    std::array<Rect,4> menu_buttons() const {
        const int x = 72, y = 300, w = 340, h = 50, gap = 12;
        return {Rect{x,y,w,h}, Rect{x,y+h+gap,w,h}, Rect{x,y+2*(h+gap),w,h}, Rect{x,y+3*(h+gap),w,h}};
    }

    int menu_button_at(int x, int y) const {
        const auto buttons = menu_buttons();
        for (int i = 0; i < 4; ++i) if (buttons[i].contains(x, y)) return i;
        return -1;
    }

    std::array<Rect,11> tool_buttons() const {
        std::array<Rect,11> result{};
        const int w = 112, h = 36, gap = 6;
        const int first_y = fb_.height() - 92;
        for (int i = 0; i < 6; ++i) result[i] = {14 + i * (w + gap), first_y, w, h};
        for (int i = 6; i < 11; ++i) result[i] = {14 + (i - 6) * (w + gap), first_y + 42, w, h};
        return result;
    }

    Rect settings_back() const { return {fb_.width()/2 - 100, fb_.height() - 92, 200, 44}; }
    std::array<Rect,3> resolution_buttons() const {
        const int cx = fb_.width()/2;
        return {Rect{cx-330,240,200,44},Rect{cx-100,240,200,44},Rect{cx+130,240,200,44}};
    }
    Rect edge_scroll_rect() const { return {fb_.width()/2 - 165, 338, 330, 44}; }
    std::array<Rect,3> scroll_speed_buttons() const {
        const int cx=fb_.width()/2;
        return {Rect{cx-260,440,160,44},Rect{cx-80,440,160,44},Rect{cx+100,440,160,44}};
    }

    void button(Rect r, const std::string& label, bool enabled, bool active = false, int scale = 2) {
        fb_.blend_rect(r, active ? hi : panel, active ? 235 : 205);
        fb_.rect(r, enabled ? gold : Color{90,78,65}, 2);
        text(fb_, r.x + 7, r.y + std::max(5,(r.h-7*scale)/2), label, enabled ? pale : Color{115,105,92}, scale);
    }

    Structure tool_structure() const {
        switch (tool_) {
            case Tool::Road: return Structure::Road;
            case Tool::House: return Structure::House;
            case Tool::Farm: return Structure::Farm;
            case Tool::Granary: return Structure::Granary;
            case Tool::Market: return Structure::Market;
            case Tool::Well: return Structure::Well;
            case Tool::HuntingLodge: return Structure::HuntingLodge;
            case Tool::ClayPit: return Structure::ClayPit;
            case Tool::Potter: return Structure::Potter;
            default: return Structure::Empty;
        }
    }

    void on_click(int x, int y) {
        mx_ = x;
        my_ = y;
        if (screen_ == Screen::Menu) {
            const int choice = menu_button_at(x, y);
            if (choice == 0) { screen_ = Screen::Game; status_.clear(); reset_camera(); }
            else if (choice == 2) screen_ = Screen::Settings;
            else if (choice == 3) running_ = false;
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Settings) { handle_settings_click(x, y); return; }

        if (main_menu_rect().contains(x, y)) { screen_ = Screen::Menu; dirty_ = true; return; }
        const auto sr = speed_rects();
        if (sr[0].contains(x,y)) { paused_ = !paused_; dirty_ = true; return; }
        if (sr[1].contains(x,y)) { paused_ = false; simulation_speed_ = 1; dirty_ = true; return; }
        if (sr[2].contains(x,y)) { paused_ = false; simulation_speed_ = 2; dirty_ = true; return; }
        if (sr[3].contains(x,y)) { paused_ = false; simulation_speed_ = 4; dirty_ = true; return; }

        if (minimap_rect().contains(x, y)) {
            const Rect r = minimap_rect();
            const int tx = std::clamp((x - r.x - 2) * World::kWidth / std::max(1, r.w - 4), 0, World::kWidth - 1);
            const int ty = std::clamp((y - r.y - 2) * World::kHeight / std::max(1, r.h - 4), 0, World::kHeight - 1);
            center_camera_on(tx, ty);
            dirty_ = true;
            return;
        }

        const auto tools = tool_buttons();
        for (int i = 0; i < static_cast<int>(tools.size()); ++i) {
            if (tools[i].contains(x, y)) {
                tool_ = static_cast<Tool>(i);
                dirty_ = true;
                return;
            }
        }

        update_hover();
        if (!world_.in_bounds(hover_x_, hover_y_)) return;
        selected_x_ = hover_x_;
        selected_y_ = hover_y_;
        if (tool_ == Tool::Bulldoze) world_.bulldoze(selected_x_, selected_y_);
        else if (tool_ != Tool::Inspect) world_.place(tool_structure(), selected_x_, selected_y_);
        dirty_ = true;
    }

    void update_hover() {
        if (screen_ != Screen::Game) { hover_x_ = hover_y_ = -1; return; }
        int x = 0, y = 0;
        if (!cam_.pick(mx_, my_, x, y) || !world_.in_bounds(x, y)) { hover_x_ = hover_y_ = -1; return; }
        hover_x_ = x;
        hover_y_ = y;
    }
