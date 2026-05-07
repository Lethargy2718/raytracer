#include <memory>

#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "material.hpp"
#include "bvh.hpp"

using std::make_shared;

void default_scene() {
    hittable_list world;

    auto gray_metal = make_shared<metal>(color{0.5,0.5,0.5}, 0.5);
    auto red_metal = make_shared<metal>(color{1,0,0}, 0.1f);
    auto blue_metal = make_shared<metal>(color{0,0,0.8});
    auto glass_mat = make_shared<dielectric>(1.5);
    auto air_bubble_in_glass_mat = make_shared<dielectric>(1/1.5);
    auto air_bubble_in_water_mat = make_shared<dielectric>(1/1.33);
    auto gray_lambertian = make_shared<lambertian>(color{0.2,0.2,0.2});

    auto checker_tex_uv = make_shared<checker_texture_uv>(0.1, color(1,0,0), color(.2,.5,.6));
    auto checker_tex_spatial = make_shared<checker_texture_spatial>(0.3, color(0,0,0), color(1,1,1));
    auto checker_mat_uv = make_shared<lambertian>(checker_tex_uv);
    auto checker_mat_spatial = make_shared<lambertian>(checker_tex_spatial);

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    const auto earth_surface = make_shared<lambertian>(earth_texture);

    // left
    world.add(make_shared<sphere>(point3(-1.76,0.4,-1), 0.8, earth_surface));

    // center
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5, checker_mat_uv));

    // right
    world.add(make_shared<sphere>(point3(1.76,0.4,-1), 0.8, air_bubble_in_water_mat));

    // ground
    world.add(make_shared<sphere>(point3(0,-1000.5,-1), 1000, checker_mat_spatial));

    world = hittable_list(make_shared<bvh_node>(world));
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 2560 / 2;
    cam.samples_per_pixel = 10;
    cam.max_depth = 50;
    cam.vfov = 90;

    cam.look_from = point3(0,0.5,1);
    cam.look_at = point3(0,0,-1);

    cam.render(world);
}

void checkered_spheres() {
    hittable_list world;

    auto checker_uv = make_shared<checker_texture_uv>(.2, color(.2, .3, .1), color(.9, .9, .9));
    auto checker_spatial = make_shared<checker_texture_spatial>(.2, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-1, 0), 1, make_shared<lambertian>(checker_uv)));
    world.add(make_shared<sphere>(point3(0, 1, 0), 1, make_shared<lambertian>(checker_spatial)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 10;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.look_from = point3(13,2,1);
    cam.look_at   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void earth() {
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    const auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.look_from = point3(0,0,12);
    cam.look_at   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(hittable_list(globe));
}

int main() {
    switch (1) {
        case 1:
            default_scene();
            break;
        case 2:
            checkered_spheres();
            break;
        case 3:
            earth();
            break;
        default:
            break;
    }
}