#include "Scene.h"
#include "StaticMesh.h"

#include <glm/gtc/quaternion.hpp>

#include <utils.h>

#include <iostream>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#include <tinygltf/tiny_gltf.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace OM3D {

bool display_gltf_loading_warnings = false;

static size_t component_count(int type) {
    switch(type) {
        case TINYGLTF_TYPE_SCALAR: return 1;
        case TINYGLTF_TYPE_VEC2: return 2;
        case TINYGLTF_TYPE_VEC3: return 3;
        case TINYGLTF_TYPE_VEC4: return 4;
        case TINYGLTF_TYPE_MAT2: return 4;
        case TINYGLTF_TYPE_MAT3: return 9;
        case TINYGLTF_TYPE_MAT4: return 16;
        default: return 0;
    }
}

static bool decode_attrib_buffer(const tinygltf::Model& gltf, const std::string& name, const tinygltf::Accessor& accessor, Span<Vertex> vertices) {
    const tinygltf::BufferView& buffer = gltf.bufferViews[accessor.bufferView];

    if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        if(display_gltf_loading_warnings) {
            std::cerr << "Unsupported component type (" << accessor.componentType << ") for \"" << name << "\"" << std::endl;
        }
        return false;
    }

    [[maybe_unused]]
    const size_t vertex_count = vertices.size();

    auto decode_attribs =  [&](auto* vertex_elems) {
        using attrib_type = std::remove_reference_t<decltype(vertex_elems[0])>;
        using value_type = typename attrib_type::value_type;
        static constexpr size_t size = sizeof(attrib_type) / sizeof(value_type);

        const size_t components = component_count(accessor.type);
        const bool normalize = accessor.normalized;

        DEBUG_ASSERT(accessor.count == vertex_count);

        if(components != size) {
            if(display_gltf_loading_warnings) {
                std::cerr << "Expected VEC" << size << " attribute, got VEC" << components << std::endl;
            }
        }

        const size_t min_size = std::min(size, components);
        auto convert = [=](const u8* data) {
            attrib_type vec;
            for(size_t i = 0; i != min_size; ++i) {
                vec[int(i)] = reinterpret_cast<const value_type*>(data)[i];
            }
            if(normalize) {
                if constexpr(size == 4) {
                    const glm::vec3 n = glm::normalize(glm::vec3(vec));
                    vec[0] = n[0];
                    vec[1] = n[1];
                    vec[2] = n[2];
                } else {
                    vec = glm::normalize(vec);
                }
            }
            return vec;
        };

        {
            u8* out_begin = reinterpret_cast<u8*>(vertex_elems);

            const auto& in_buffer = gltf.buffers[buffer.buffer].data;
            const u8* in_begin = in_buffer.data() + buffer.byteOffset + accessor.byteOffset;
            const size_t attrib_size = components * sizeof(value_type);
            const size_t input_stride = buffer.byteStride ? buffer.byteStride : attrib_size;

            for(size_t i = 0; i != accessor.count; ++i) {
                const u8* attrib = in_begin + i * input_stride;
                DEBUG_ASSERT(attrib < in_buffer.data() + in_buffer.size());
                *reinterpret_cast<attrib_type*>(out_begin + i * sizeof(Vertex)) = convert(attrib);
            }
        }
    };

    if(name == "POSITION") {
        decode_attribs(&vertices[0].position);
    } else if(name == "NORMAL") {
        decode_attribs(&vertices[0].normal);
    } else if(name == "TANGENT") {
        decode_attribs(&vertices[0].tangent_bitangent_sign);
    } else if(name == "TEXCOORD_0") {
        decode_attribs(&vertices[0].uv);
    } else if(name == "COLOR_0") {
        decode_attribs(&vertices[0].color);
    } else {
        if(display_gltf_loading_warnings) {
            std::cerr << "Attribute \"" << name << "\" is not supported" << std::endl;
        }
    }
    return true;
}

static bool decode_index_buffer(const tinygltf::Model& gltf, const tinygltf::Accessor& accessor, Span<u32> indices) {
    const tinygltf::BufferView& buffer = gltf.bufferViews[accessor.bufferView];

    auto decode_indices = [&](u32 elem_size, auto convert_index) {
        const u8* in_buffer = gltf.buffers[buffer.buffer].data.data() + buffer.byteOffset + accessor.byteOffset;
        const size_t input_stride = buffer.byteStride ? buffer.byteStride : elem_size;

        for(size_t i = 0; i != accessor.count; ++i) {
            indices[i] = convert_index(in_buffer + i * input_stride);
        }
    };

    switch(accessor.componentType) {
        case TINYGLTF_PARAMETER_TYPE_BYTE:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
            decode_indices(1, [](const u8* data) -> u32 { return *data; });
        break;

        case TINYGLTF_PARAMETER_TYPE_SHORT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
            decode_indices(2, [](const u8* data) -> u32 { return *reinterpret_cast<const u16*>(data); });
        break;

        case TINYGLTF_PARAMETER_TYPE_INT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
            decode_indices(4, [](const u8* data) -> u32 { return *reinterpret_cast<const u32*>(data); });
        break;

        default:
            std::cerr << "Index component type not supported" << std::endl;
            return false;
    }

    return true;
}

static Result<MeshData> build_mesh_data(const tinygltf::Model& gltf, const tinygltf::Primitive& prim) {
    std::vector<Vertex> vertices;
    for(auto&& [name, id] : prim.attributes) {
        tinygltf::Accessor accessor = gltf.accessors[id];
        if(!accessor.count) {
            continue;
        }

        if(accessor.sparse.isSparse) {
            return {false, {}};
        }

        if(!vertices.size()) {
            std::fill_n(std::back_inserter(vertices), accessor.count, Vertex{});
        } else if(vertices.size() != accessor.count) {
            return {false, {}};
        }

        if(!decode_attrib_buffer(gltf, name, accessor, vertices)) {
            return {false, {}};
        }
    }

    std::vector<u32> indices;
    {
        tinygltf::Accessor accessor = gltf.accessors[prim.indices];
        if(!accessor.count || accessor.sparse.isSparse) {
            return {false, {}};
        }

        if(!indices.size()) {
            std::fill_n(std::back_inserter(indices), accessor.count, u32(0));
        } else if(indices.size() != accessor.count) {
            return {false, {}};
        }

        if(!decode_index_buffer(gltf, accessor, indices)) {
            return {false, {}};
        }
    }

    return {true, MeshData{std::move(vertices), std::move(indices)}};
}

static Result<TextureData> build_texture_data(const tinygltf::Image& image, bool as_sRGB) {
    if(image.bits != 8 && image.pixel_type != TINYGLTF_COMPONENT_TYPE_BYTE && image.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        std::cerr << "Unsupported image format (pixel type)" << std::endl;
        return {false, {}};
    }

    ImageFormat format = ImageFormat::RGBA8_UNORM;
    switch(image.component) {
        case 3:
            format = as_sRGB ? ImageFormat::RGB8_sRGB : ImageFormat::RGB8_UNORM;
        break;

        case 4:
            format = as_sRGB ? ImageFormat::RGBA8_sRGB : ImageFormat::RGBA8_UNORM;
        break;

        default:
            std::cerr << "Unsupported image format (components)" << std::endl;
            return {false, {}};
    }

    auto data = std::make_unique<u8[]>(image.image.size());
    std::copy(image.image.begin(), image.image.end(), data.get());

    return {true, TextureData{std::move(data), glm::uvec2(image.width, image.height), format}};
}


static glm::mat4 parse_node_matrix(const tinygltf::Node& node) {
    glm::vec3 translation(0.0f, 0.0f, 0.0f);
    for(u32 k = 0; k != node.translation.size(); ++k) {
        translation[k] = float(node.translation[k]);
    }

    glm::vec3 scale(1.0f, 1.0f, 1.0f);
    for(u32 k = 0; k != node.scale.size(); ++k) {
        scale[k] = float(node.scale[k]);
    }

    glm::vec4 rotation(0.0f, 0.0f, 0.0f, 1.0f);
    for(u32 k = 0; k != node.rotation.size(); ++k) {
        rotation[k] = float(node.rotation[k]);
    }

    const glm::tquat<float> q(rotation.w, rotation.x, rotation.y, rotation.z);
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(q) * glm::scale(glm::mat4(1.0f), scale);
}

static glm::mat4 base_transform() {
    return glm::mat4(1.0f);
}

static void parse_node_transforms(int node_index, const tinygltf::Model& gltf, std::unordered_map<int, glm::mat4>& node_transforms, const glm::mat4& parent_transform = base_transform()) {
    const tinygltf::Node& node = gltf.nodes[node_index];
    const glm::mat4 transform = parent_transform * parse_node_matrix(node);
    node_transforms[node_index] = transform;
    for(int child : node.children)  {
        parse_node_transforms(child, gltf, node_transforms, transform);
    }
}

static void compute_tangents(MeshData& mesh) {
    for(Vertex& vert : mesh.vertices) {
        vert.tangent_bitangent_sign = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    for(size_t i = 0; i < mesh.indices.size(); i += 3) {
        const u32 tri[] = {
            mesh.indices[i + 0],
            mesh.indices[i + 1],
            mesh.indices[i + 2]
        };

        const glm::vec3 edges[] = {
            mesh.vertices[tri[1]].position - mesh.vertices[tri[0]].position,
            mesh.vertices[tri[2]].position - mesh.vertices[tri[0]].position
        };

        const glm::vec2 uvs[] = {
            mesh.vertices[tri[0]].uv,
            mesh.vertices[tri[1]].uv,
            mesh.vertices[tri[2]].uv
        };

        const float dt[] = {
            uvs[1].y - uvs[0].y,
            uvs[2].y - uvs[0].y
        };

        const glm::vec3 tangent = -glm::normalize((edges[0] * dt[1]) - (edges[1] * dt[0]));
        mesh.vertices[tri[0]].tangent_bitangent_sign += glm::vec4(tangent, 0.0f);
        mesh.vertices[tri[1]].tangent_bitangent_sign += glm::vec4(tangent, 0.0f);
        mesh.vertices[tri[2]].tangent_bitangent_sign += glm::vec4(tangent, 0.0f);
    }

    for(Vertex& vert : mesh.vertices) {
        const glm::vec3 tangent = vert.tangent_bitangent_sign;
        vert.tangent_bitangent_sign = glm::vec4(glm::normalize(tangent), 1.0f);
    }
}


Result<std::unique_ptr<Scene>> Scene::from_gltf(const std::string& file_name) {
    const double time = program_time();
    DEFER(std::cout << file_name << " loaded in " << std::round((program_time() - time) * 100.0) / 100.0 << "s" << std::endl);

    tinygltf::TinyGLTF ctx;
    tinygltf::Model gltf;

    {
        std::string err;
        std::string warn;

        const bool is_ascii = ends_with(file_name, ".gltf");
        const bool ok = is_ascii
                ? ctx.LoadASCIIFromFile(&gltf, &err, &warn, file_name)
                : ctx.LoadBinaryFromFile(&gltf, &err, &warn, file_name);

        if(!err.empty()) {
            std::cerr << "Error while loading gltf: " << err << std::endl;
        }
        if(!warn.empty()) {
            std::cerr << "Warning while loading gltf: " << warn << std::endl;
        }

        if(!ok) {
            return {false, {}};
        }
    }

    std::cout << file_name << " parsed in " << std::round((program_time() - time) * 100.0) / 100.0 << "s" << std::endl;

    auto scene = std::make_unique<Scene>();

    std::unordered_map<int, std::shared_ptr<Texture>> textures;
    std::unordered_map<int, std::shared_ptr<Material>> materials;
    std::unordered_map<int, glm::mat4> node_transforms;
    std::vector<std::pair<int, int>> light_nodes;

    {
        std::vector<int> node_indices;
        if(gltf.defaultScene >= 0) {
            node_indices = gltf.scenes[gltf.defaultScene].nodes;
        } else {
            for(u32 i = 0; i != gltf.nodes.size(); ++i) {
                node_indices.push_back(i);
                node_transforms[i] = base_transform();
            }
        }

        for(const int node_index : node_indices) {
            parse_node_transforms(node_index, gltf, node_transforms);
        }

        for(const int node_index : node_indices) {
            const auto& node = gltf.nodes[node_index];
            if(const auto it = node.extensions.find("KHR_lights_punctual"); it != node.extensions.end()) {
                const int light_index = it->second.Get("light").Get<int>();
                if(light_index < 0 || light_index >= static_cast<int>(gltf.lights.size())) {
                    continue;
                }
                light_nodes.emplace_back(std::pair{node_index, light_index});
            }
        }
    }

    for(auto [node_index, node_transform] : node_transforms) {
        const tinygltf::Node& node = gltf.nodes[node_index];

        if(node.mesh < 0) {
            continue;
        }

        const tinygltf::Mesh& mesh = gltf.meshes[node.mesh];

        for(size_t j = 0; j != mesh.primitives.size(); ++j) {
            const tinygltf::Primitive& prim = mesh.primitives[j];

            if(prim.mode != TINYGLTF_MODE_TRIANGLES) {
                continue;
            }

            auto mesh = build_mesh_data(gltf, prim);
            if(!mesh.is_ok) {
                return {false, {}};
            }

            if(mesh.value.vertices[0].tangent_bitangent_sign == glm::vec4(0.0f)) {
                compute_tangents(mesh.value);
            }

            std::shared_ptr<Material> material;
            if(prim.material >= 0) {
                auto& mat = materials[prim.material];

                if(!mat) {
                    const auto& albedo_info = gltf.materials[prim.material].pbrMetallicRoughness.baseColorTexture;
                    const auto& normal_info = gltf.materials[prim.material].normalTexture;

                    auto load_texture = [&](auto texture_info, bool as_sRGB) -> std::shared_ptr<Texture> {
                        if(texture_info.texCoord != 0) {
                            std::cerr << "Unsupported texture coordinate channel (" << texture_info.texCoord << ")" << std::endl;
                            return nullptr;
                        }

                        if(texture_info.index < 0) {
                            return nullptr;
                        }

                        const int index = gltf.textures[texture_info.index].source;
                        if(index < 0) {
                            return nullptr;
                        }

                        auto& texture = textures[index];
                        if(!texture) {
                            if(const auto r = build_texture_data(gltf.images[index], as_sRGB); r.is_ok) {
                                texture = std::make_shared<Texture>(r.value);
                            }
                        }
                        return texture;
                    };

                    auto albedo = load_texture(albedo_info, true);
                    auto normal = load_texture(normal_info, false);

                    if(!albedo) {
                        mat = Material::empty_material();
                    } else if(!normal) {
                        mat = std::make_shared<Material>(Material::textured_material());
                        mat->set_texture(0u, albedo);
                    } else {
                        mat = std::make_shared<Material>(Material::textured_normal_mapped_material());
                        mat->set_texture(0u, albedo);
                        mat->set_texture(1u, normal);
                    }
                }

                material = mat;
            }

            auto scene_object = SceneObject(std::make_shared<StaticMesh>(mesh.value), std::move(material));
            scene_object.set_transform(node_transform);
            scene->add_object(std::move(scene_object));
        }
    }

    for(auto [node_index, light_index] : light_nodes) {
        const auto& gltf_light = gltf.lights[light_index];

        const glm::vec3 color = glm::vec3(float(gltf_light.color[0]), float(gltf_light.color[1]), float(gltf_light.color[2])) * float(gltf_light.intensity);;

        PointLight light;
        light.set_position(node_transforms[node_index][3]);
        light.set_color(color);
        if(gltf_light.range > 0.0) {
            light.set_radius(float(gltf_light.range));
        } else {
            const float intensity = glm::dot(color, glm::vec3(1.0f));
            light.set_radius(std::sqrt(intensity * 1000.0f)); // Put radius where lum < 0.1%
        }
        scene->add_light(light);
    }


    return {true, std::move(scene)};
}

}

