#pragma once

#include "ray.hpp"
#include "color.hpp"

class hit_record;

class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
        return false;
    }
};


class lambertian : public material {
  public:
    lambertian(const color &albedo, const float scatter_probability = 1.0f) : albedo(albedo), scatter_probability(scatter_probability) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        if (random_double(0,1) < scatter_probability) {
            attenuation = albedo / scatter_probability; // expected/average value

            vec3 scatter_dir = rec.normal + random_unit_vector();
            if (scatter_dir.near_zero())
                scatter_dir = rec.normal;

            scattered = ray(rec.p, scatter_dir);
            return true;
        }
        return false;
    }

  private:
    color albedo;
    float scatter_probability;
};

class metal : public material {
  public:
    metal(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        attenuation = albedo;
        vec3 reflected_vector = reflect(r_in.direction(), rec.normal);
        scattered = ray(rec.p, reflected_vector);
        return true;
    }

  private:
    color albedo;
};