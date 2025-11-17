#ifndef MATERIAL_H
#define MATERIAL_H

#include <Program.h>
#include <Texture.h>

#include <memory>
#include <vector>

namespace OM3D {

enum class BlendMode {
    None,
    Alpha,
};

enum class DepthTestMode {
    Standard,
    Reversed,
    Equal,
    None
};

class Material {

    public:
        Material();

        void set_program(std::shared_ptr<Program> prog);
        void set_blend_mode(BlendMode blend);
        void set_depth_test_mode(DepthTestMode depth);
        void set_texture(u32 slot, std::shared_ptr<Texture> tex);

        bool is_opaque() const;

        template<typename T>
        void set_uniform(u32 name_hash, T&& args) {
            set_uniform_inner(name_hash, FWD(args));
        }

        template<typename T>
        void set_uniform(std::string_view name, T&& args) {
            set_uniform_inner(str_hash(name), FWD(args));
        }


        void bind() const;

        static Material textured_pbr_material(bool alpha_test = false);

    private:
        void set_uniform_inner(u32 name_hash, UniformValue value);

        std::shared_ptr<Program> _program;
        std::vector<std::pair<u32, std::shared_ptr<Texture>>> _textures;
        std::vector<std::pair<u32, UniformValue>> _uniforms;

        BlendMode _blend_mode = BlendMode::None;
        DepthTestMode _depth_test_mode = DepthTestMode::Standard;

};

}

#endif // MATERIAL_H
