#pragma once

#include <memory>

#include "hittable.hpp"
#include "constants.hpp"

class sphere : public hittable {
  public:
    // Static
    sphere(const point3& center, const double radius, std::shared_ptr<material> mat) : center(center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(std::move(mat)) {
        const auto rvec = vec3(radius, radius, radius);
        bbox = {center - rvec, center + rvec};
    }

    // Moving
    sphere(const point3& center1, const point3& center2, const double radius, std::shared_ptr<material> mat) : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(std::move(mat)) {
        auto rvec = vec3(radius, radius, radius);
        aabb box1(center.at(0) - rvec, center.at(0) + rvec);
        aabb box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = aabb(box1, box2);
    }

    bool hit(const ray& r, const interval ray_t, hit_record& rec) const override {
        vec3 oc = center.at(r.time()) - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center.at(r.time())) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    ray center;
    double radius;
    std::shared_ptr<material> mat;
    aabb bbox;

    static void get_sphere_uv(const point3& p, double& u, double& v) {
        // p: a given point on the unit sphere, centered at the origin.
        // the same thing as the unit direction from the center to the point.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.

        auto theta = std::acos(-p.y());
        auto phi = std::atan2(-p.z(), p.x());
        if (phi < 0) phi += 2 * math::pi; // Handle flip/discontinuity

        u = phi / (2 * math::pi);
        v = theta / math::pi;
    }
};