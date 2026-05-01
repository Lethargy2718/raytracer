#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "material.hpp"

int main() {
    hittable_list world;

    auto gray_metal = std::make_shared<metal>(color{0.5,0.5,0.5}, 0.5);
    auto red_metal = std::make_shared<metal>(color{1,0,0}, 0.1f);
    auto blue_metal = std::make_shared<metal>(color{0,0,0.8});
    auto glass_mat = std::make_shared<dielectric>(1.5);
    auto air_bubble_in_glass_mat = std::make_shared<dielectric>(1/1.5);
    auto air_bubble_in_water_mat = std::make_shared<dielectric>(1/1.33);
    auto gray_lambertian = std::make_shared<lambertian>(color{0.2,0.2,0.2});

    // left
    world.add(std::make_shared<sphere>(point3(-2,0,-2), 0.8, red_metal));

    // center
    world.add(std::make_shared<sphere>(point3(0,0,-1), 0.5, glass_mat));
    world.add(std::make_shared<sphere>(point3(0,0,-1), 0.4, air_bubble_in_glass_mat));

    // right
    world.add(std::make_shared<sphere>(point3(1,0,-1), 0.3, air_bubble_in_water_mat));

    // ground
    world.add(std::make_shared<sphere>(point3(0,-100.5,-1), 100, blue_metal));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 1250;
    cam.samples_per_pixel = 10;
    cam.max_depth = 50;

    cam.render(world);
}