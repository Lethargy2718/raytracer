#pragma once

#include <memory>
#include <utility>

#include "hittable.hpp"
#include "ray.hpp"
#include "interval.hpp"

class quad : public hittable {
public:
    quad(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat)
      : Q(Q), u(u), v(v), mat(std::move(mat))
    {
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);

        set_bounding_box();
    }

    void set_bounding_box() {
        // Compute the bounding box of all four vertices.
        auto bbox_diagonal1 = aabb(Q, Q + u + v);
        auto bbox_diagonal2 = aabb(Q + u, Q + v);
        bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        auto denom = dot(normal, r.direction());

        // denom = 0 so ray is parallel
        if (std::fabs(denom) < 1e-8) {
            return false;
        }

        auto t = (D - dot(normal, r.origin())) / denom;
        if (!ray_t.contains(t)) {
            return false;
        }

        auto intersection = r.at(t);
        vec3 QP = intersection - Q;
        auto alpha = dot(w, cross(QP, v));
        auto beta = dot(w, cross(u, QP));

        if (!in_quad(alpha, beta, rec))
            return false;

        rec.t = t;
        rec.p = intersection;
        rec.set_face_normal(r, normal);
        rec.mat = mat;
        rec.u = alpha;
        rec.v = beta;
        return true;
    }

private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    std::shared_ptr<material> mat;
    aabb bbox;
    vec3 normal;
    double D;

    virtual bool in_quad(double a, double b, hit_record& rec) const {
        interval unit_interval = interval(0, 1);

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
};