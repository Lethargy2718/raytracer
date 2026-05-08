#pragma once

#include <algorithm>
#include <cmath>

#include "random.hpp"
#include "vec3.hpp"

class perlin {
public:
    explicit perlin() {
        for (int i = 0; i < point_count; i++) {
            randvec[i] = unit_vector(vec3::random(-1, 1));
        }

        perlin_generate_perm(perm_x);
        perlin_generate_perm(perm_y);
        perlin_generate_perm(perm_z);
    }

    double noise(const point3& p) const {
        // Returns a value in [-1,1]

        // Instead of just picking the value of the closest grid point, we take a weighted
        // average of the contributions from all 8 surrounding corners. Each corner stores
        // a random unit vector. The contribution from a corner is the dot product of that
        // vector with the offset from the corner to the point (how aligned they are).
        // Weights are based on the distance between the corner and the point.
        // This creates smooth gradients rather than blocky jumps at grid points.

        // Fractional part of x, y, and z to tell how far p is in its cube
        auto u = p.x() - std::floor(p.x());
        auto v = p.y() - std::floor(p.y());
        auto w = p.z() - std::floor(p.z());

        // Bottom-left corner of the current cube
        auto i = static_cast<int>(std::floor(p.x()));
        auto j = static_cast<int>(std::floor(p.y()));
        auto k = static_cast<int>(std::floor(p.z()));

        vec3 c[2][2][2];

        for (int di = 0; di < 2; di++) {
            for (int dj = 0; dj < 2; dj++) {
                for (int dk = 0; dk < 2; dk++) {
                    auto cornerX = i + di;
                    auto cornerY = j + dj;
                    auto cornerZ = k + dk;
                    // & 255 is equivalent to % 256
                    c[di][dj][dk] = randvec[perm_x[cornerX & 255] ^ perm_y[cornerY & 255] ^ perm_z[cornerZ & 255]];
                }
            }
        }

        return perlin_interp(c, u, v, w);
    }

    double turb(const point3& p, int depth) const {
        auto accum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        for (int i = 0; i < depth; i++) {
            accum += weight * noise(temp_p);
            weight *= 0.5;
            temp_p *= 2;
        }

        return std::fabs(accum);
    }

private:
    static constexpr int point_count = 256;
    vec3 randvec[point_count]{};
    int perm_x[point_count]{};
    int perm_y[point_count]{};
    int perm_z[point_count]{};

    static void perlin_generate_perm(int p[]) {
        for (int i = 0; i < point_count; i++)
            p[i] = i;

        permute(p, point_count);
    }

    static void permute(int p[], int n) {
        for (int i = n-1; i > 0; i--) {
            int target = random_int(0, i);
            std::swap(p[target], p[i]);
        }
    }

    static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w) {
        // Hermite smoothing to remove grid artifacts
        auto uu = u*u*(3-2*u);
        auto vv = v*v*(3-2*v);
        auto ww = w*w*(3-2*w);

        double result = 0.0;

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    // Weight for this corner based on the distance between it and the point
                    double weightX = i == 0 ? 1.0 - uu : uu;
                    double weightY = j == 0 ? 1.0 - vv : vv;
                    double weightZ = k == 0 ? 1.0 - ww : ww;
                    double weight = weightX * weightY * weightZ;

                    // Vector from the corner to the point (p - corner)
                    vec3 offset(u - i, v - j, w - k);

                    // Weighted alignment of the corner vector with the offset
                    result += weight * dot(c[i][j][k], offset);
                }
            }
        }

        return result;
    }
};