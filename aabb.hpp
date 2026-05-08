#pragma once

#include "interval.hpp"
#include "vec3.hpp"
#include "ray.hpp"

class aabb {
public:
    interval x, y, z;

    aabb() = default;

    aabb(interval x, interval y, interval z) : x(x), y(y), z(z) {
        pad_to_minimums();
    }

    aabb(const point3& p1, const point3& p2) :
        x(interval(std::min(p1[0], p2[0]), std::max(p1[0], p2[0]))),
        y(interval(std::min(p1[1], p2[1]), std::max(p1[1], p2[1]))),
        z(interval(std::min(p1[2], p2[2]), std::max(p1[2], p2[2])))
    {}

    aabb(const aabb& aabb1, const aabb& aabb2) :
        x(interval(aabb1.x, aabb2.x)),
        y(interval(aabb1.y, aabb2.y)),
        z(interval(aabb1.z, aabb2.z))
    {}

    const interval& axis_interval(int idx) const {
      if (idx == 0) return x;
      if (idx == 1) return y;
      return z;
    }

    bool hit(const ray& r, interval t_interval) const {
      const point3 &origin = r.origin();
      const vec3 &dir = r.direction();

      for (int axis{}; axis < 3; axis++) {
        const interval ax_interval = axis_interval(axis);

        const auto dir_inv = 1.0 / dir[axis]; // To prevent nan errors

        auto t0 = (ax_interval.min - origin[axis]) * dir_inv;
        auto t1 = (ax_interval.max - origin[axis]) * dir_inv;

        // Ensure t0 < t1
        if (t0 > t1) std::swap(t0, t1);

        double mn = std::max(t_interval.min, t0);
        double mx = std::min(t_interval.max, t1);

        if (mn > mx)
          return false;
      }

      return true;
    }

    int longest_axis() const {
        if (x.size() > y.size())
          return x.size() > z.size() ? 0 : 2;
        return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty, universe;

private:
    static constexpr double delta = 0.0001;

    void pad_to_minimums() {
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const aabb aabb::empty    = aabb(interval::empty,    interval::empty,    interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);