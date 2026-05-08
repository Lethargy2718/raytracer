#include <memory>

#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "quad.hpp"
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
    auto marble_tex = make_shared<marble_texture>(6.0, 12.0, 7);

    auto checker_tex_uv = make_shared<checker_texture_uv>(0.1, color(1,0,0), color(.2,.5,.6));
    auto checker_tex_spatial = make_shared<checker_texture_spatial>(0.3, color(0,0,0), color(1,1,1));
    auto checker_mat_uv = make_shared<lambertian>(checker_tex_uv);
    auto checker_mat_spatial = make_shared<lambertian>(checker_tex_spatial);
    auto wall_mat = make_shared<metal>(color(0.8, 0.7, 0.7), 0.0f);
    auto camo = make_shared<lambertian>(make_shared<camo_texture>(8.0, 7));
    auto marble_mat = make_shared<lambertian>(marble_tex);

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    const auto earth_surface = make_shared<lambertian>(earth_texture);

    // left
    world.add(make_shared<sphere>(point3(-1.76,0.4,-1), 0.8, earth_surface));

    // center
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5, checker_mat_uv));

    // right
    world.add(make_shared<sphere>(point3(1.76,0.4,-1), 0.8, air_bubble_in_water_mat));

    // ground
    world.add(make_shared<sphere>(point3(0,-1000.5,-1), 1000, camo));

    // triangle
    auto tri_base = 1.25;
    world.add(make_shared<triangle>(point3(-tri_base/2, 0, -1.5), vec3(tri_base, 0, 0), vec3(tri_base/2, tri_base, 0), blue_metal));

    // wall
    double wall_size = 7;
    world.add(make_shared<quad>(point3(-wall_size, 0, -4), vec3(wall_size * 2, 0, 0), vec3(0, wall_size/3, 0), wall_mat));

    // ring
    auto ring_size = 1.8;
    world.add(make_shared<annulus>(point3(-ring_size/2, 1.5, -1), vec3(ring_size, 0, 0), vec3(0, 0, ring_size), checker_mat_spatial, ring_size * 8.0/10));


    world = hittable_list(make_shared<bvh_node>(world));
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 2560 / 2;
    cam.samples_per_pixel = 10;
    cam.max_depth = 50;
    cam.vfov = 90;

    cam.look_from = point3(0,0.8,1.125);
    cam.look_at = point3(0,0.8,-1);

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

void perlin_spheres() {
    hittable_list world;

    // Ground
    auto camo_tex = make_shared<camo_texture>(8.0, 7);
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(camo_tex)));

    // Left
    auto marble_tex = make_shared<marble_texture>(6.0, 12.0, 7);
    world.add(make_shared<sphere>(point3(-3, 1.5, 0), 1.5, make_shared<lambertian>(marble_tex)));

    // Center
    auto perlin_tex = make_shared<perlin_texture>(8.0);
    world.add(make_shared<sphere>(point3(0, 1.2, 0), 1.2, make_shared<lambertian>(perlin_tex)));

    // Right
    auto marble_diag = make_shared<marble_texture>(5.0, 15.0, 7, vec3(1, 1, 0));
    world.add(make_shared<sphere>(point3(3, 1.5, 0), 1.5, make_shared<lambertian>(marble_diag)));

    // Floating
    auto high_freq_perlin = make_shared<perlin_texture>(20.0);
    world.add(make_shared<sphere>(point3(0, 2.8, 0), 0.5, make_shared<lambertian>(high_freq_perlin)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov        = 40;
    cam.look_from   = point3(8, 4, 8);
    cam.look_at     = point3(0, 1.5, 0);
    cam.vup         = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void quads() {
    hittable_list world;

    // Materials
    auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<metal>(color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    // Middle ring
    world.add(make_shared<annulus>(point3(-1, -1, 1), vec3(2, 0, 0), vec3(0, 2, 0), back_green, 1.75));

    // Middle triangle
    world.add(make_shared<triangle>(point3(-0.5, -0.4, 1), vec3(1, 0, 0), vec3(0.5, 1, 0), back_green));

    // Upper ellipse
    world.add(make_shared<ellipse>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 10;
    cam.max_depth         = 50;

    cam.vfov        = 80;
    cam.look_from   = point3(0,0,9);
    cam.look_at     = point3(0,0,0);
    cam.vup         = vec3(0,1,0);

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
        case 3:
            earth();
            break;
        case 4:
            perlin_spheres();
            break;
        case 5:
            quads();
            break;
        default:
            default_scene();
    }
}