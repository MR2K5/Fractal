#pragma once

#include <gtkmm-4.0/gtkmm.h>

#include <Eigen/Core>

using vec2 = Eigen::Vector2d;

class EscapeTimeFractal;

class Drawer: public Gtk::DrawingArea {
public:
    Drawer();

    virtual Gtk::Box& get_drawer_options() {
        static Gtk::Box e;
        return e;
    }
    virtual void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h) = 0;
};

class MovableDrawer: public Drawer {
protected:
    void on_resize(int w, int h) {
        vec2 end = screen_to_world(top_left + size);
        vec2 nw = screen_to_world(top_left + vec2{w, h});
        vec2 ch = (nw - end) / scale;
        size = ch;
        queue_draw();
    }

    vec2 world_to_screen(vec2 const& world) const noexcept {
        return (world - top_left) * scale;
    }
    vec2 screen_to_world(vec2 const& screen) const noexcept {
        return screen / scale + top_left;
    }

    void mouse_move(double x, double y) {
        mouse_pos = {x, y};
    }

    void drag_beg(double x, double y) {
        last_pan  = {0, 0};
    }
    void drag(double x, double y) {
        top_left -= (vec2{x, y} - last_pan) / scale;
        last_pan = {x, y};
        queue_draw();
    }

    bool scroll(double dx, double dy);

public:
    MovableDrawer();

protected:
    vec2 top_left     = {-2, -2};
    vec2 size         = {4, 4};
    double scale      = 1;
    double const step = 0.1;

    Glib::RefPtr<Gtk::EventControllerMotion> mouse_input;
    Glib::RefPtr<Gtk::GestureDrag> drag_input;
    Glib::RefPtr<Gtk::EventControllerScroll> scroll_input;

    vec2 last_pan;
    vec2 mouse_pos;
};

class EmptyDrawer: public Drawer {
public:
    EmptyDrawer() {
        set_content_width(500);
        set_content_height(500);
    }
    static Glib::ustring display_name() { return "Empty"; }
    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h) {}
};

class EscapeTimeOptions: public Gtk::Box {
public:
};

class EscapeTimeDrawer: public MovableDrawer {
protected:
    EscapeTimeFractal* bound;
    EscapeTimeOptions opts;
    std::vector<int> escape_times;
    void update_coordinates();

public:
    EscapeTimeDrawer() {
        set_content_width(500);
        set_content_height(500);
    }

//    Gtk::Box& get_drawer_options() override { return opts; }

    static Glib::ustring display_name() { return "Default escape-time drawer"; }

    void set_source(EscapeTimeFractal* sp) { bound = sp; }

    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h) override;
};

class DrawerSelectorFor: public Gtk::ComboBoxText {
public:
    DrawerSelectorFor() {
    }
    template<class T> void add_drawer() {
        append(T::display_name());
    }

    int id() {
        return get_active_row_number();
    }

};

class HistogramColoring: public EscapeTimeDrawer {
public:
    static Glib::ustring display_name() { return "Histogram coloring"; }
    void on_draw(Cairo::RefPtr<Cairo::Context> const& cr, int w, int h) override;
};
