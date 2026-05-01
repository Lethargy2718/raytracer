#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "material.hpp"

int main() {
    hittable_list world;

    auto gray_metal = std::make_shared<metal>(color{0.5,0.5,0.5});
    auto red_lambertian = std::make_shared<lambertian>(color{1,0,0});
    auto blue_metal = std::make_shared<metal>(color{0,0,0.8});
    auto green_metal = std::make_shared<metal>(color{0,1,0});

    world.add(std::make_shared<sphere>(point3(0,0,-1), 0.5, gray_metal));
    world.add(std::make_shared<sphere>(point3(-2,0,-2), 0.8, red_lambertian));
    world.add(std::make_shared<sphere>(point3(1,0,-1), 0.2, green_metal));
    world.add(std::make_shared<sphere>(point3(0,-100.5,-1), 100, blue_metal));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 1250;
    cam.samples_per_pixel = 1;

    cam.render(world);
}