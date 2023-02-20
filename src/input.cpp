#include <input.hpp>


void InputCapture::on_resize(int w, int h) { size = {w, h}; }

void InputCapture::mouse_move(double x, double y) { mouse_pos = {x, y}; }

void InputCapture::drag_beg(double x, double y) { last_pan = {0, 0}; }

void InputCapture::drag(double x, double y) {
    top_left -= (vec2{x, y} - last_pan) / scale;
    last_pan = {x, y};
    sig_changed();
}

bool InputCapture::scroll(double dx, double dy) {
    if (dy == 0) return false;
    vec2 before = screen_to_world(mouse_pos);
    if (dy < 0) {
        scale *= 1 + (dy / -100);
    } else if (dy > 0) {
        scale /= 1 + (dy / 100);
    }
    vec2 after = screen_to_world(mouse_pos);
    top_left   += (before - after);
    sig_changed();
    return true;
}

InputCapture::InputCapture(Gtk::DrawingArea &frame) {
    frame.set_focus_on_click();
    frame.set_can_focus();
    frame.set_focusable();
    frame.set_sensitive();

    mouse_input = Gtk::EventControllerMotion::create();
    mouse_input->signal_motion().connect(
                sigc::mem_fun(*this, &InputCapture::mouse_move));

    mouse_input->signal_enter().connect([this](double x, double y) {
        mouse_inside = true;
        mouse_move(x, y);
    });
    mouse_input->signal_leave().connect([this]() { mouse_inside = false; });
    frame.add_controller(mouse_input);

    drag_input = Gtk::GestureDrag::create();
    frame.add_controller(drag_input);
    drag_input->signal_drag_begin().connect(
                sigc::mem_fun(*this, &InputCapture::drag_beg));
    drag_input->signal_drag_update().connect(
                sigc::mem_fun(*this, &InputCapture::drag));

    scroll_input = Gtk::EventControllerScroll::create();
    scroll_input->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL);
    frame.add_controller(scroll_input);
    scroll_input->signal_scroll().connect(
                sigc::mem_fun(*this, &InputCapture::scroll), true);

    scale    = 500 / 4;
    top_left = {-2, -2};

    frame.signal_resize().connect(
                [this](int w, int h) { on_resize(w, h); });
}
