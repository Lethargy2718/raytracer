#pragma once

#include <atomic>
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

class camera {
  public:
    double aspect_ratio = 1.0;                  // Ratio of image width over height
    int image_width  = 1000;                    // Rendered image width in pixel count
    int samples_per_pixel = 10;                 // Count of random samples for each pixel
    int max_depth = 50;                         // Maximum number of bounces
    color background = color(0.70, 0.80, 1.00); // Scene background

    double vfov = 90;
    point3 look_from = point3(0,0,0);
    point3 look_at = point3(0,0,-1);
    vec3 vup = vec3(0,1,0);

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable& world) {
        initialize();

        timer stopwatch;

        std::vector<color> framebuffer(image_width * image_height);
        std::atomic<int> rows_done{0};

        unsigned int thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0) thread_count = 4;

        std::vector<std::thread> threads;
        int rows_per_thread = image_height / thread_count;

        for (unsigned int t = 0; t < thread_count; t++) {
            unsigned int start = t * rows_per_thread;
            unsigned int end = (t == thread_count - 1) ? image_height : start + rows_per_thread;
            threads.emplace_back([=, &world, &framebuffer, &rows_done]() {
                render_rows(start, end, world, framebuffer, rows_done);
            });
        }

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

        for (auto& t : threads) {
            t.join();
        }
        progress.join();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; j++) {
            for (int i = 0; i < image_width; i++) {
                write_color(std::cout, framebuffer[j * image_width + i]);
            }
        }

        std::clog << "\rDone.                                           \n"
                  << "Took " << stopwatch.elapsed() << " seconds.\n";
    }

  private:
    int    image_height;   // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3 u, v, w; // Camera space orthonormal basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = look_from;

        const double viewport_height = 2 * std::tan(math::degrees_to_radians(vfov) / 2) * focus_dist;
        const double viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

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

    void render_rows(int start_j, int end_j,
                     const hittable& world,
                     std::vector<color>& framebuffer,
                     std::atomic<int>& rows_done) const {

        for (int j = start_j; j < end_j; j++) {
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);

                for (int s = 0; s < samples_per_pixel; s++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                framebuffer[j * image_width + i] = pixel_color * pixel_samples_scale;
            }
            ++rows_done;
        }
    }

    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j.

        auto offset = sample_square();
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

    static vec3 sample_square() {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, const int depth, const hittable& world) const {
        if (depth <= 0) return {0,0,0};
        hit_record rec;

        // Hit the sky
        if (!world.hit(r, interval(0.001, math::infinity), rec))
            return background;

        auto &mat = rec.mat;
        ray scattered;
        color attenuation;
        color emission_color = rec.mat->emitted(rec.u, rec.v, rec.p);

        // Only return the natural emission of the mat and ignore the ray
        if (!mat->scatter(r, rec, attenuation, scattered))
            return emission_color;

        color scatter_color = attenuation * ray_color(scattered, depth - 1, world);

        return scatter_color + emission_color;
    }
};