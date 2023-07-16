#include <function.hpp>

#include <cassert>

void Function::on_function_changed() {
    dw.queue_draw();

    func_options.unset_child();
    auto* f = functions[choose_function.get_active_row_number()].get();
    func_options.set_child(f->function_specific_options());
    f->signal_redraw.connect([this] { dw.queue_draw(); });
}

std::vector<vec2> Function::evaluate_function(int w, [[maybe_unused]] int h) {
    function_impl const* const active =
        functions[choose_function.get_active_row_number()].get();

    const vec2 tl      = movement.get_top_left();
    const vec2 br      = movement.get_bottom_right();
    double const xstep = (br.x() - tl.x()) / w;

    std::vector<vec2> points;
    points.reserve(w);
    for (int i = 0; i < w * 10; ++i) {
        double const x = tl.x() + xstep * i / 10;
        points.push_back(movement.world_to_screen({x, -active->call(x)}));
    }

    return points;
}

void Function::on_draw(Glib::RefPtr<Cairo::Context> const& cr, int w, int h) {
    auto points = evaluate_function(w, h);

    cr->set_source_rgb(0, 0, 0);
    cr->rectangle(0, 0, w, h);
    cr->fill();

    cr->set_source_rgb(255, 255, 255);
    cr->set_line_width(1);
    cr->set_line_cap(Cairo::Context::LineCap::BUTT);
    cr->set_line_join(Cairo::Context::LineJoin::MITER);
    cr->move_to(points[0].x(), points[0].y());

    for (auto& p : points) { cr->line_to(p.x(), p.y()); }
    cr->stroke();

    draw_coordinate_axes(cr, movement);
}

namespace {
struct x2 final: Function::FunctionBase {
    Gtk::CheckButton button;
    Gtk::Widget& function_specific_options() override { return button; }

    x2() {
        name = "x^2";
        button.set_label("Testbutton");
        button.signal_toggled().connect(signal_redraw);
    }

    double call(double x) const noexcept override {
        return x * x * (button.get_active() ? 2 : 1);
    }
};

struct Weierstrass final: Function::FunctionBase {
    Gtk::Box options;
    Gtk::SpinButton iters;
    Gtk::Frame iters_frame, A_frame, B_frame;
    Gtk::SpinButton A, B;

    Gtk::Widget& function_specific_options() override { return options; }

    Weierstrass() {
        name = "Weierstrass";
        iters = get_iters_spinbutton();
        iters.set_value(10);
        iters.signal_value_changed().connect(signal_redraw);

        options.set_orientation(Gtk::Orientation::VERTICAL);
        options.append(iters_frame);
        options.append(A_frame);
            options.append(B_frame);

        B = get_iters_spinbutton();
        B.set_value(7);
        B.set_increments(0.001, 0);
        B.set_range(0, 10000);
        B.set_digits(3);
        B.signal_value_changed().connect(signal_redraw);

        A = get_iters_spinbutton();
        A.set_digits(3);
        A.set_increments(0.001, 0);
        A.set_range(0, 1);
        A.set_value(0.25);
        A.signal_value_changed().connect(signal_redraw);

        iters_frame.set_label("Iters");
        iters_frame.set_child(iters);
        A_frame.set_label("A");
        A_frame.set_child(A);
        B_frame.set_label("B");
        B_frame.set_child(B);
    }
    double call(double x) const noexcept override {
        const int mx = iters.get_value_as_int();
        const double a = A.get_value();
        const int b = B.get_value();
        double sum = 0.0;

        for (int i = 0; i < mx; ++i) {
            sum += std::pow(a, i) * std::cos(std::pow(b, i) * std::numbers::pi * x);
        }
        return sum;
    }
};

}  // namespace

Function::Function(): movement(dw) {
    dw.signal_resize().connect([this](int w, int h) {
        pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
        dw.queue_draw();
    });
    dw.set_draw_func(sigc::mem_fun(*this, &Function::on_draw));
    dw.set_hexpand();
    dw.set_vexpand();
    dw.set_content_height(500);
    dw.set_content_width(500);

    options.set_orientation(Gtk::Orientation::VERTICAL);
    options.append(choose_function);
    options.append(func_options);

    func_options.set_label("Funciton-specific options");

    add_function(std::make_unique<x2>());
    add_function(std::make_unique<Weierstrass>());
    choose_function.signal_changed().connect([this] { on_function_changed(); });
    choose_function.set_active(0);

    movement.signal_changed().connect([this] { dw.queue_draw(); });
}

void Function::add_function(std::unique_ptr<function_impl>&& func) {
    choose_function.append(func->name);
    functions.push_back(std::move(func));
}

Gtk::SpinButton get_iters_spinbutton() {
    Gtk::SpinButton iters;
    iters.set_numeric();
    iters.set_range(1, std::numeric_limits<int>::max());
    iters.set_snap_to_ticks();
    iters.set_increments(1, 0);
    iters.set_value(10);
    return iters;
}
