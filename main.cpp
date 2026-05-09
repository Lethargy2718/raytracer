#include <memory>

#include "camera.hpp"
#include "hittable_list.hpp"
#include "vec3.hpp"
#include "sphere.hpp"
#include "quad.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "obj_loader.hpp"
#include "constant_medium.hpp"

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

void obj_scene() {
    hittable_list world;

    auto model_mat = make_shared<metal>(color(0.8,0.8,0.8));

    auto triangles = load_obj("bunny.obj", model_mat);

    for (auto& tri : triangles) {
        world.add(tri);
    }

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 1;
    cam.max_depth         = 50;

    cam.vfov        = 30;
    cam.look_from = point3(0, 0.15, 0.5);
    cam.look_at   = point3(0, 0.12, 0);
    cam.vup         = vec3(0, 1, 0);
    cam.defocus_angle = 0;

    cam.render(world);
}

void simple_light() {
    hittable_list world;

    auto pertext = make_shared<perlin_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    auto difflight1 = make_shared<diffuse_light>(color(0,0,1));
    auto difflight2 = make_shared<diffuse_light>(color(1,0,0));
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight1));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight2));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 2560 / 2;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.look_from = point3(26,3,6);
    cam.look_at   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 5;
    cam.background        = color(0,0,0);

    cam.vfov        = 40;
    cam.look_from   = point3(278, 278, -800);
    cam.look_at     = point3(278, 278, 0);
    cam.vup         = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornell_smoke() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    world.add(make_shared<constant_medium>(box1, 0.01, color(1,0,0)));
    world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 300;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.look_from = point3(278, 278, -800);
    cam.look_at   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornell_box_with_bunny() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    // Load bunny
    auto bunny_mat = make_shared<metal>(color(0.8, 0.8, 0.8), 0.05);
    auto bunny_triangles = load_obj("bunny.obj", bunny_mat);
    if (!bunny_triangles.empty()) {
        aabb bbox = aabb::empty;
        for (const auto& tri_ptr : bunny_triangles) {
            auto* tri = dynamic_cast<triangle*>(tri_ptr.get());
            if (tri) bbox = aabb(bbox, tri->bounding_box());
        }

        double y_min = bbox.y.min;
        double x_center = (bbox.x.min + bbox.x.max) * 0.5;
        double z_center = (bbox.z.min + bbox.z.max) * 0.5;
        double bunny_height = bbox.y.max - bbox.y.min;

        double desired_height = 340.0;
        double scale = desired_height / bunny_height;

        vec3 target(278.0, 0.0, 278.0);

        // Center, scale, rotate 180 deg, move to target
        auto transform = [&](point3 p) -> point3 {
            p = point3(p.x() - x_center, p.y() - y_min, p.z() - z_center);
            p = p * scale;
            p = point3(-p.x(), p.y(), -p.z());
            p = p + target;
            return p;
        };

        for (auto& tri_ptr : bunny_triangles) {
            auto* tri = dynamic_cast<triangle*>(tri_ptr.get());
            if (!tri) continue;

            point3 p0 = tri->v0();
            point3 p1 = tri->v1();
            point3 p2 = tri->v2();

            point3 new_p0 = transform(p0);
            point3 new_p1 = transform(p1);
            point3 new_p2 = transform(p2);

            world.add(make_shared<triangle>(new_p0, new_p1 - new_p0, new_p2 - new_p0, bunny_mat));
        }
    }

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = 1500;
    cam.samples_per_pixel = 1000;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov        = 40;
    cam.look_from   = point3(278, 278, -800);
    cam.look_at     = point3(278, 278, 0);
    cam.vup         = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

int main() {
    switch (10) {
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
        case 6:
            obj_scene();
            break;
        case 7:
            simple_light();
            break;
        case 8:
            cornell_box();
            break;
        case 9:
            cornell_smoke();
            break;
        case 10:
            cornell_box_with_bunny();
            break;
        default:
            break;
    }
}