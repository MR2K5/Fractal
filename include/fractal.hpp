#pragma once

namespace Gtk {
class DrawingArea;
class Widget;
}

class FractalBase {
public:
    virtual Gtk::DrawingArea& draw_area() = 0;
    virtual Gtk::Widget& get_options()    = 0;
};
