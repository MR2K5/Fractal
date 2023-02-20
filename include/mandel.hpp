#pragma once

#include <input.hpp>
#include <threadpool.hpp>
#include <config.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>


class FractalBase {
public:
    virtual Gtk::DrawingArea& draw_area() = 0;
    virtual Gtk::Widget& get_options()    = 0;
};

class Mandelbrot2: public FractalBase {
    Gtk::DrawingArea dw;
    InputCapture movement;

    Gtk::Box options;
    Gtk::SpinButton max_iters;
    Gtk::ComboBoxText algorithm_select;
    Gtk::CheckButton show_path;
    Pango::FontDescription font;

    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    ThreadPool tpool;

    void on_resize(int w, int h) {
        pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
    }

    std::vector<int> calculate_iters(int w, int h);

    Glib::RefPtr<Gdk::Pixbuf> default_alg(int w, int h);

    Glib::RefPtr<Gdk::Pixbuf> default_alg_optimized(int const w, int const h);

    Glib::RefPtr<Gdk::Pixbuf> avx512_alg(int w, int h);

    Glib::RefPtr<Gdk::Pixbuf> histogram_alg(int w, int h);

    void render_pixbuf(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h,
                       Glib::RefPtr<Gdk::Pixbuf> const& pb) {
        Gdk::Cairo::set_source_pixbuf(cr, pb);
        cr->rectangle(0, 0, w, h);
        cr->fill();
    }

    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h);

    std::vector<vec2> generate_path(vec2 const& screenpos);

    std::vector<int> simd_escape_times(int w, int h);

public:
    Mandelbrot2();

    Gtk::DrawingArea& draw_area() override;
    Gtk::Widget& get_options() override;
};
