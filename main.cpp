#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "material.hpp"

int main() {
    hittable_list world;

    world.add(std::make_shared<sphere>(point3(0,0,-1), 0.5));
    world.add(std::make_shared<sphere>(point3(-2,0,-2), 0.8));
    world.add(std::make_shared<sphere>(point3(0,-100.5,-1), 100));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 1250;
    cam.samples_per_pixel = 1;

    cam.render(world);
}