#pragma once

#include <Eigen/Core>

using vec2 = Eigen::Vector2d;

using RGB = Eigen::Vector3d;

class Palette {
    const std::vector<std::pair<double, RGB>> palette = {
        {0,    {0 / 255.0, 7 / 255.0, 100 / 255.0}    },
        {0.16, {32 / 255.0, 107 / 255.0, 203 / 255.0} },
        {0.32, {237 / 255.0, 255 / 255.0, 255 / 255.0}},
        {0.50, {255 / 255.0, 170 / 255.0, 0 / 255.0}  },
        {0.86, {150 / 255.0, 32 / 255.0, 0 / 255.0}   },
        {1,    {0 / 255.0, 0 / 255.0, 0 / 255.0}      },
    };

    RGB lerp(RGB a, RGB b, double x) { return a + x * (b - a); }

public:
    RGB operator[](double hue) {
        //        assert(0 <= hue && hue <= 1);

        int a = 0, b;
        for (size_t i = 0; i < palette.size(); ++i) {
            if (palette[i].first < hue) { a = i; }
        }
        b = a + 1;

        auto pa  = palette[a];
        auto pb  = palette[b];
        double x = (hue - pa.first) / std::abs(pb.first - pa.first);
        //        assert(0 <= x && x <= 1);
        return lerp(pa.second, pb.second, x) * 255.0;
    }
};

inline RGB get_color_for_hue(double hue) {
    assert(0 <= hue <= 1);
    static Palette pl;

    return pl[hue];
}

[[noreturn]] inline void unreachable() {
#if defined(__GNUC__) || defined(__clang__)
    __builtin_unreachable();
#endif
}
