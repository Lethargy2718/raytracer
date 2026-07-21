#pragma once

#include <utility>

#include "ray.hpp"
#include "color.hpp"
#include "texture.hpp"
#include "pdf.hpp"

class hit_record;

class scatter_record {
public:
    color attenuation;
    std::shared_ptr<pdf> pdf_ptr;
    bool skip_pdf;
    ray skip_pdf_ray;
};

class material {
  public:
    virtual ~material() = default;

    virtual color emitted(const ray& r_in, const hit_record& rec) const {
        return color(0,0,0);
    }

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const {
        return false;
    }

    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
        return 0;
    }
};

class lambertian : public material {
  public:
    lambertian(const color &albedo, const float scatter_probability = 1.0f) : lambertian(std::make_shared<solid_color>(albedo), scatter_probability) {}
    lambertian(std::shared_ptr<texture> tex, const float scatter_probability = 1.0f) : tex(std::move(tex)), scatter_probability(scatter_probability) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec)
    const override {
        // TODO: include russian roulette
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = std::make_shared<cosine_pdf>(rec.normal);
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        return cos_theta < 0 ? 0 : cos_theta/math::pi;
    }

  private:
    std::shared_ptr<texture> tex;
    float scatter_probability;
};

class metal : public material {
  public:
    metal(const color& albedo, const double fuzz = 0) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1)  {}

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());

        srec.attenuation = albedo;
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());

        // TODO: check why the book just returns true instead of checking if fuzz
        // reflected into an invalid direction
        return dot(reflected, rec.normal) > 0.0;
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        double eta_ratio = rec.front_face ? 1.0 / refraction_index : refraction_index;
        vec3 unit_in = unit_vector(r_in.direction());

        double cos = dot(-unit_in, rec.normal);
        double sin = std::sqrt(1 - cos * cos);

        vec3 dir;

        if (eta_ratio * sin > 1.0 || random_double() < reflectance(cos, eta_ratio)) {
            // Can't refract
            dir = reflect(unit_in, rec.normal);
        }
        else {
            dir = refract(unit_in, rec.normal, eta_ratio);
        }

        srec.attenuation = color(1,1,1); // glass absorbs nothing, so everything is returned
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray(rec.p, dir, r_in.time());

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

class diffuse_light : public material {
public:
    diffuse_light(std::shared_ptr<texture> tex) : tex(std::move(tex)) {}
    diffuse_light(const color& emit) : tex(std::make_shared<solid_color>(emit)) {}

    color emitted(const ray& r_in, const hit_record& rec)
    const override {
        if (!rec.front_face)
            return color(0,0,0);
        return tex->value(rec.u, rec.v, rec.p);
    }

private:
    std::shared_ptr<texture> tex;
};

class isotropic : public material {
public:
    explicit isotropic(const color& albedo) : tex(std::make_shared<solid_color>(albedo)) {}
    explicit isotropic(const std::shared_ptr<texture> &tex) : tex(tex) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = std::make_shared<sphere_pdf>();
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
        return 1 / (4 * math::pi);
    }

private:
    std::shared_ptr<texture> tex;
};