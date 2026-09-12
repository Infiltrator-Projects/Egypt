#include "game.hpp"
#include "platform.hpp"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <infiltratr/timing.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <thread>

namespace egypt::platform {
namespace {

Key map_key(KeySym key) {
    switch (key) {
        case XK_Escape: return Key::Escape;
        case XK_Left: return Key::Left;
        case XK_Right: return Key::Right;
        case XK_Up: return Key::Up;
        case XK_Down: return Key::Down;
        case XK_a: case XK_A: return Key::A;
        case XK_d: case XK_D: return Key::D;
        case XK_w: case XK_W: return Key::W;
        case XK_s: case XK_S: return Key::S;
        case XK_plus: case XK_equal: return Key::Plus;
        case XK_minus: return Key::Minus;
        case XK_Home: return Key::Home;
        case XK_space: return Key::Space;
        case XK_f: case XK_F: return Key::F;
        default: return Key::Unknown;
    }
}

MouseButton map_button(unsigned button) {
    switch (button) {
        case Button1: return MouseButton::Left;
        case Button2: return MouseButton::Middle;
        case Button3: return MouseButton::Right;
        case Button4: return MouseButton::WheelUp;
        case Button5: return MouseButton::WheelDown;
        default: return MouseButton::Unknown;
    }
}

class X11Host {
public:
    X11Host(Game& game, Framebuffer& framebuffer)
        : game_(game), framebuffer_(framebuffer) {
        display_ = XOpenDisplay(nullptr);
        if (!display_) throw std::runtime_error("Unable to open X11 host display");

        screen_ = DefaultScreen(display_);
        window_ = XCreateSimpleWindow(
            display_, RootWindow(display_, screen_), 100, 100,
            static_cast<unsigned>(framebuffer_.width()),
            static_cast<unsigned>(framebuffer_.height()),
            0, BlackPixel(display_, screen_), BlackPixel(display_, screen_));
        XStoreName(display_, window_, "Egypt");
        XSelectInput(display_, window_, ExposureMask | KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

        Bool detectable = False;
        XkbSetDetectableAutoRepeat(display_, True, &detectable);

        delete_atom_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display_, window_, &delete_atom_, 1);
        gc_ = XCreateGC(display_, window_, 0, nullptr);
        XMapWindow(display_, window_);
        recreate_image(framebuffer_.width(), framebuffer_.height());

        if (!infiltratr_fixed_step_configure(&scheduler_, 1000000000ULL, 16ULL, 500000000ULL, 16ULL)) {
            throw std::runtime_error("Common fixed-step scheduler configuration failed");
        }
        const std::uint64_t start = now_ns();
        infiltratr_fixed_step_reset(&scheduler_, start);
        last_frame_ns_ = start;
    }

    ~X11Host() {
        if (image_) {
            std::free(image_->data);
            image_->data = nullptr;
            XDestroyImage(image_);
        }
        if (gc_) XFreeGC(display_, gc_);
        if (window_) XDestroyWindow(display_, window_);
        if (display_) XCloseDisplay(display_);
    }

    int run() {
        constexpr std::uint64_t kPresentIntervalNs = 1000000000ULL / 60ULL;
        while (game_.running()) {
            pump_events();

            const std::uint64_t now = now_ns();
            const double dt = static_cast<double>(now - last_frame_ns_) / 1000000000.0;
            last_frame_ns_ = now;
            game_.frame(std::clamp(dt, 0.0, 0.05));

            int requested_w = 0;
            int requested_h = 0;
            if (game_.take_resize_request(requested_w, requested_h)) {
                XResizeWindow(display_, window_, static_cast<unsigned>(requested_w), static_cast<unsigned>(requested_h));
            }

            InfiltratrFixedStepResult result{};
            if (infiltratr_fixed_step_advance(&scheduler_, now, &result)) {
                for (std::uint64_t i = 0; i < result.steps_to_run; ++i) game_.tick();
            }

            if (game_.dirty() && (last_present_ns_ == 0 || now - last_present_ns_ >= kPresentIntervalNs)) {
                game_.draw();
                present();
                game_.rendered();
                last_present_ns_ = now;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        return 0;
    }

private:
    Display* display_ = nullptr;
    int screen_ = 0;
    Window window_ = 0;
    GC gc_ = 0;
    Atom delete_atom_ = 0;
    XImage* image_ = nullptr;
    Game& game_;
    Framebuffer& framebuffer_;
    InfiltratrFixedStepScheduler scheduler_{};
    std::uint64_t last_frame_ns_ = 0;
    std::uint64_t last_present_ns_ = 0;
    std::array<std::uint32_t, 256> red_lut_{}, green_lut_{}, blue_lut_{};
    bool direct_32_ = false;

    static std::uint64_t now_ns() {
        return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    static unsigned long pack(std::uint8_t value, unsigned long mask) {
        if (!mask) return 0;
        unsigned shift = 0;
        while (((mask >> shift) & 1UL) == 0UL) ++shift;
        const unsigned long max = mask >> shift;
        return ((static_cast<unsigned long>(value) * max + 127UL) / 255UL << shift) & mask;
    }

    void pump_events() {
        while (XPending(display_) > 0) {
            XEvent event;
            XNextEvent(display_, &event);
            switch (event.type) {
                case Expose:
                    game_.resize();
                    break;
                case ConfigureNotify:
                    if (event.xconfigure.width != framebuffer_.width() || event.xconfigure.height != framebuffer_.height()) {
                        framebuffer_.resize(event.xconfigure.width, event.xconfigure.height);
                        recreate_image(event.xconfigure.width, event.xconfigure.height);
                        game_.resize();
                    }
                    break;
                case MotionNotify:
                    game_.on_motion(event.xmotion.x, event.xmotion.y);
                    break;
                case ButtonPress:
                    game_.on_button_press(map_button(event.xbutton.button), event.xbutton.x, event.xbutton.y);
                    break;
                case ButtonRelease:
                    game_.on_button_release(map_button(event.xbutton.button), event.xbutton.x, event.xbutton.y);
                    break;
                case KeyPress:
                    game_.on_key_press(map_key(XLookupKeysym(&event.xkey, 0)));
                    break;
                case KeyRelease:
                    game_.on_key_release(map_key(XLookupKeysym(&event.xkey, 0)));
                    break;
                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == delete_atom_) game_.request_quit();
                    break;
                default:
                    break;
            }
        }
    }

    void rebuild_colour_tables() {
        for (int i = 0; i < 256; ++i) {
            red_lut_[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(pack(static_cast<std::uint8_t>(i), image_->red_mask));
            green_lut_[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(pack(static_cast<std::uint8_t>(i), image_->green_mask));
            blue_lut_[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(pack(static_cast<std::uint8_t>(i), image_->blue_mask));
        }
        const std::uint16_t endian_test = 1;
        const bool host_lsb = *reinterpret_cast<const std::uint8_t*>(&endian_test) == 1;
        direct_32_ = image_->bits_per_pixel == 32 &&
            ((host_lsb && image_->byte_order == LSBFirst) || (!host_lsb && image_->byte_order == MSBFirst));
    }

    void recreate_image(int width, int height) {
        if (image_) {
            std::free(image_->data);
            image_->data = nullptr;
            XDestroyImage(image_);
            image_ = nullptr;
        }
        image_ = XCreateImage(display_, DefaultVisual(display_, screen_), DefaultDepth(display_, screen_),
            ZPixmap, 0, nullptr, width, height, 32, 0);
        if (!image_) throw std::runtime_error("Unable to create X11 presentation image");
        image_->data = static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line) * height, 1));
        if (!image_->data) throw std::bad_alloc();
        rebuild_colour_tables();
    }

    void present() {
        const auto& pixels = framebuffer_.pixels();
        const int width = framebuffer_.width();
        const int height = framebuffer_.height();
        if (direct_32_) {
            for (int y = 0; y < height; ++y) {
                auto* dst = reinterpret_cast<std::uint32_t*>(image_->data + static_cast<std::size_t>(y) * image_->bytes_per_line);
                const Color* src = pixels.data() + static_cast<std::size_t>(y) * width;
                for (int x = 0; x < width; ++x) {
                    const Color c = src[x];
                    dst[x] = red_lut_[c.r] | green_lut_[c.g] | blue_lut_[c.b];
                }
            }
        } else {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const Color c = pixels[static_cast<std::size_t>(y * width + x)];
                    XPutPixel(image_, x, y, red_lut_[c.r] | green_lut_[c.g] | blue_lut_[c.b]);
                }
            }
        }
        XPutImage(display_, window_, gc_, image_, 0, 0, 0, 0, width, height);
        XFlush(display_);
    }
};

} // namespace

int run(Game& game, Framebuffer& framebuffer) {
    X11Host host(game, framebuffer);
    return host.run();
}

} // namespace egypt::platform
