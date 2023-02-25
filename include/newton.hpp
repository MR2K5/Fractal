#pragma once

#include "config.hpp"
#include "fractal.hpp"
#include "input.hpp"
#include "math_tools.hpp"
#include "threadpool.hpp"

#include <cstdint>

class NewtonFractal: public FractalBase {
    Gtk::DrawingArea dw;
    Gtk::Box options;
    InputCapture movement;
    Gtk::SpinButton max_iters;
    Gtk::CheckButton show_path;
    Gtk::CheckButton draw_axis;
    Pango::FontDescription font;
    ThreadPool tpool;

    Gtk::Button input_polynomial;
    Gtk::Dialog polynomial_input_dialog;
    Gtk::Scale dialog_degree;
    constexpr static int max_degree = 10;
    std::array<Gtk::Entry, max_degree + 1> dialog_text;
    std::shared_ptr<Gtk::Box> dialog_text_box;
    Gtk::Frame dialog_text_box_frame;

    math::Polynomial polynomial;
    math::Polynomial derivative;
    std::vector<math::complex> roots;
    void change_polynomial(math::Polynomial nw);
    void change_root();

    math::complex* active_root = nullptr;

    static const std::vector<RGB> root_colors;

    struct point {
        int iterations;
        int root;
    };

    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    void on_resize(int w, int h);

    void on_input_polynomial_pressed();
    void on_dialog_ok_pressed();
    void on_dialog_response(int response_id);

    std::vector<point> simple_alg(int w, int h);
    void render_color(std::vector<point> const& pts);
    std::vector<vec2> generate_path(math::complex const& z);

    bool is_near(std::complex<double> const& a, std::complex<double> const& b) const;

    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h);

    void on_mouse_click(InputCapture::MOUSE_CLICK);
    void on_mouse_moved(double x, double y);

public:
    NewtonFractal();

    Gtk::Widget& get_options() override { return options; }
    Gtk::DrawingArea& draw_area() override { return dw; }
};
