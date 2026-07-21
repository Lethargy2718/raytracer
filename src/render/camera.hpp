#pragma once

#include <atomic>
#include <hittable_list.hpp>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>

#include "color.hpp"
#include "ray.hpp"
#include "random.hpp"
#include "hittable.hpp"
#include "constants.hpp"
#include "material.hpp"
#include "timer.hpp"
#include "math_utils.hpp"
#include "pdf.hpp"

class camera {
public:
    double aspect_ratio     = 1.0;                      // Ratio of image width over height
    int image_width         = 1000;                     // Rendered image width in pixel count
    int samples_per_pixel   = 10;                       // Count of random samples for each pixel
    int max_depth           = 50;                       // Maximum number of bounces
    color background        = color(0.70, 0.80, 1.00);  // Scene background color
    double vfov             = 90;                       // Vertical field of view angle in degrees
    point3 look_from        = point3(0,0,0);            // Camera position
    point3 look_at          = point3(0,0,-1);           // Target position
    vec3 vup                = vec3(0,1,0);              // Up vector
    double defocus_angle = 0;                           // Variation angle of rays through each pixel
    double focus_dist = 10;                             // Distance from look_from to plane of perfect focus

    void render(const hittable& world, const hittable& lights) {
        initialize();

        timer stopwatch;

        std::vector<color> framebuffer(image_width * image_height);
        std::atomic<int> rows_done{0};

        unsigned int thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0) thread_count = 4;

        std::vector<std::thread> threads;
        unsigned int rows_per_thread = image_height / thread_count;

        // Create computing threads
        for (size_t t = 0; t < thread_count; t++) {
            size_t start = t * rows_per_thread;
            size_t end = (t == thread_count - 1) ? image_height : start + rows_per_thread;
            threads.emplace_back([=, &world, &lights, &framebuffer, &rows_done]() {
                render_rows(start, end, world, lights, framebuffer, rows_done);
            });
        }

        // Create progress logging thread
        std::thread progress([&]() {
            while (true) {
                int done = rows_done.load();
                double elapsed = stopwatch.elapsed();

                std::clog << "\rRemaining: " << (image_height - done)
                          << " scanlines | Elapsed: " << std::fixed << std::setprecision(0)
                          << elapsed << "s   " << std::flush;

                if (done >= image_height) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        // Join threads
        for (auto& t : threads) {
            t.join();
        }
        progress.join();

        // Write into image
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; j++) {
            for (int i = 0; i < image_width; i++) {
                write_color(std::cout, framebuffer[j * image_width + i]);
            }
        }

        // Console output
        std::clog << "\rDone.                                           \n"
                  << "Took " << stopwatch.elapsed() << " seconds.\n";
    }

private:
    int    image_height = 0;            // Rendered image height
    double pixel_samples_scale = 0;     // Color scale factor for a sum of pixel samples
    int    sqrt_spp = 0;                // Square root of number of samples per pixel
    double recip_sqrt_spp = 0;          // 1 / sqrt_spp
    point3 center;                      // Camera center
    point3 pixel00_loc;                 // Location of pixel 0, 0
    vec3   pixel_delta_u;               // Offset to pixel to the right
    vec3   pixel_delta_v;               // Offset to pixel below
    vec3   u, v, w;                     // Camera space orthonormal basis vectors
    vec3   defocus_disk_u;              // Defocus disk horizontal radius
    vec3   defocus_disk_v;              // Defocus disk vertical radius

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;
        sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));

        center = look_from;

        const double viewport_height = 2 * std::tan(math::degrees_to_radians(vfov) / 2) * focus_dist;
        const double viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // NOTE: use right hand rule to visualize
        w = unit_vector(look_from - look_at);
        u = unit_vector(cross(vup, w)); // always to the right of -w
        v = cross(w, u); // always above w

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        const auto viewport_u = viewport_width * u;
        const auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        const auto viewport_upper_left = center - focus_dist * w - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        const auto defocus_radius = focus_dist * std::tan(math::degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    void render_rows(size_t start_j, size_t end_j, const hittable& world, const hittable& lights, std::vector<color>& framebuffer, std::atomic<int>& rows_done) const {
        for (size_t j = start_j; j < end_j; j++) {
            for (int i = 0; i < image_width; i++) {
                color pixel_color = sample_pixel(i, j, world, lights);
                framebuffer[j * image_width + i] = pixel_color * pixel_samples_scale;
            }
            ++rows_done;
        }
    }

    color sample_pixel(size_t i, size_t j, const hittable& world, const hittable& lights) const {
        // Sample a pixel using Stratified Monte Carlo
        color pixel_color(0,0,0);
        for (int s_j = 0; s_j < sqrt_spp; s_j++) {
            for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                ray r = get_ray(i, j, s_i, s_j);
                pixel_color += ray_color(r, max_depth, world, lights);
            }
        }
        return pixel_color;
    }

    ray get_ray(size_t i, size_t j, int s_i, int s_j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j for stratified sample square s_i, s_j.

        auto offset = sample_square_stratified(s_i, s_j);
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        // Assuming:
        // 1 Frame
        // Shutter duration = frame duration
        // time range is [0,1]
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square_stratified(int s_i, int s_j) const {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return vec3(px, py, 0);
    }

    static vec3 sample_square() {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, const int depth, const hittable& world, const hittable& lights) const {
        if (depth <= 0) return {0,0,0};
        hit_record rec;

        // Hit the sky
        if (!world.hit(r, interval(0.001, math::infinity), rec))
            return background;

        scatter_record srec;
        color emission_color = rec.mat->emitted(r, rec);

        // If it never scattered, only return the natural emission of the mat and ignore the ray
        if (!rec.mat->scatter(r, rec, srec))
            return emission_color;

        // Handle specular rays
        if (srec.skip_pdf) {
            return srec.attenuation * ray_color(srec.skip_pdf_ray, depth-1, world, lights);
        }

        auto light_ptr = std::make_shared<hittable_pdf>(lights, rec.p); // Steers towards light
        mixture_pdf p(light_ptr, srec.pdf_ptr); // Mixes steering with the material's natural scattering distribution

        ray scattered = ray(rec.p, p.generate(), r.time()); // Random scattered ray based on the mixture PDF
        auto pdf_value = p.value(scattered.direction()); // Density for that direction

        // Scattering (based on material)
        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

        // Full form: (Albedo * pScatter * Color) / p
        color sample_color = ray_color(scattered, depth-1, world, lights);
        // color scatter_color = (attenuation * scattering_pdf * sample_color) / pdf_value;
        color scatter_color = (srec.attenuation * scattering_pdf * sample_color) / pdf_value;
        
        return scatter_color + emission_color;
    }
};