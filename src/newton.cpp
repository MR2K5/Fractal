#include <newton.hpp>

#include <chrono>
#include <iostream>

const std::vector<RGB> NewtonFractal::root_colors = {
    {255, 0,   0  },
    {0,   255, 0  },
    {0,   0,   255},
    {255, 255, 0  },
    {255, 0,   255},
    {0,   255, 255},
    {128, 255, 0  },
    {255, 128, 0  },
    {0,   255, 128},
    {128, 0,   255}
};

void NewtonFractal::change_polynomial(math::Polynomial nw) {
    polynomial = nw;
    derivative = math::derivative(polynomial);
    roots      = math::find_roots(polynomial, 1e-10);
    if (polynomial.degree() > root_colors.size())
        throw std::logic_error("Polynomial degree too big, not enough colors "
                               "in NewtonFractal::root_colors");
    dw.queue_draw();
}

void NewtonFractal::change_root() {
    polynomial = math::Polynomial::from_roots(roots);
    derivative = math::derivative(polynomial);
    dw.queue_draw();
}

void NewtonFractal::on_resize(int w, int h) {
    pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
}

void NewtonFractal::on_input_polynomial_pressed() {
    dialog_degree.set_value(polynomial.degree());
    for (int i = 0; i <= polynomial.degree(); ++i) {
        std::stringstream ss;
        ss.imbue(std::locale::classic());
        math::complex c = polynomial[i];
        if (c.real() == 0 && c.imag() == 0) {
            ss << 0;
        } else if (c.imag() == 0) {
            ss << c.real();
        } else {
            ss << c;
        }
        dialog_text[i].set_text(ss.str());
    }
    polynomial_input_dialog.show();
}

void NewtonFractal::on_dialog_ok_pressed() {
    // Validate text
    math::Polynomial poly;
    for (int i = 0; i <= dialog_degree.get_value(); ++i) {
        math::complex coeff;
        std::stringstream ss(dialog_text[i].get_text());
        ss.imbue(std::locale::classic());
        ss >> coeff;
        if (ss.fail()) return;
        poly[i] = coeff;
    }

    change_polynomial(poly);
    polynomial_input_dialog.hide();
}

std::vector<NewtonFractal::point> NewtonFractal::simple_alg(int w, int h) {
    std::vector<point> data(w * h);

    const vec2 tl      = movement.get_top_left();
    const vec2 br      = movement.get_bottom_right();
    const vec2 sz      = br - tl;
    double const ystep = sz.y() / h;
    double const xstep = sz.x() / w;
    double const x1    = tl.x();

    auto run_line = [this, &xstep, &x1, &w](double y1, point* data) {
        for (int i = 0; i < w; ++i) {
            double x = x1 + xstep * i;
            std::complex zn(x, y1);

            int iter     = 0;
            const int mx = max_iters.get_value_as_int();
            for (; iter < mx; ++iter) {
                zn -= polynomial(zn) / derivative(zn);
                for (int r = 0; r < roots.size(); ++r) {
                    if (is_near(roots[r], zn)) {
                        data[i] = point{.iterations = iter, .root = r};
                        goto break_out_of_iter_loop;
                    }
                }
            }
            data[i] = point{.iterations = iter, .root = -1};

        break_out_of_iter_loop:;
        }
    };
    auto run_lines = [&](int first, int last) {
        for (; first != last; ++first) {
            const double y = tl.y() + ystep * first;
            run_line(y, data.data() + first * w);
        }
    };

    constexpr int threads_count = 32;
    int const dy                = h / threads_count;
    std::vector<std::future<void>> fts;
    fts.reserve(threads_count);
    for (int j = 0; j < h; j += dy) {
        fts.push_back(tpool.queue(run_lines, j, std::min(j + dy, h)));
    }
    for (auto& f : fts) f.get();

    return data;
}

void NewtonFractal::render_color(std::vector<point> const& pts) {
    guint8* data = pixbuf->get_pixels();

    for (int i = 0; i < pts.size(); ++i) {
        int r           = pts[i].root;
        double const mx = max_iters.get_value();
        double mult     = 0.2 + 0.8 * (mx - pts[i].iterations) / mx;
        RGB c           = (r == -1) ? RGB(0, 0, 0) : root_colors[r] * mult;
        data[3 * i]     = c[0];
        data[3 * i + 1] = c[1];
        data[3 * i + 2] = c[2];
    }
}

std::vector<vec2> NewtonFractal::generate_path(math::complex const& z_) {
    std::vector<vec2> path;
    int const mx = max_iters.get_value_as_int();
    path.reserve(mx);
    math::complex z = z_;
    vec2 sp_        = movement.world_to_screen({z.real(), z.imag()});
    path.push_back(sp_);

    for (int i = 0; i < mx; ++i) {
        z         -= polynomial(z) / derivative(z);
        vec2 spos = movement.world_to_screen({z.real(), z.imag()});
        path.push_back(spos);
        //        if (!movement.is_inside(spos)) break;
    }
    return path;
}

bool NewtonFractal::is_near(std::complex<double> const& a,
                            std::complex<double> const& b) const {
    constexpr double tol = 0.00001;
    double const d       = std::norm(a - b);
    return d < tol;
}

void NewtonFractal::on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w,
                            int h) {

    auto t1                   = std::chrono::steady_clock::now();
    std::vector<point> points = simple_alg(w, h);
    render_color(points);
    auto t2 = std::chrono::steady_clock::now();
    auto et =
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
            t2 - t1);

    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->rectangle(0, 0, w, h);
    cr->fill();

    for (auto& root : roots) {
        cr->set_source_rgb(255, 255, 255);
        if (&root == active_root) { cr->set_source_rgb(0, 0, 0); }
        vec2 spos = movement.world_to_screen({root.real(), root.imag()});
        cr->arc(spos.x(), spos.y(), 4, 0, 2 * std::numbers::pi);
        cr->fill();
    }

    if (show_path.get_active() && movement.mouse_is_inside()) {
        vec2 pos  = movement.screen_to_world(movement.get_mouse_pos());
        auto path = generate_path({pos.x(), pos.y()});

        cr->begin_new_sub_path();
        cr->set_source_rgb(255, 255, 255);
        cr->set_line_width(2);
        cr->set_line_join(Cairo::Context::LineJoin::MITER);
        for (auto& point : path) { cr->line_to(point.x(), point.y()); }
        cr->stroke();
    }

    const Glib::ustring str =
        "Render time: " + std::to_string(et.count()) + " ms";
    auto layout = dw.create_pango_layout(str);
    layout->set_font_description(font);

    if (draw_axis.get_active()) {
        vec2 wtl  = movement.get_top_left();
        vec2 wbr  = movement.get_bottom_right();
        vec2 xbeg = movement.world_to_screen({wtl.x(), 0});
        vec2 xend = movement.world_to_screen({wbr.x(), 0});
        vec2 ybeg = movement.world_to_screen({0, wtl.y()});
        vec2 yend = movement.world_to_screen({0, wbr.y()});

        cr->set_line_width(2);
        cr->set_source_rgb(255, 255, 255);
        cr->move_to(xbeg.x(), xbeg.y());
        cr->line_to(xend.x(), xend.y());
        cr->stroke();
        cr->move_to(ybeg.x(), ybeg.y());
        cr->line_to(yend.x(), yend.y());
        cr->stroke();
    }

    cr->set_source_rgb(0, 0, 0);
    cr->move_to(10, 10);
    layout->show_in_cairo_context(cr);
}

void NewtonFractal::on_mouse_click(InputCapture::MOUSE_CLICK c) {
    if (active_root) {
        active_root = nullptr;
        dw.queue_draw();
        return;
    }
    if (c == InputCapture::MOUSE_CLICK::RIGHT || static_cast<int>(c) == 3) {
        vec2 screen_click = movement.get_mouse_pos();
        for (auto& root : roots) {
            vec2 srt = movement.world_to_screen({root.real(), root.imag()});
            if ((screen_click - srt).norm() < 8) {
                active_root = &root;
                dw.queue_draw();
                break;
            }
        }
    }
}

void NewtonFractal::on_mouse_moved(double x, double y) {
    if (show_path.get_active()) dw.queue_draw();
    if (active_root != nullptr) {
        vec2 wp      = movement.screen_to_world({x, y});
        *active_root = {wp.x(), wp.y()};
        change_root();
    }
}

NewtonFractal::NewtonFractal(): movement(dw) {
    dw.signal_resize().connect([this](int w, int h) { on_resize(w, h); });
    dw.set_draw_func(sigc::mem_fun(*this, &NewtonFractal::on_draw));
    dw.set_content_height(500);
    dw.set_content_width(500);
    dw.set_hexpand();
    dw.set_vexpand();

    change_polynomial(
        math::Polynomial(std::to_array<math::complex>({-1, 0, 0, 1})));

    options.set_orientation(Gtk::Orientation::VERTICAL);
    options.append(max_iters);
    options.append(show_path);
    options.append(draw_axis);
    options.append(input_polynomial);

    max_iters.set_increments(1, 0);
    max_iters.set_snap_to_ticks();
    max_iters.set_range(1, std::numeric_limits<int>::max());
    max_iters.set_numeric();
    max_iters.set_value(30);
    max_iters.signal_value_changed().connect([this] { dw.queue_draw(); });

    show_path.set_active(false);
    show_path.signal_toggled().connect([this] { dw.queue_draw(); });
    show_path.set_label("Show path");

    draw_axis.set_active(false);
    draw_axis.set_label("Draw axes");
    draw_axis.signal_toggled().connect([this] { dw.queue_draw(); });

    input_polynomial.set_label("Input polynomial");
    input_polynomial.signal_clicked().connect(
        [this] { on_input_polynomial_pressed(); });

    polynomial_input_dialog.add_button("Ok!", Gtk::ResponseType::OK)
        ->signal_clicked()
        .connect([this] { on_dialog_ok_pressed(); });
    polynomial_input_dialog.add_button("Close", Gtk::ResponseType::CLOSE)
        ->signal_clicked()
        .connect([this] { polynomial_input_dialog.hide(); });
    polynomial_input_dialog.set_modal();
    polynomial_input_dialog.set_hide_on_close();
    polynomial_input_dialog.set_transient_for(get_main_window());
    polynomial_input_dialog.get_content_area()->append(dialog_degree);
    polynomial_input_dialog.get_content_area()->append(dialog_text_box_frame);

    dialog_degree.set_digits(0);
    dialog_degree.set_increments(1, 0);
    dialog_degree.set_range(1, max_degree);
    dialog_degree.set_slider_size_fixed();
    dialog_degree.set_draw_value();
    dialog_degree.signal_value_changed().connect([this] {
        dialog_text_box_frame.unset_child();
        dialog_text_box = std::make_shared<Gtk::Box>();
        dialog_text_box->set_orientation(Gtk::Orientation::VERTICAL);
        for (int i = 0; i <= dialog_degree.get_value(); ++i) {
            dialog_text_box->append(dialog_text[i]);
        }
        dialog_text_box_frame.set_child(*dialog_text_box);
    });
    dialog_degree.set_value(2);

    for (int i = 0; i <= max_degree; ++i) {
        dialog_text[i].set_placeholder_text("Degree " + std::to_string(i));
    }

    movement.signal_mouse_moved().connect(
        sigc::mem_fun(*this, &NewtonFractal::on_mouse_moved));
    movement.signal_changed().connect([this] { dw.queue_draw(); });
    movement.signal_mouse_clicked().connect(
        [this](auto x) { on_mouse_click(x); });

    font.set_family("Monospace");
    font.set_absolute_size(20 * Pango::SCALE);
    font.set_weight(Pango::Weight::MEDIUM);
}
