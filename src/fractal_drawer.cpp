#include <fractal_drawer.hpp>

#include <cstring>
#include <iostream>
#include <numeric>

//void EscapeTimeDrawer::update_coordinates() {
//    vec2 tl = screen_to_world(top_left);
//    vec2 sz = screen_to_world({this->get_width(), get_height()}) - tl;
//    bound->set_view_area(tl.x(), tl.y(), sz.x(), sz.y());
//}

//void EscapeTimeDrawer::on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w,
//                               int h) {
//    update_coordinates();
//    escape_times = bound->generate_escape_times(w, h);

//    auto pb    = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
//    auto* data = pb->get_pixels();

//    for (int i = 0; i < escape_times.size(); ++i) {
//        unsigned char color = escape_times[i] / double(bound->get_max_iters()) * 255;
//        std::memset(data + i * 3, color, 3);
//    }

//    Gdk::Cairo::set_source_pixbuf(cr, pb, 0, 0);
//    cr->rectangle(0, 0, w, h);
//    cr->fill();
//}

//bool MovableDrawer::scroll(double dx, double dy) {
//    if (dy == 0) return false;
//    vec2 before = screen_to_world(mouse_pos);
//    if (dy < 0) {
//        scale *= 1 + (dy / -100);
//    } else if (dy > 0) {
//        scale /= 1 + (dy / 100);
//    }
//    vec2 after = screen_to_world(mouse_pos);
//    top_left   += (before - after);
//    queue_draw();
//    return true;
//}

//MovableDrawer::MovableDrawer() {
//    set_focus_on_click();
//    set_can_focus();
//    set_focusable();
//    set_sensitive();

//    mouse_input = Gtk::EventControllerMotion::create();
//    mouse_input->signal_motion().connect(
//        sigc::mem_fun(*this, &MovableDrawer::mouse_move));
//    add_controller(mouse_input);

//    drag_input = Gtk::GestureDrag::create();
//    add_controller(drag_input);
//    drag_input->signal_drag_begin().connect(
//        sigc::mem_fun(*this, &MovableDrawer::drag_beg));
//    drag_input->signal_drag_update().connect(
//        sigc::mem_fun(*this, &MovableDrawer::drag));

//    scroll_input = Gtk::EventControllerScroll::create();
//    scroll_input->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL);
//    add_controller(scroll_input);
//    scroll_input->signal_scroll().connect(
//        sigc::mem_fun(*this, &MovableDrawer::scroll), true);

//    scale    = 500 / 4;
//    top_left = world_to_screen({-2, -2});

//    signal_resize().connect([this](int w, int h) { on_resize(w, h); });
//}

//Drawer::Drawer() {
//    //        set_draw_func(sigc::mem_fun(*this, &Drawer::on_draw));
//    set_draw_func([this](auto const&... as) { on_draw(as...); });
//    set_can_focus();
//    set_focusable();
//    set_focus_on_click();
//    set_sensitive();

//    set_hexpand();
//    set_vexpand();
//}

//using RGB = Eigen::Vector3d;

//class Palette {
//    const std::vector<std::pair<double, RGB>> palette = {
//        {0.,     {0, 7, 100}    },
//        {0.16,   {32, 107, 203} },
//        {0.42,   {237, 255, 255}},
//        {0.6425, {255, 170, 0}  },
//        {0.8575, {0, 2, 0}      },
//        {1.,     {0, 7, 100}    }
//    };

//    RGB lerp(RGB a, RGB b, double x) { return a + x * (b - a); }

//public:
//    RGB operator[](double hue) {
////        assert(0 <= hue && hue <= 1);

//        int a = 0, b;
//        for (size_t i = 0; i < palette.size(); ++i) {
//            if (palette[i].first < hue) { a = i;  }
//        }
//        b = a + 1;

//        auto pa = palette[a];
//        auto pb = palette[b];
//        double x = (hue - pa.first) / std::abs(pb.first - pa.first);
////        assert(0 <= x && x <= 1);
//        return lerp(pa.second, pb.second, x);
//    }
//};

//void HistogramColoring::on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w,
//                                int h) {
//    update_coordinates();
//    escape_times = bound->generate_escape_times(w, h);
//    const int size = escape_times.size();

//    std::vector<int> num_iters_per_px;
//    num_iters_per_px.resize(bound->get_max_iters() + 1);
//    for (int i: escape_times) {
//        ++num_iters_per_px[i];
//    }
//    int total = 0;
//    for (int i: num_iters_per_px)
//        total += i;
//    total -= num_iters_per_px.back();

//    std::vector<double> hue(size);
//    for (int xy = 0; xy < size; ++xy) {
//        int iters = escape_times[xy];
//        for (int i = 0; i <= iters; ++i) {
//            hue[xy] += num_iters_per_px[i] / double(total);
//        }
//    }

//    auto pb    = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, w, h);
//    auto* data = pb->get_pixels();
//    Palette pl;

//    for (int i = 0; i < size; ++i) {
//        RGB val         = pl[hue[i]];
//        data[i * 3]     = val[0];
//        data[i * 3 + 1] = val[1];
//        data[i * 3 + 2] = val[2];
//    }

//    Gdk::Cairo::set_source_pixbuf(cr, pb);
//    cr->rectangle(0, 0, w, h);
//    cr->fill();

//    return;
//}
