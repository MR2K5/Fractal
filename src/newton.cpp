#include <newton.hpp>

const std::vector<RGB> NewtonFractal::root_colors = {
    {255, 0,   0  },
    {0,   255, 0  },
    {0,   0,   255}
};

void NewtonFractal::change_polynomial(math::Polynomial nw) {
    polynomial = nw;
    derivative = math::derivative(polynomial);
    roots      = math::find_roots(polynomial);
}

std::vector<NewtonFractal::point> NewtonFractal::test_z3_1(int w, int h) {
    std::vector<point> data(w * h);

    const vec2 tl = movement.get_top_left();
    const vec2 br = movement.get_bottom_right();
    const vec2 sz = br - tl;

    for (int j = 0; j < h; ++j) {
        double y = tl.y() + sz.y() / h * j;
        for (int i = 0; i < w; ++i) {
            double x = tl.x() + sz.x() / w * i;
            std::complex zn(x, y);

            int iter = 0;
            for (; iter < max_iters; ++iter) {
                zn -= polynomial(zn) / derivative(zn);
                for (int r = 0; r < roots.size(); ++r) {
                    if (is_near(roots[r], zn)) {
                        data[j * w + i] = point{ .iterations = iter, .root = r };
                        goto end;
                    }
                }
            }
            data[j * w + i] = point{.iterations = iter, .root = -1};

        end:
            void();
        }
    }

    return data;
}

void NewtonFractal::render_color(std::vector<point> const& pts) {
    guint8* data = pixbuf->get_pixels();

    for (int i = 0; i < pts.size(); ++i) {
        int r           = pts[i].root;
        RGB c           = (r == -1) ? RGB(0, 0, 0) : root_colors[r];
        data[3 * i]     = c[0];
        data[3 * i + 1] = c[1];
        data[3 * i + 2] = c[2];
    }
}

bool NewtonFractal::is_near(std::complex<double> const& a,
                            std::complex<double> const& b) const {
    constexpr double tol = 0.00001;
    double const d       = std::abs(std::norm(a) - std::norm(b));
    return d < tol;
}

void NewtonFractal::on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w,
                            int h) {
    std::vector<point> points = test_z3_1(w, h);
    render_color(points);

    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->rectangle(0, 0, w, h);
    cr->fill();
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
}
