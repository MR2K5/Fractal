#include <cassert>
#include <iostream>

#include <mandel.hpp>

#include <gtkmm-4.0/gtkmm.h>

class Viewer: public Gtk::ApplicationWindow {

public:
    Viewer() {
        mainbox.set_margin(5);
        set_child(mainbox);

        mainbox.append(options_area);
        mainbox.append(draw_area);

        options_area.set_orientation(Gtk::Orientation::VERTICAL);
        options_area.append(fractal_ops);
        options_area.append(drawer_ops);


        draw_area.set_focus_on_click();
        draw_area.set_can_focus();
        draw_area.set_focusable();
        draw_area.set_sensitive();

        draw_area.set_child(md.draw_area());
        drawer_ops.set_child(md.get_options());
    }

    Gtk::Box mainbox;
    Gtk::Frame draw_area;
    Gtk::Box options_area;

    Gtk::Frame fractal_ops, drawer_ops;

    Mandelbrot2 md;

};

int main(int argc, char** argv) {
    auto App = Gtk::Application::create("org.fractal.mr");
    return App->make_window_and_run<Viewer>(argc, argv);
}
