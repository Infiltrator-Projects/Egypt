public:
    explicit Game(Framebuffer& fb)
        : fb_(fb), menu_(common::load_e16("build/assets/menu.e16")), view_w_(fb.width()), view_h_(fb.height()) {
        world_.lay_initial_kingdom_road();
        if (!menu_.valid()) std::cerr << "Egypt: menu artwork failed to load\n";
    }

    [[nodiscard]] bool running() const { return running_; }
    [[nodiscard]] bool dirty() const { return dirty_; }
    void rendered() { dirty_ = false; }
    void request_quit() { running_ = false; }

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
        if (screen_ != Screen::Game || dragging_) return;

        double dx = 0.0;
        double dy = 0.0;
        const double keyboard_speed = scroll_speed_px_ * 1.15;
        if (pan_left_) dx += keyboard_speed * dt;
        if (pan_right_) dx -= keyboard_speed * dt;
        if (pan_up_) dy += keyboard_speed * dt;
        if (pan_down_) dy -= keyboard_speed * dt;

        if (edge_scroll_) {
            constexpr int margin = 48;
            constexpr int hard_edge = 8;
            auto strength = [](int distance) {
                const double penetration = 1.0 - std::clamp(double(distance) / double(margin), 0.0, 1.0);
                return 0.55 + 0.45 * penetration;
            };

            if (mx_ < margin) dx += scroll_speed_px_ * strength(mx_) * dt;
            else if (mx_ >= fb_.width() - margin) dx -= scroll_speed_px_ * strength(fb_.width() - 1 - mx_) * dt;

            const bool over_ui = over_game_ui(mx_, my_);
            if (my_ < margin && (!over_ui || my_ <= hard_edge)) {
                dy += scroll_speed_px_ * strength(my_) * dt;
            } else if (my_ >= fb_.height() - margin && (!over_ui || my_ >= fb_.height() - 1 - hard_edge)) {
                dy -= scroll_speed_px_ * strength(fb_.height() - 1 - my_) * dt;
            }
        }

        apply_camera_motion(dx, dy);
    }

    bool take_resize_request(int& w, int& h) {
        if (!resize_pending_) return false;
        w = requested_w_;
        h = requested_h_;
        resize_pending_ = false;
        return true;
    }

    void on_key_press(Key key) {
        if (key == Key::Escape) {
            pan_left_ = pan_right_ = pan_up_ = pan_down_ = false;
            if (screen_ == Screen::Game || screen_ == Screen::Settings) screen_ = Screen::Menu;
            else running_ = false;
            dragging_ = false;
            dirty_ = true;
            return;
        }
        if (screen_ != Screen::Game) return;

        set_pan_key(key, true);
        if (key == Key::Plus) zoom_by(10);
        else if (key == Key::Minus) zoom_by(-10);
        else if (key == Key::Home) reset_camera();
        else if (key == Key::Space) paused_ = !paused_;
        else if (key == Key::F) flat_mode_ = !flat_mode_;
        update_hover();
        dirty_ = true;
    }

    void on_key_release(Key key) {
        set_pan_key(key, false);
    }

    void on_motion(int x, int y) {
        const int oldx = hover_x_, oldy = hover_y_;
        if (dragging_) {
            cam_.pan_x += x - drag_last_x_;
            cam_.pan_y += y - drag_last_y_;
            drag_last_x_ = x;
            drag_last_y_ = y;
            pan_fraction_x_ = 0.0;
            pan_fraction_y_ = 0.0;
        }
        mx_ = x;
        my_ = y;
        update_hover();
        if (dragging_ || oldx != hover_x_ || oldy != hover_y_ || screen_ != Screen::Game) dirty_ = true;
    }

    void on_button_press(MouseButton button, int x, int y) {
        mx_ = x;
        my_ = y;
        if (screen_ == Screen::Game && (button == MouseButton::WheelUp || button == MouseButton::WheelDown)) {
            zoom_by(button == MouseButton::WheelUp ? 10 : -10);
            update_hover();
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Game && (button == MouseButton::Middle || button == MouseButton::Right) && drag_pan_) {
            dragging_ = true;
            drag_button_ = button;
            drag_last_x_ = x;
            drag_last_y_ = y;
            return;
        }
        if (button == MouseButton::Left) on_click(x, y);
    }

    void on_button_release(MouseButton button, int x, int y) {
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
