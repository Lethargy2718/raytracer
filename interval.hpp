#pragma once

#include "constants.hpp"

class interval {
  public:
    double min, max;

    interval() : min(+math::infinity), max(-math::infinity) {} // Default interval is empty

    interval(double min, double max) : min(min), max(max) {}

    interval(const interval& a, const interval& b) {
        // Create the interval tightly enclosing the two input intervals.
        min = std::min(a.min, b.min);
        max = std::max(a.max, b.max);
    }

    double size() const {
        return max - min;
    }

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }
    
    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    interval expand(const double delta) const {
        auto padding = delta/2;
        return {min - padding, max + padding};
    }

    static const interval empty, universe;
};

const interval interval::empty    = interval(+math::infinity, -math::infinity);
const interval interval::universe = interval(-math::infinity, +math::infinity);