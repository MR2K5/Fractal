#pragma once

#include <config.hpp>
#include <gtkmm-4.0/gtkmm.h>

class InputCapture {
    vec2 top_left;
    vec2 size;
    double scale;
    double const step = 0.1;

    Glib::RefPtr<Gtk::EventControllerMotion> mouse_input;
    Glib::RefPtr<Gtk::GestureDrag> drag_input;
    Glib::RefPtr<Gtk::EventControllerScroll> scroll_input;

    vec2 last_pan;
    vec2 mouse_pos;

    bool mouse_inside = false;

    void on_resize(int w, int h);

    void mouse_move(double x, double y);

    void drag_beg(double x, double y);
    void drag(double x, double y);

    bool scroll(double dx, double dy);

    sigc::signal<void()> sig_changed;

public:
    vec2 get_top_left() const { return screen_to_world({0, 0}); }
    vec2 get_bottom_right() const { return screen_to_world(size); }
    vec2 get_mouse_pos() const noexcept { return mouse_pos; }
    bool mouse_is_inside() const { return mouse_inside; }
    bool is_inside(vec2 const& screenpos) {
        return 0 <= screenpos.x() && screenpos.x() < size.x()
            && 0 <= screenpos.y() && screenpos.y() < size.y();
    }

    vec2 world_to_screen(vec2 const& world) const noexcept {
        return (world - top_left) * scale;
    }
    vec2 screen_to_world(vec2 const& screen) const noexcept {
        return screen / scale + top_left;
    }

    InputCapture(Gtk::DrawingArea& frame);

    auto signal_changed() { return sig_changed; }
    auto signal_mouse_moved() { return mouse_input->signal_motion(); }
};
