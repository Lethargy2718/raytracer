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

        area = n.length();

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

    double pdf_value(const point3& origin, const vec3& direction) const override {
        hit_record rec;

        // If light never even hits the light source, the probability (density) of reaching it is 0
        if (!this->hit(ray(origin, direction), interval(0.001, math::infinity), rec))
            return 0;

        auto distance_squared = rec.t * rec.t * direction.length_squared();
        auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);
    }

    vec3 random(const point3& origin) const override {
        auto random_on_surface = Q + (random_double() * u) + (random_double() * v); // Q is the top left point
        return random_on_surface - origin; // Q - P = PQ
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
    double area;

    virtual bool in_quad(double a, double b, hit_record& rec) const {
        interval unit_interval = interval(0, 1);

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
};

inline std::shared_ptr<hittable_list> box(const point3& a, const point3& b, std::shared_ptr<material> mat)
{
    using std::make_shared;
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}

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
    triangle(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat)
        : quad(Q, u, v, std::move(mat)), ver1(Q + u), ver2(Q + v) {}

    aabb bounding_box() const override {
        aabb box(Q, ver1);
        box = aabb(box, aabb(ver2, ver2));
        return box;
    }

    bool in_quad(double a, double b, hit_record& rec) const override {
        if (a < 0 || b < 0 || a + b > 1)
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

    const point3& v0() const { return Q; }
    const point3& v1() const { return ver1; }
    const point3& v2() const { return ver2; }

private:
    point3 ver1, ver2;
};