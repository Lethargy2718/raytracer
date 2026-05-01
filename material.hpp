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
    metal(const color& albedo, const double fuzz = 0) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1)  {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        attenuation = albedo;
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        return dot(reflected, rec.normal) > 0.0;
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        attenuation = color(1,1,1); // glass absorbs nothing, so everything is returned
        double eta_ratio = rec.front_face ? 1.0 / refraction_index : refraction_index;
        vec3 unit_in = unit_vector(r_in.direction());

        double cos = dot(-unit_in, rec.normal);
        double sin = std::sqrt(1 - cos * cos);

        vec3 dir;

        if (eta_ratio * sin > 1.0 || random_double() > reflectance(cos, eta_ratio)) {
            // Can't refract
            dir = reflect(unit_in, rec.normal);
        }
        else {
            dir = refract(unit_in, rec.normal, eta_ratio);
        }

        scattered = ray(rec.p, dir);
        return true;
    }

  private:
    double refraction_index; // eta in / eta out

    // Probability of reflection instead of refraction
    static double reflectance(const double cosine, const double refraction_index) {
        // Schlick's approximation
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};