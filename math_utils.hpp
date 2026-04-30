#pragma once
#include "constants.hpp"

namespace math {
    inline double degrees_to_radians(double degrees) {
        return degrees * pi / 180.0;
    }
}