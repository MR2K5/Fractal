#pragma once

#include <config.hpp>
#include <fractal.hpp>
#include <input.hpp>
#include <math_tools.hpp>

#include <cstdint>

class NewtonFractal: public FractalBase {
    Gtk::DrawingArea dw;
    Gtk::Box options;
    InputCapture movement;

    math::Polynomial polynomial;
    math::Polynomial derivative;
    std::vector<math::complex> roots;
    void change_polynomial(math::Polynomial nw);

    static const std::vector<RGB> root_colors;
    int max_iters = 100;

    struct point {
        int iterations;
        int root;
    };

    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    void on_resize(int w, int h) {
        pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
    }

    std::vector<point> test_z3_1(int w, int h);
    void render_color(std::vector<point> const& pts);

    bool is_near(std::complex<double> const& a, std::complex<double> const& b) const;

    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h);

public:
    NewtonFractal();

    Gtk::Widget& get_options() override { return options; }
    Gtk::DrawingArea& draw_area() override { return dw; }
};
