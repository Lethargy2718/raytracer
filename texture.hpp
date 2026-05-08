#pragma once

#include <memory>
#include <utility>

#include "color.hpp"
#include "vec3.hpp"
#include "rtw_stb_image.hpp"
#include "perlin.hpp"

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

class checker_texture_spatial : public texture {
public:
    checker_texture_spatial(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
      : inv_scale(1.0 / scale), even(std::move(even)), odd(std::move(odd)) {}

    checker_texture_spatial(double scale, const color& c1, const color& c2)
      : checker_texture_spatial(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        int x = static_cast<int>(std::floor(inv_scale * p.x()));
        int y = static_cast<int>(std::floor(inv_scale * p.y()));
        int z = static_cast<int>(std::floor(inv_scale * p.z()));
        bool isEven = (x + y + z) % 2 == 0;
        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    std::shared_ptr<texture> even;
    std::shared_ptr<texture> odd;
};

class checker_texture_uv : public texture {
public:
    checker_texture_uv(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
      : inv_scale(1.0 / scale), even(std::move(even)), odd(std::move(odd)) {}

    checker_texture_uv(double scale, const color& c1, const color& c2)
      : checker_texture_uv(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        int ui = static_cast<int>(std::floor(inv_scale * u));
        int vi = static_cast<int>(std::floor(inv_scale * v));
        bool isEven = (ui + vi) % 2 == 0;
        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    std::shared_ptr<texture> even;
    std::shared_ptr<texture> odd;
};

class image_texture : public texture {
public:
    image_texture(const char* filename) : image(filename) {}

    color value(double u, double v, const point3& p) const override {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (image.height() <= 0) return color(0,1,1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = interval(0,1).clamp(u);

        // Flip V to image coordinates because texture coordinates have v = 0 at the sphere's bottom
        // and v = 1 at the top, while it's the opposite for images where the top row is 0 and the
        // bottom is 1

        v = 1.0 - interval(0,1).clamp(v);

        auto i = static_cast<int>(u * image.width());
        auto j = static_cast<int>(v * image.height());
        auto pixel = image.pixel_data(i,j);

        auto color_scale = 1.0 / 255.0; // Less cpu cycles
        return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

private:
    rtw_image image;
};

class perlin_texture : public texture {
public:
    explicit perlin_texture(const double scale = 4.0) : scale(scale) {}

    color value(double u, double v, const point3& p) const override {
        return color(1, 1, 1) * (1 + noise.noise(scale * p));
    }

private:
    perlin noise;
    double scale;
};

class camo_texture : public texture {
public:
    explicit camo_texture(const double scale = 4.0, const int depth = 7) : scale(scale), depth(depth) {}

    color value(double u, double v, const point3& p) const override {
        return color(1, 1, 1) * noise.turb(p, depth);
    }

private:
    perlin noise;
    double scale;
    int depth;
};

class marble_texture : public texture {
public:
    explicit marble_texture(
        double scale = 4.0,
        double warp_strength = 10.0,
        int octaves = 7,
        const vec3 &stripe_axis = vec3(0,0,1)
    ) : scale(scale), warp_strength(warp_strength), octaves(octaves), stripe_axis(unit_vector(stripe_axis)) {}

    color value(double u, double v, const point3& p) const override {
        double t = dot(p, stripe_axis);
        double val = std::sin(scale * t + warp_strength * noise.turb(p, octaves));
        return color(0.5, 0.5, 0.5) * (1 + val);
    }

private:
    perlin noise;
    double scale;           // Stripe frequency
    double warp_strength;   // How wavy (10 is typical)
    int octaves;            // Turbulence detail
    vec3 stripe_axis;       // Which direction to stripe along
};