#include <cassert>
#include <iostream>

#include <mandel.hpp>
#include <newton.hpp>

#include <gtkmm-4.0/gtkmm.h>

static Gtk::Window* mainwindow = nullptr;

class Viewer: public Gtk::ApplicationWindow {

public:
    Viewer() {
        mainwindow = this;
        mainbox.set_margin(5);
        set_child(mainbox);

        mainbox.append(options_area);
        mainbox.append(draw_area);

        options_area.set_orientation(Gtk::Orientation::VERTICAL);
        options_area.append(select_fractal);
        options_area.append(fractal_ops);
        options_area.append(drawer_ops);

        select_fractal.append("Mandelbrot");
        select_fractal.append("Newton");
        select_fractal.set_active(0);
        select_fractal.signal_changed().connect([this] { change_fractal(); });

        draw_area.set_focus_on_click();
        draw_area.set_can_focus();
        draw_area.set_focusable();
        draw_area.set_sensitive();

        select_fractal.set_active(1);
    }

    std::shared_ptr<FractalBase> create_fractal(int id) {
        switch (id) {
        case 0: return std::make_shared<Mandelbrot>();
        case 1: return std::make_shared<NewtonFractal>();
        default: return nullptr;
        }
    }

    void change_fractal() {
        auto nw = create_fractal(select_fractal.get_active_row_number());
        if (nw == nullptr) return;

        fractal_ops.unset_child();
        draw_area.unset_child();
        fractal_ops.set_child(nw->get_options());
        draw_area.set_child(nw->draw_area());
        fractal = nw;
    }

    Gtk::Box mainbox;
    Gtk::Frame draw_area;
    Gtk::Box options_area;

    Gtk::Frame fractal_ops, drawer_ops;
    Gtk::ComboBoxText select_fractal;

    std::shared_ptr<FractalBase> fractal;
};

Gtk::Window& get_main_window() {
    return *mainwindow;
}

int main(int argc, char** argv) {
    auto App = Gtk::Application::create("org.fractal.mr");
    return App->make_window_and_run<Viewer>(argc, argv);
}
