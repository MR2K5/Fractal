#pragma once

#include <fractal.hpp>
#include <config.hpp>
#include <input.hpp>

Gtk::SpinButton get_iters_spinbutton();

class Function: public FractalBase {
    struct function_impl {
        Glib::ustring name;
        sigc::signal<void()> signal_redraw;

        virtual Gtk::Widget& function_specific_options() = 0;
        virtual double call(double x) const noexcept = 0;
        virtual ~function_impl() = default;
    };

    Gtk::DrawingArea dw;
    InputCapture movement;
    Gtk::Box options;
    Gtk::Frame func_options;
    Gtk::ComboBoxText choose_function;

    std::vector<std::unique_ptr<function_impl>> functions;

    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    void on_function_changed();
    std::vector<vec2> evaluate_function(int w, int h);

    void on_draw(Glib::RefPtr<Cairo::Context> const& cr, int w, int h);

public:
    using FunctionBase = function_impl;

    Function();

    Gtk::DrawingArea& draw_area() override { return dw; }
    Gtk::Widget& get_options() override { return options; }

    void add_function(std::unique_ptr<function_impl>&& func);
};
