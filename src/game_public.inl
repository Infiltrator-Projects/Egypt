public:
    explicit Game(Framebuffer& fb)
        : fb_(fb), menu_(common::load_e16("build/assets/menu.e16")), view_w_(fb.width()), view_h_(fb.height()) {
        if (!menu_.valid()) std::cerr << "Egypt: menu artwork failed to load\n";
    }

    [[nodiscard]] bool running() const { return running_; }
    [[nodiscard]] bool dirty() const { return dirty_; }
    void rendered() { dirty_ = false; }

    void resize() {
        const int dx = fb_.width() - view_w_;
        const int dy = fb_.height() - view_h_;
        cam_.origin_x += dx / 2;
        cam_.origin_y += dy / 3;
        view_w_ = fb_.width();
        view_h_ = fb_.height();
        update_hover();
        dirty_ = true;
    }

    void tick() {
        if (screen_ != Screen::Game || paused_) return;
        ++simulation_subtick_;
        const std::uint64_t divisor = simulation_speed_ >= 4 ? 1U : (simulation_speed_ == 2 ? 2U : 4U);
        if ((simulation_subtick_ % divisor) == 0U) world_.tick();
        dirty_ = true;
    }

    void frame(double dt) {
        atmosphere_time_ += dt;
        atmosphere_redraw_ += dt;
        if (screen_ == Screen::Game && atmosphere_redraw_ >= (1.0 / 30.0)) {
            atmosphere_redraw_ = 0.0;
            dirty_ = true;
        }
        if (screen_ != Screen::Game || dragging_ || !edge_scroll_ || over_game_ui(mx_, my_)) return;
        const int margin = 18;
        double dx = 0.0, dy = 0.0;
        if (mx_ <= margin) dx += scroll_speed_px_ * dt;
        else if (mx_ >= fb_.width() - 1 - margin) dx -= scroll_speed_px_ * dt;
        if (my_ <= 78 + margin) dy += scroll_speed_px_ * dt;
        else if (my_ >= fb_.height() - 1 - margin) dy -= scroll_speed_px_ * dt;
        if (dx != 0.0 || dy != 0.0) {
            cam_.pan_x += static_cast<int>(std::lround(dx));
            cam_.pan_y += static_cast<int>(std::lround(dy));
            update_hover();
            dirty_ = true;
        }
    }

    bool take_resize_request(int& w, int& h) {
        if (!resize_pending_) return false;
        w = requested_w_;
        h = requested_h_;
        resize_pending_ = false;
        return true;
    }

    void on_key(KeySym key) {
        if (key == XK_Escape) {
            if (screen_ == Screen::Game || screen_ == Screen::Settings) screen_ = Screen::Menu;
            else running_ = false;
            dragging_ = false;
            dirty_ = true;
            return;
        }
        if (screen_ != Screen::Game) return;
        const int step = 48;
        if (key == XK_Left || key == XK_a || key == XK_A) cam_.pan_x += step;
        else if (key == XK_Right || key == XK_d || key == XK_D) cam_.pan_x -= step;
        else if (key == XK_Up || key == XK_w || key == XK_W) cam_.pan_y += step;
        else if (key == XK_Down || key == XK_s || key == XK_S) cam_.pan_y -= step;
        else if (key == XK_plus || key == XK_equal) zoom_by(10);
        else if (key == XK_minus) zoom_by(-10);
        else if (key == XK_Home) reset_camera();
        else if (key == XK_space) paused_ = !paused_;
        else if (key == XK_f || key == XK_F) flat_mode_ = !flat_mode_;
        update_hover();
        dirty_ = true;
    }

    void on_motion(int x, int y) {
        const int oldx = hover_x_, oldy = hover_y_;
        if (dragging_) {
            cam_.pan_x += x - drag_last_x_;
            cam_.pan_y += y - drag_last_y_;
            drag_last_x_ = x;
            drag_last_y_ = y;
        }
        mx_ = x;
        my_ = y;
        update_hover();
        if (dragging_ || oldx != hover_x_ || oldy != hover_y_ || screen_ != Screen::Game) dirty_ = true;
    }

    void on_button_press(unsigned button, int x, int y) {
        mx_ = x;
        my_ = y;
        if (screen_ == Screen::Game && (button == Button4 || button == Button5)) {
            zoom_by(button == Button4 ? 10 : -10);
            update_hover();
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Game && (button == Button2 || button == Button3) && drag_pan_) {
            dragging_ = true;
            drag_button_ = button;
            drag_last_x_ = x;
            drag_last_y_ = y;
            return;
        }
        if (button == Button1) on_click(x, y);
    }

    void on_button_release(unsigned button, int x, int y) {
        mx_ = x;
        my_ = y;
        if (dragging_ && button == drag_button_) {
            dragging_ = false;
            update_hover();
            dirty_ = true;
        }
    }

    void draw() {
        if (screen_ == Screen::Menu) draw_menu();
        else if (screen_ == Screen::Settings) draw_settings();
        else draw_game();
    }
