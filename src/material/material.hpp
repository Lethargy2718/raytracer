#pragma once

#include "ray.hpp"
#include "color.hpp"
#include "texture.hpp"
#include "onb.hpp"

class hit_record;

class material {
  public:
    virtual ~material() = default;

    virtual color emitted(double u, double v, const point3& p) const {
        return color(0,0,0);
    }

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
    ) const {
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
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
    ) const override {
        // TODO: include russian roulette
        onb uvw(rec.normal);
        auto scatter_direction = uvw.transform(random_cosine_direction());

        scattered = ray(rec.p, unit_vector(scatter_direction), r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);

        auto cos_theta = dot(uvw.w(), scattered.direction());
        pdf =  cos_theta / math::pi; // TODO: check later if it's always going to be immediately overridden in camera

        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
        // auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        // return cos_theta < 0 ? 0 : cos_theta/math::pi;
        return 1 / (2 * math::pi);
    }

  private:
    std::shared_ptr<texture> tex;
    float scatter_probability;
};

class metal : public material {
  public:
    metal(const color& albedo, const double fuzz = 0) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1)  {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf) const override {
        attenuation = albedo;
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected, r_in.time());
        return dot(reflected, rec.normal) > 0.0;
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf) const override {
        attenuation = color(1,1,1); // glass absorbs nothing, so everything is returned
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

        scattered = ray(rec.p, dir, r_in.time());
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
    diffuse_light(std::shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(std::make_shared<solid_color>(emit)) {}

    color emitted(double u, double v, const point3& p) const override {
        return tex->value(u, v, p);
    }

private:
    std::shared_ptr<texture> tex;
};

class isotropic : public material {
public:
    explicit isotropic(const color& albedo) : tex(std::make_shared<solid_color>(albedo)) {}
    explicit isotropic(const std::shared_ptr<texture> &tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf)
    const override {
        scattered = ray(rec.p, random_unit_vector(), r_in.time()); // full sphere
        attenuation = tex->value(rec.u, rec.v, rec.p);
        pdf = 1 / (4 * math::pi); // samples from full sphere of area 4pi
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
        return 1 / (4 * math::pi);
    }

private:
    std::shared_ptr<texture> tex;
};