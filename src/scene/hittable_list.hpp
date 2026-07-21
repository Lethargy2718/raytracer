#pragma once

#include "hittable.hpp"
#include <memory>
#include <vector>

class hittable_list : public hittable {
  public:
    std::vector<std::shared_ptr<hittable>> objects;

    hittable_list() = default;

    hittable_list(const std::shared_ptr<hittable>& object) { add(object); }

    void clear() { objects.clear(); }

    void add(const std::shared_ptr<hittable>& object) {
        objects.push_back(object);
        bbox = aabb(bbox, object->bounding_box());
    }

    aabb bounding_box() const override { return bbox; }

    //
    double pdf_value(const point3& origin, const vec3& direction) const override {
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += object->pdf_value(origin, direction);

        return sum / objects.size();
    }

    // Pick a random hittable, then pick a random point on its
    // surface and generate a vector from the origin to that
    // point.
    vec3 random(const point3& origin) const override {
        auto int_size = int(objects.size());
        return objects[random_int(0, int_size-1)]->random(origin);
    }

    // Hits the closest object
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

  private:
    aabb bbox;
};
