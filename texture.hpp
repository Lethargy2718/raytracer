#pragma once

#include <memory>
#include <utility>

#include "color.hpp"
#include "vec3.hpp"

class texture {
public:
    virtual ~texture() = default;

    virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
public:
    solid_color(const color& albedo) : albedo(albedo) {}

    solid_color(double red, double green, double blue) : solid_color(color(red,green,blue)) {}

    color value(double u, double v, const point3& p) const override {
        return albedo;
    }

private:
    color albedo;
};

class checker_texture : public texture {
public:
    checker_texture(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
      : inv_scale(1.0 / scale), even(std::move(even)), odd(std::move(odd)) {}

    checker_texture(double scale, const color& c1, const color& c2)
      : checker_texture(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        // Dividing by scale means more numbers get floored to the same color
        // so squares become bigger
        int xInteger = static_cast<int>(std::floor(inv_scale * p.x()));
        int yInteger = static_cast<int>(std::floor(inv_scale * p.y()));
        int zInteger = static_cast<int>(std::floor(inv_scale * p.z()));

        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    std::shared_ptr<texture> even;
    std::shared_ptr<texture> odd;
};