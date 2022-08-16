#include "StaticMesh.h"

#include <glad/glad.h>

#include <unordered_map>
#include <charconv>

#include <iostream>

Result<MeshData> MeshData::from_obj(const std::string& file_name) {
    const auto file = read_text_file(file_name);
    if(!file.is_ok) {
        return {false, {}};
    }

    const glm::vec3 default_color(1.0f);

    std::vector<Vertex> vertices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<u32> indices;
    std::unordered_map<u64, u32> vertex_map;

    std::string_view content = file.value;
    while(!content.empty()) {
        const size_t end_of_line = content.find('\n');
        std::string_view line = end_of_line == std::string_view::npos
            ? content : content.substr(0, end_of_line);
        const std::string_view full_line = line;
        content = content.substr(line.size() + 1);

#define PARSE_ERROR(type) do { std::cerr << "Unable to parse " type " : \"" << full_line << "\"" << std::endl; return {false, {}}; } while(false)

        auto starts_with = [line](std::string_view sv) { return line.substr(0, sv.size()) == sv; };

        auto trim = [&] {
            while(!line.empty() && std::isspace(line.front())) {
                line = line.substr(1);
            }
        };

        auto read_next = [&](auto& value) {
            trim();
            const auto result = std::from_chars(line.data(), line.data() + line.size(), value);
            if(result.ec != std::errc{}) {
                return false;
            }

            line = line.substr(result.ptr - line.data());
            return true;
        };

        if(starts_with("v ")) {
            line = line.substr(2);
            glm::vec3 pos = {};
            if(!(read_next(pos[0]) && read_next(pos[1]) && read_next(pos[2]))) {
                PARSE_ERROR("vertex position");
            }
            if(trim(); !line.empty()) {
                PARSE_ERROR("vertex position");
            }
            positions.push_back(pos);
        } else if(starts_with("vn ")) {
            line = line.substr(3);
            glm::vec3 norm = {};
            if(!(read_next(norm[0]) && read_next(norm[1]) && read_next(norm[2]))) {
                PARSE_ERROR("vertex normal");
            }
            if(trim(); !line.empty()) {
                PARSE_ERROR("vertex normal");
            }
            normals.push_back(norm);
        } else if(starts_with("vt ")) {
            line = line.substr(3);
            glm::vec2 uv = {};
            if(!(read_next(uv[0]) && read_next(uv[1]))) {
                PARSE_ERROR("vertex normal");
            }
            if(trim(); !line.empty()) {
                PARSE_ERROR("vertex normal");
            }
            uvs.push_back(uv);
        } else if(starts_with("f ")) {
            line = line.substr(2);
            for(int i = 0; i != 3; ++i) {
                u64 pos_idx = 0;
                u64 uv_idx = 0;
                u64 norm_idx = 0;
                if(!(read_next(pos_idx) && pos_idx > 0 && pos_idx <= positions.size())) {
                    PARSE_ERROR("face position");
                } else if(trim(); !line.empty() && line[0] == '/') {
                    line = line.substr(1);
                    if(trim(); !line.empty() && line[0] != '/') {
                        if(!(read_next(uv_idx) && uv_idx > 0 && uv_idx <= uvs.size())) {
                            PARSE_ERROR("face uv");
                        }
                    }
                    if(trim(); !line.empty() && line[0] == '/') {
                        line = line.substr(1);
                        if(!(read_next(norm_idx) && norm_idx > 0 && norm_idx <= normals.size())) {
                            PARSE_ERROR("face normal");
                        }
                    }
                }

                const u64 key = (pos_idx << 42) | (norm_idx << 21) | (uv_idx);
                if(const auto it = vertex_map.find(key); it != vertex_map.end()) {
                    indices.push_back(it->second);
                } else {
                    const u32 index = u32(vertices.size());
                    vertices.push_back(Vertex{
                        positions[pos_idx - 1],
                        norm_idx ? normals[norm_idx - 1] : glm::vec3{},
                        uv_idx ? uvs[uv_idx - 1] : glm::vec2{},
                        default_color
                    });
                    vertex_map[key] = index;
                    indices.push_back(index);
                }
            }
            if(trim(); !line.empty()) {
                PARSE_ERROR("face");
            }
        }
    }

    return {true, MeshData{std::move(vertices), std::move(indices)}};
}

StaticMesh::StaticMesh(const MeshData& data) :
    _vertex_buffer(data.vertices),
    _index_buffer(data.indices) {
}

void StaticMesh::draw() const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}
