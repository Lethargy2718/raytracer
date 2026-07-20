#pragma once

#include <utility>

#include "onb.hpp"
#include "vec3.hpp"
#include "constants.hpp"
#include "hittable_list.hpp"

class pdf {
public:
    virtual ~pdf() = default;

    // Density at a specific direction
    virtual double value(const vec3& direction) const = 0;

    // Sampled direction
    virtual vec3 generate() const = 0;
};

// Full sphere
class sphere_pdf : public pdf {
public:
    sphere_pdf() {}

    double value(const vec3& direction) const override {
        return 1/ (4 * math::pi);
    }

    vec3 generate() const override {
        return random_unit_vector();
    }
};

// Cosine-weighted hemispherical sampling
class cosine_pdf : public pdf {
public:
    cosine_pdf(const vec3& w) : uvw(w) {}

    double value(const vec3& direction) const override {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta/math::pi);
    }

    vec3 generate() const override {
        return uvw.transform(random_cosine_direction());
    }

private:
    onb uvw;
};

// Uniform sampling over hittable surfaces, which will be light sources.
class hittable_pdf : public pdf {
public:
    // "origin" is the point light is shot from
    hittable_pdf(const hittable& objects, const point3& origin)
      : objects(objects), origin(origin)
    {}

    double value(const vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override {
        // Random vector from where the ray of light starts
        // to a uniform random point on the light sources
        return objects.random(origin);
    }

private:
    const hittable& objects;
    point3 origin;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(std::shared_ptr<pdf> p0, std::shared_ptr<pdf> p1) {
        p[0] = std::move(p0);
        p[1] = std::move(p1);
    }

    double value(const vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override {
        // NOTE: Combining them to form one net PDF is also valid, but
        // can be infeasible due to the difficulty of the integral needed
        // to find the ICD. This monte carlo method equivalently achieves
        // the same result. It averages out at some point.
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

private:
    std::shared_ptr<pdf> p[2];
};