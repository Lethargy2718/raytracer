#pragma once
#include "constants.hpp"

namespace math {
    constexpr double degrees_to_radians(double degrees) {
        return degrees * pi / 180.0;
    }
}