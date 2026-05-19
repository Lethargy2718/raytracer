#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "quad.hpp"

// Parses a face token which can be any of:
// v
// v/vt
// v//vn
// v/vt/vn
// Returns the vertex index (1-based), or 0 on failure.
// TODO: read vt and vn as well
static int parse_face_vertex_index(const std::string& token) {
    char* end;
    long v = strtol(token.c_str(), &end, 10);
    if (end == token.c_str()) {  // No characters consumed
        return 0;
    }
    return static_cast<int>(v);
}

inline std::vector<std::shared_ptr<hittable>> load_obj(const std::string& filename, const std::shared_ptr<material>& mat) {
    std::vector<point3> vertices;
    std::vector<std::shared_ptr<hittable>> tris;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << filename << "\n";
        return {};
    }

    int line_num = 0;
    std::string line;

    while (std::getline(file, line)) {
        line_num++;

        // Strip Windows-style \r
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            double x, y, z;
            if (!(iss >> x >> y >> z)) { // Extra numbers in the line are ignored
                std::cerr << "Warning: malformed vertex on line " << line_num << "\n";
                continue;
            }
            vertices.emplace_back(x, y, z);
        }
        else if (token == "f") {
            std::vector<int> face_indices;
            std::string face_token;

            while (iss >> face_token) {
                int idx = parse_face_vertex_index(face_token);
                if (idx == 0) {
                    std::cerr << "Warning: bad face token '" << face_token << "' on line " << line_num << "\n";
                    face_indices.clear();
                    break;
                }

                // From end of list
                if (idx < 0)
                    idx = static_cast<int>(vertices.size()) + idx + 1;

                if (idx < 1 || idx > static_cast<int>(vertices.size())) {
                    std::cerr << "Warning: vertex index " << idx << " out of range on line " << line_num << "\n";
                    face_indices.clear();
                    break;
                }

                face_indices.push_back(idx);
            }

            if (face_indices.size() < 3) continue;

            // Fan triangulation: break polygons into triangles v0v1v2, v0v2v3, v0v3v4, etc...
            const point3& v0 = vertices[face_indices[0] - 1];
            for (int i = 1; i + 1 < static_cast<int>(face_indices.size()); ++i) {
                const point3& v1 = vertices[face_indices[i] - 1];
                const point3& v2 = vertices[face_indices[i + 1] - 1];
                tris.push_back(std::make_shared<triangle>(v0, v1 - v0, v2 - v0, mat));
            }
        }
    }

    std::clog << "Loaded " << tris.size() << " triangles from " << filename << " (" << vertices.size() << " vertices)\n";
    return tris;
}