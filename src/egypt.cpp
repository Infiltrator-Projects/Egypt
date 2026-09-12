#include "game.hpp"

namespace egypt {

class X11App {
public:
    X11App(int w,int h):fb_(w,h),game_(fb_){
        display_=XOpenDisplay(nullptr);if(!display_)throw std::runtime_error("Unable to open X11 display");
        screen_=DefaultScreen(display_);
        window_=XCreateSimpleWindow(display_,RootWindow(display_,screen_),100,100,w,h,0,BlackPixel(display_,screen_),BlackPixel(display_,screen_));
        XStoreName(display_,window_,"Egypt");
        XSelectInput(display_,window_,ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask);
        delete_atom_=XInternAtom(display_,"WM_DELETE_WINDOW",False);XSetWMProtocols(display_,window_,&delete_atom_,1);
        gc_=XCreateGC(display_,window_,0,nullptr);XMapWindow(display_,window_);recreate(w,h);
        if(!infiltratr_fixed_step_configure(&scheduler_,1000000000ULL,4ULL,500000000ULL,8ULL))throw std::runtime_error("Common fixed-step scheduler configuration failed");
        infiltratr_fixed_step_reset(&scheduler_,now_ns());last_frame_ns_=now_ns();
    }

    ~X11App(){
        if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);}
        if(gc_) XFreeGC(display_,gc_);
        if(window_) XDestroyWindow(display_,window_);
        if(display_) XCloseDisplay(display_);
    }

    int run(){
        while(game_.running()){
            while(XPending(display_)>0){
                XEvent event;XNextEvent(display_,&event);
                switch(event.type){
                    case Expose:game_.resize();break;
                    case ConfigureNotify:
                        if(event.xconfigure.width!=fb_.width()||event.xconfigure.height!=fb_.height()){
                            fb_.resize(event.xconfigure.width,event.xconfigure.height);recreate(event.xconfigure.width,event.xconfigure.height);game_.resize();
                        }
                        break;
                    case MotionNotify:game_.on_motion(event.xmotion.x,event.xmotion.y);break;
                    case ButtonPress:game_.on_button_press(event.xbutton.button,event.xbutton.x,event.xbutton.y);break;
                    case ButtonRelease:game_.on_button_release(event.xbutton.button,event.xbutton.x,event.xbutton.y);break;
                    case KeyPress:game_.on_key(XLookupKeysym(&event.xkey,0));break;
                    case ClientMessage:if(static_cast<Atom>(event.xclient.data.l[0])==delete_atom_)return 0;break;
                    default:break;
                }
            }
            const std::uint64_t now=now_ns();
            const double dt=static_cast<double>(now-last_frame_ns_)/1000000000.0;last_frame_ns_=now;
            game_.frame(std::clamp(dt,0.0,0.05));
            int rw=0,rh=0;if(game_.take_resize_request(rw,rh))XResizeWindow(display_,window_,static_cast<unsigned>(rw),static_cast<unsigned>(rh));
            InfiltratrFixedStepResult result{};
            if(infiltratr_fixed_step_advance(&scheduler_,now,&result))for(std::uint64_t i=0;i<result.steps_to_run;++i)game_.tick();
            if(game_.dirty()){game_.draw();present();game_.rendered();}
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
        return 0;
    }

private:
    Display* display_=nullptr;int screen_=0;Window window_=0;GC gc_=0;Atom delete_atom_=0;XImage* image_=nullptr;
    Framebuffer fb_;Game game_;InfiltratrFixedStepScheduler scheduler_{};std::uint64_t last_frame_ns_=0;

    static std::uint64_t now_ns(){return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
    static unsigned long pack(std::uint8_t value,unsigned long mask){if(!mask)return 0;unsigned shift=0;while(((mask>>shift)&1UL)==0UL)++shift;const unsigned long max=mask>>shift;return((static_cast<unsigned long>(value)*max+127UL)/255UL<<shift)&mask;}

    void recreate(int w,int h){
        if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);image_=nullptr;}
        image_=XCreateImage(display_,DefaultVisual(display_,screen_),DefaultDepth(display_,screen_),ZPixmap,0,nullptr,w,h,32,0);
        if(!image_)throw std::runtime_error("Unable to create XImage");
        image_->data=static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line)*h,1));if(!image_->data)throw std::bad_alloc();
    }

    void present(){
        const auto& pixels=fb_.pixels();const int w=fb_.width(),h=fb_.height();
        for(int y=0;y<h;++y)for(int x=0;x<w;++x){const Color c=pixels[static_cast<std::size_t>(y*w+x)];XPutPixel(image_,x,y,pack(c.r,image_->red_mask)|pack(c.g,image_->green_mask)|pack(c.b,image_->blue_mask));}
        XPutImage(display_,window_,gc_,image_,0,0,0,0,w,h);XFlush(display_);
    }
};

} // namespace egypt

int main(){
    try{egypt::X11App app(1280,720);return app.run();}
    catch(const std::exception& error){std::cerr<<"Egypt failed to start: "<<error.what()<<'\n';return 1;}
}
