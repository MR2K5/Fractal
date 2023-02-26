#include <mandel.hpp>

#include <cassert>
#include <immintrin.h>

namespace {

constexpr size_t avx512_align = 64;
constexpr size_t avx512_size  = 64;

int iters_for(double cx, double cy, int mx) {
    double x = 0, y = 0, x2 = 0, y2 = 0;
    int iters = 0;
    for (; iters < mx; ++iters) {
        if (x2 + y2 > 4) break;
        y  = std::fma(x + x, y, cy);
        x  = x2 - y2 + cx;
        x2 = x * x;
        y2 = y * y;
    }
    return iters;
}

void avx2_render_line(int* const __restrict pline, double const x1,
                      double const x2, double const y1, int const linew,
                      int const maxiters) {
    double const stepsize = (x2 - x1) / linew;

    const __m256d cy        = _mm256_set1_pd(y1);
    const __m256d x1_       = _mm256_set1_pd(x1);
    const __m256d stepsize_ = _mm256_set1_pd(stepsize);
    const __m256i one       = _mm256_set1_epi64x(1);
    const __m256d escape    = _mm256_set1_pd(4.0);
    const __m256i all_ones  = _mm256_cmpeq_epi64(one, one);
    __m256d cx;
    auto const calc_cx = [x1_, stepsize_](int i) {
        __m128i i_ = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
        return _mm256_fmadd_pd(stepsize_, _mm256_cvtepi32_pd(i_), x1_);
    };
    int i = 0;
    for (; i < linew - 4; i += 4) {
        cx                 = calc_cx(i);
        __m256d x          = _mm256_setzero_pd();
        __m256d y          = _mm256_setzero_pd();
        __m256d x2         = _mm256_setzero_pd();
        __m256d y2         = _mm256_setzero_pd();
        __m256i iters      = _mm256_setzero_si256();
        __m256d iters_mask = _mm256_setzero_pd();

        for (int iter = 0; iter < maxiters; ++iter) {
            auto nw_mask =
                _mm256_cmp_pd(_mm256_add_pd(x2, y2), escape, _CMP_GT_OQ);
            iters_mask = _mm256_or_pd(iters_mask, nw_mask);
            iters =
                _mm256_add_epi64(iters, _mm256_andnot_si256(iters_mask, one));

            if (_mm256_testc_si256(iters_mask, all_ones) == 1) break;

            y  = _mm256_fmadd_pd(_mm256_add_pd(x, x), y, cy);
            x  = _mm256_add_pd(_mm256_sub_pd(x2, y2), cx);
            x2 = _mm256_mul_pd(x, x);
            y2 = _mm256_mul_pd(y, y);
        }

        alignas(32) int64_t val[4];
        _mm256_store_si256((__m256i*)val, iters);
        pline[i]     = val[0];
        pline[i + 1] = val[1];
        pline[i + 2] = val[2];
        pline[i + 3] = val[3];
    }

    for (; i < linew; ++i) {
        pline[i] = iters_for(x1 + stepsize * i, y1, maxiters);
    }
}

void avx512_render_line(int* const __restrict pline, double const x1,
                        double const x2, double const y1, int const linew,
                        int const maxiters) {
#ifndef HAS_AVX512
    avx2_render_line(pline, x1, x2, y1, linew, maxiters);
#else
    double const stepsize = (x2 - x1) / linew;

    const __m512d cy        = _mm512_set1_pd(y1);
    const __m512d x1_       = _mm512_set1_pd(x1);
    const __m512d stepsize_ = _mm512_set1_pd(stepsize);
    const __m512i one       = _mm512_set1_epi64(1);
    const __m512d escape    = _mm512_set1_pd(4.0);
    __m512d cx;
    auto const calc_cx = [x1_, stepsize_](int i) {
        __m512i i_ = _mm512_setr_epi64(i, i + 1, i + 2, i + 3, i + 4, i + 5,
                                       i + 6, i + 7);
        return _mm512_fmadd_pd(stepsize_, _mm512_cvtepi64_pd(i_), x1_);
    };
    int i = 0;
    for (; i < linew - 8; i += 8) {
        cx                  = calc_cx(i);
        __m512d x           = _mm512_setzero_pd();
        __m512d y           = _mm512_setzero_pd();
        __m512d x2          = _mm512_setzero_pd();
        __m512d y2          = _mm512_setzero_pd();
        __m512i iters       = _mm512_setzero_si512();
        __mmask8 iters_mask = _cvtu32_mask8(0xFF);

        for (int iter = 0; iter < maxiters; ++iter) {
            iters_mask = _kand_mask8(
                _mm512_cmp_pd_mask(_mm512_add_pd(x2, y2), escape, _CMP_LT_OQ),
                iters_mask);
            iters = _mm512_mask_add_epi64(iters, iters_mask, iters, one);

            if (_cvtmask8_u32(iters_mask) == 0) break;

            y  = _mm512_fmadd_pd(_mm512_add_pd(x, x), y, cy);
            x  = _mm512_add_pd(_mm512_sub_pd(x2, y2), cx);
            x2 = _mm512_mul_pd(x, x);
            y2 = _mm512_mul_pd(y, y);
        }

        __m256i cvt = _mm512_cvtepi64_epi32(iters);
        _mm256_storeu_epi32(pline + i, cvt);
    }

    for (; i < linew; ++i) {
        pline[i] = iters_for(x1 + stepsize * i, y1, maxiters);
    }
#endif
}

}  // namespace


std::vector<int> Mandelbrot::calculate_iters(int w, int h) {
    std::vector<int> iterations(w * h);
    vec2 tl = movement.get_top_left();
    vec2 br = movement.get_bottom_right();
    vec2 sz = br - tl;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int iters = 0;
            std::complex c{
                tl.x() + sz.x() * (double(x) / w),
                tl.y() + sz.y() * (double(y) / h),
            };
            std::complex z{0.0, 0.0};
            for (; iters < max_iters.get_value_as_int(); ++iters) {
                if (std::norm(z) > 4.0) break;
                z = z * z + c;
            }
            iterations[y * w + x] = iters;
        }
    }
    return iterations;
}

Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::default_alg(int w, int h) {
    auto pb    = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
    auto* data = pb->get_pixels();

    auto f          = tpool.queue(&Mandelbrot::calculate_iters, this, w, h);
    auto iterations = std::move(f.get());

    double mx = max_iters.get_value();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx           = y * w + x;
            RGB vl            = get_color_for_hue(iterations[idx] / mx);
            data[3 * idx]     = vl[0];
            data[3 * idx + 1] = vl[1];
            data[3 * idx + 2] = vl[2];
        }
    }

    return pb;
}

Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::default_alg_optimized(int const w,
                                                            int const h) {
    auto* data    = pixbuf->get_pixels();
    const vec2 tl = movement.get_top_left();
    const vec2 br = movement.get_bottom_right();
    const vec2 sz = br - tl;

    int const mx = max_iters.get_value_as_int();
    //    int const size = w * h;

    // Divide into 8 x 8 areas, render multithreaded

    auto calc = [=, this, sw_ = sz.x() / w,
                 sh_ = sz.y() / h](int sx1, int sy1, int sx2, int sy2) {
        for (int j = sy1; j < sy2; ++j) {
            const double cy = tl.y() + sh_ * j;
            for (int i = sx1; i < sx2; ++i) {
                const double cx = tl.x() + sw_ * i;

                double x = 0, y = 0;
                double x2 = 0, y2 = 0;
                int iters = 0;
                for (; iters < mx; ++iters) {
                    if (x2 + y2 > 4.0) break;
                    y  = std::fma(x + x, y, cy);
                    x  = x2 - y2 + cx;
                    x2 = x * x;
                    y2 = y * y;
                }

                RGB c         = get_color_for_hue(iters / double(mx));
                const int idx = 3 * (i + j * w);
                data[idx]     = c[0];
                data[idx + 1] = c[1];
                data[idx + 2] = c[2];
            }
        }
    };

    int ar_w = w / 8;
    int ar_h = h / 8;
    std::vector<std::future<void>> fts;
    fts.reserve(8 * 8);

    for (int j = 0; j < h; j += ar_h) {
        int jend = std::min(j + ar_h, h);
        for (int i = 0; i < w; i += ar_w) {
            int iend = std::min(i + ar_w, w);
            //                calc(i, j, iend, jend);
            fts.push_back(tpool.queue(calc, i, j, iend, jend));
        }
    }

    for (auto& f : fts) f.get();

    return pixbuf;
}

Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::avx512_alg(int w, int h) {
    auto escape_times = simd_escape_times(w, h, &avx512_render_line);

    guint8* data = pixbuf->get_pixels();

    double mx = max_iters.get_value();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx           = y * w + x;
            RGB vl            = get_color_for_hue(escape_times[idx] / mx);
            data[3 * idx]     = vl[0];
            data[3 * idx + 1] = vl[1];
            data[3 * idx + 2] = vl[2];
        }
    }
    return pixbuf;
}
Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::avx2_alg(int w, int h) {
    auto escape_times = simd_escape_times(w, h, &avx2_render_line);

    guint8* data = pixbuf->get_pixels();

    double mx = max_iters.get_value();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx           = y * w + x;
            RGB vl            = get_color_for_hue(escape_times[idx] / mx);
            data[3 * idx]     = vl[0];
            data[3 * idx + 1] = vl[1];
            data[3 * idx + 2] = vl[2];
        }
    }
    return pixbuf;
}

Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::histogram_alg(int w, int h) {
    auto iterations = simd_escape_times(w, h, &avx512_render_line);
    int const size  = iterations.size();
    assert(size == w * h);
    int const mx = max_iters.get_value_as_int();

    auto pb    = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
    auto* data = pb->get_pixels();

    int total_ = 0;
    std::vector<int> iter_counts(mx + 1);
    for (int i : iterations) {
        ++iter_counts[i];
        ++total_;
    }
    double const total = total_;
    std::vector<long> iter_counts_cumulative(mx + 1);
    iter_counts_cumulative[0] = iter_counts[0];
    for (int i = 1; i < mx + 1; ++i) {
        iter_counts_cumulative[i] =
            iter_counts_cumulative[i - 1] + iter_counts[i];
    }

    std::vector<double> hue(size);
    for (int i = 0; i < size; ++i) {
        int iters = iterations[i];
        hue[i]    = iter_counts_cumulative[iters] / total;
    }

    for (int idx = 0; idx < size; ++idx) {
        RGB vl            = get_color_for_hue(hue[idx]);
        data[3 * idx]     = vl[0];
        data[3 * idx + 1] = vl[1];
        data[3 * idx + 2] = vl[2];
    }

    return pb;
}

Glib::RefPtr<Gdk::Pixbuf> Mandelbrot::black_and_white(int w, int h) {
    auto iters = simd_escape_times(w, h, &avx512_render_line);

    guint8 color1 = 0;
    guint8 color2 = 0;
    if (max_iters.get_value_as_int() % 2 == 1)
        color1 = 0xff;
    else
        color2 = 0xff;

    int const sz = w * h;
    guint8* data = pixbuf->get_pixels();
    for (int i = 0; i < sz; ++i) {
        guint8 c        = (iters[i] & 1) == 0 ? color1 : color2;
        data[3 * i]     = c;
        data[3 * i + 1] = c;
        data[3 * i + 2] = c;
    }

    return pixbuf;
}

void Mandelbrot::on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w,
                         int h) {
    namespace chrono = std::chrono;
    auto beg         = chrono::steady_clock::now();

    Glib::RefPtr<Gdk::Pixbuf> pb;

    switch (algorithm_select.get_active_row_number()) {
    case 0: pb = default_alg(w, h); break;
    case 1: pb = histogram_alg(w, h); break;
    case 2: pb = default_alg_optimized(w, h); break;
    case 3: pb = avx2_alg(w, h); break;
    case 4: pb = avx512_alg(w, h); break;
    case 5: pb = black_and_white(w, h); break;
    }

    auto end = chrono::steady_clock::now();
    auto et =
        chrono::duration_cast<chrono::duration<double, std::milli>>(end - beg);

    render_pixbuf(cr, w, h, pb);

    const Glib::ustring str =
        "Render time: " + std::to_string(et.count()) + " ms";
    auto layout = dw.create_pango_layout(str);
    layout->set_font_description(font);
    int tw, th;

    cr->set_source_rgb(1, 1, 1);
    cr->move_to(10, 10);
    layout->show_in_cairo_context(cr);

    if (show_path.get_active() && movement.mouse_is_inside()) {
        auto const points = generate_path(movement.get_mouse_pos());
        cr->begin_new_sub_path();
        cr->set_source_rgb(255, 255, 255);
        cr->set_line_width(2);
        cr->set_line_join(Cairo::Context::LineJoin::MITER);
        for (auto& point : points) { cr->line_to(point.x(), point.y()); }
        cr->stroke();
    }
}

std::vector<vec2> Mandelbrot::generate_path(vec2 const& screenpos) {
    const vec2 c_ = movement.screen_to_world(screenpos);
    const std::complex c{c_.x(), c_.y()};
    std::complex z{0.0, 0.0};
    constexpr int iters = 1000;

    std::vector<vec2> res;
    res.reserve(iters);

    for (int i = 0; i < iters; ++i) {
        z = z * z + c;
        res.push_back(movement.world_to_screen({z.real(), z.imag()}));
        if (!movement.is_inside(res.back()) && std::norm(z) > 4.0) break;
    }
    return res;
}


std::vector<int> Mandelbrot::simd_escape_times(int w, int h, simd_func* const alg) {
    const vec2 tl      = movement.get_top_left();
    const vec2 br      = movement.get_bottom_right();
    const vec2 sz      = br - tl;
    double const ystep = sz.y() / h;
    int const mx       = max_iters.get_value_as_int();

    std::vector<int> res(w * h);

    auto exec_lines = [&, this](int sy1, int sy2) {
        for (int line = sy1; line < sy2; ++line) {
            alg(res.data() + line * w, tl.x(), br.x(),
                               tl.y() + ystep * line, w, mx);
        }
    };

    int y_line_step = h / 64;
    int i           = 0;
    std::vector<std::future<void>> fts;
    fts.reserve(65);
    for (; i < h; i += y_line_step) {
        fts.push_back(tpool.queue(exec_lines, i, std::min(i + y_line_step, h)));
    }
    for (auto& f : fts) f.get();

    return res;
}

Mandelbrot::Mandelbrot(): movement(dw) {
    dw.set_draw_func(sigc::mem_fun(*this, &Mandelbrot::on_draw));
    dw.set_content_width(500);
    dw.set_content_height(500);
    dw.set_hexpand();
    dw.set_vexpand();
    dw.signal_resize().connect(sigc::mem_fun(*this, &Mandelbrot::on_resize));

    auto queue_update = [this] { dw.queue_draw(); };
    movement.signal_changed().connect(queue_update);
    movement.signal_mouse_moved().connect([this](double, double) {
        if (show_path.get_active()) dw.queue_draw();
    });

    options.set_orientation(Gtk::Orientation::VERTICAL);
    options.append(max_iters);
    options.append(algorithm_select);
    options.append(show_path);

    max_iters.set_numeric();
    max_iters.set_range(1, std::numeric_limits<int>::max());
    max_iters.set_increments(1, 0);
    max_iters.set_snap_to_ticks();
    max_iters.set_value(30);
    max_iters.signal_value_changed().connect(queue_update);

    algorithm_select.append("Default");
    algorithm_select.append("Histogram coloring");
    algorithm_select.append("Optimized");
    algorithm_select.append("AVX");
    algorithm_select.append("AVX512");
    algorithm_select.append("Black and white");
    algorithm_select.set_active(3);
    algorithm_select.signal_changed().connect(queue_update);

    show_path.set_label("Show path");
    show_path.set_active(false);
    show_path.signal_toggled().connect(queue_update);

    font.set_family("Monospace");
    font.set_absolute_size(10 * Pango::SCALE);
    font.set_weight(Pango::Weight::MEDIUM);
}

Gtk::DrawingArea& Mandelbrot::draw_area() { return dw; }

Gtk::Widget& Mandelbrot::get_options() { return options; }
