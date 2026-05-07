#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "material.hpp"
#include "bvh.hpp"

void default_scene() {
    hittable_list world;

    auto gray_metal = std::make_shared<metal>(color{0.5,0.5,0.5}, 0.5);
    auto red_metal = std::make_shared<metal>(color{1,0,0}, 0.1f);
    auto blue_metal = std::make_shared<metal>(color{0,0,0.8});
    auto glass_mat = std::make_shared<dielectric>(1.5);
    auto air_bubble_in_glass_mat = std::make_shared<dielectric>(1/1.5);
    auto air_bubble_in_water_mat = std::make_shared<dielectric>(1/1.33);
    auto gray_lambertian = std::make_shared<lambertian>(color{0.2,0.2,0.2});

    auto checker_tex_uv = std::make_shared<checker_texture_uv>(0.1, color(1,0,0), color(.2,.5,.6));
    auto checker_tex_spatial = std::make_shared<checker_texture_spatial>(0.3, color(0,0,0), color(1,1,1));
    auto checker_mat_uv = std::make_shared<lambertian>(checker_tex_uv);
    auto checker_mat_spatial = std::make_shared<lambertian>(checker_tex_spatial);

    // left
    world.add(std::make_shared<sphere>(point3(-1.76,0.4,-1), 0.8, red_metal));

    // center
    world.add(std::make_shared<sphere>(point3(0,0,-1), 0.5, checker_mat_uv));

    // right
    world.add(std::make_shared<sphere>(point3(1.76,0.4,-1), 0.8, air_bubble_in_water_mat));

    // ground
    world.add(std::make_shared<sphere>(point3(0,-1000.5,-1), 1000, checker_mat_spatial));

    world = hittable_list(std::make_shared<bvh_node>(world));
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

    auto checker_uv = std::make_shared<checker_texture_uv>(.2, color(.2, .3, .1), color(.9, .9, .9));
    auto checker_spatial = std::make_shared<checker_texture_spatial>(.2, color(.2, .3, .1), color(.9, .9, .9));

    world.add(std::make_shared<sphere>(point3(0,-1, 0), 1, std::make_shared<lambertian>(checker_uv)));
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1, std::make_shared<lambertian>(checker_spatial)));

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

int main() {
    switch (1) {
        case 1:
            default_scene();
            break;
        case 2:
            checkered_spheres();
            break;
        default:
            break;
    }
}