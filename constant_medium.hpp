#pragma once

#include <memory>
#include <cmath>

#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "constants.hpp"

class constant_medium : public hittable {
public:
    constant_medium(const std::shared_ptr<hittable> &boundary, double density, const std::shared_ptr<texture>& tex)
      : boundary(boundary), neg_inv_density(-1/density),
        phase_function(std::make_shared<isotropic>(tex))
    {}

    constant_medium(const std::shared_ptr<hittable> &boundary, double density, const color& albedo)
      : boundary(boundary), neg_inv_density(-1/density),
        phase_function(std::make_shared<isotropic>(albedo))
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record rec1, rec2;

        // Check if ray hits fog
        if (!boundary->hit(r, interval::universe, rec1))
            return false;

        // Check if ray exits fog. Add an epsilon to avoid intersecting again with the same entry point
        if (!boundary->hit(r, interval(rec1.t+0.0001, math::infinity), rec2))
            return false;

        // Compute the intersection of [rec1.t, rec2.t] and [ray_t.min, ray_t.max]
        if (rec1.t < ray_t.min) rec1.t = ray_t.min; // Ex: camera is inside fog
        if (rec2.t > ray_t.max) rec2.t = ray_t.max; // Ex: an object in the fog blocked the ray from exiting

        if (rec1.t >= rec2.t)
            return false;

        if (rec1.t < 0)
            rec1.t = 0;

        // P(t2) - P(t1) = P1P2 = (o+t2*d) - (o+t1*d) = (t2-t1)*d
        // Distance = |P1P2| = |(t2-t1)*d| = (t2-t1)*|d|
        auto ray_length = r.direction().length();
        auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;

        // Exponential distribution inverse CDF sampling
        auto hit_distance = neg_inv_density * std::log(random_double());

        if (hit_distance > distance_inside_boundary)
            return false;

        // Distance moved = t_moved * D = hit_distance
        // t_moved = hit_distance / D
        // t2 = t1 + t_moved
        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        rec.normal = vec3(1,0,0);  // arbitrary
        rec.front_face = true;     // also arbitrary
        rec.mat = phase_function;

        return true;
    }

    aabb bounding_box() const override { return boundary->bounding_box(); }

private:
    std::shared_ptr<hittable> boundary;
    double neg_inv_density;
    std::shared_ptr<material> phase_function;
};