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

    const glm::vec3 default_color(0.7f, 0.7f, 0.7f);

    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    // std::unordered_map<u64, u32> index_map;

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
            size_t spaces = 0;
            for(const char c : line) {
                if(!std::isspace(c)) {
                    break;
                }
                ++spaces;
            }
            line = line.substr(spaces);
        };

        auto remove_extras = [&] {
            size_t extras = 0;
            for(const char c : line) {
                if(std::isspace(c)) {
                    break;
                }
                ++extras;
            }
            line = line.substr(extras);
            return true;
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
            vertices.push_back(Vertex{pos, default_color});
        } else if(starts_with("f ")) {
            line = line.substr(2);
            for(int i = 0; i != 3; ++i) {
                u32 idx = 0;
                if(!(read_next(idx) && remove_extras() && idx > 0)) {
                    PARSE_ERROR("face");
                }
                indices.push_back(idx - 1);
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
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}
