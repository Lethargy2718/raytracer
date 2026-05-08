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

        quad::set_bounding_box();
    }

    virtual void set_bounding_box() {
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
        return true;
    }

protected:
    point3 Q;
    vec3 u, v;

private:
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

class annulus : public quad {
public:
    annulus(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat,
            double r_inner)
        : quad(Q, u, v, std::move(mat))
    {
        auto v_length = v.length();
        r_inner_norm = r_inner / v_length;

        if (std::fabs(dot(unit_vector(u), unit_vector(v))) > 1e-4)
            std::cerr << "Warning: u and v are not perpendicular for annulus\n";
        if (std::fabs(u.length() - v.length()) > 1e-4)
            std::cerr << "Warning: u and v have different lengths for annulus\n";
        if (r_inner > v_length)
            std::cerr << "Warning: r_inner > r_outer for annulus\n";
    }

    bool in_quad(double a, double b, hit_record& rec) const override {
        // Map from [0,1] to [-1,1]
        a = a*2 - 1;
        b = b*2 - 1;

        double lhs = a*a + b*b;
        if (lhs < r_inner_norm * r_inner_norm || lhs > 1.0) return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

private:
    double r_inner_norm;
};

class ellipse : public quad {
public:
    ellipse(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat) : quad(Q, u, v, std::move(mat)) {}

    bool in_quad(double a, double b, hit_record& rec) const override {
        // Map from [0,1] to [-1,1]
        a = a*2 - 1;
        b = b*2 - 1;

        if (a*a + b*b > 1)
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
};

class triangle : public quad {
public:
    triangle(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat) : quad(Q, u, v, std::move(mat)) {}

    aabb bounding_box() const override {
        point3 v0 = Q;
        point3 v1 = Q + u;
        point3 v2 = Q + v;
        aabb box(v0, v1);
        box = aabb(box, aabb(v2, v2));
        return box;
    }

    bool in_quad(double a, double b, hit_record& rec) const override {
        if (a < 0 || b < 0 || a + b > 1)
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
};