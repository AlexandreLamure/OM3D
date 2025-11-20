#include <algorithm>
#include <glad/gl.h>

#include "Material.h"

namespace OM3D
{

    Material::Material()
    {}

    void Material::set_program(std::shared_ptr<Program> prog)
    {
        _program = std::move(prog);
    }

    void Material::set_blend_mode(BlendMode blend)
    {
        _blend_mode = blend;
    }

    DepthTestMode Material::get_depth_test_mode() const
    {
        return _depth_test_mode;
    }

    void Material::set_depth_test_mode(DepthTestMode depth)
    {
        _depth_test_mode = depth;
    }

    void Material::set_double_sided(bool doubleSided)
    {
        _doubleSided = doubleSided;
    }

    void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex)
    {
        if (const auto it =
                std::find_if(_textures.begin(), _textures.end(),
                             [&](const auto &t) { return t.second == tex; });
            it != _textures.end())
        {
            it->second = std::move(tex);
        }
        else
        {
            _textures.emplace_back(slot, std::move(tex));
        }
    }

    bool Material::is_opaque() const
    {
        return _blend_mode == BlendMode::None;
    }

    void Material::set_stored_uniform(u32 name_hash, UniformValue value)
    {
        for (auto &[h, v] : _uniforms)
        {
            if (h == name_hash)
            {
                v = value;
                return;
            }
        }
        _uniforms.emplace_back(name_hash, std::move(value));
    }

    void Material::bind(const bool backface_culling) const
    {
        switch (_blend_mode)
        {
        case BlendMode::None:
            glDisable(GL_BLEND);

            if (backface_culling)
            {
                // Enable back_face culling when the object is not transparent
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
            }
            break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (backface_culling)
            {
                // Disable back_face culling when the object is transparent
                glDisable(GL_CULL_FACE);
            }
            break;
        }

        switch (_depth_test_mode)
        {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
            break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using standard-Z
            glDepthFunc(GL_GEQUAL);
            break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
            break;
        }

        for (const auto &texture : _textures)
        {
            texture.second->bind(texture.first);
        }

        for (const auto &[h, v] : _uniforms)
        {
            _program->set_uniform(h, v);
        }
        _program->bind();
    }

    Material Material::textured_pbr_material(bool alpha_test)
    {
        Material material;
        std::vector<std::string> defines;
        if (alpha_test)
        {
            defines.emplace_back("ALPHA_TEST");
        }

        material._program =
            Program::from_files("lit.frag", "basic.vert", defines);

        material.set_texture(0u, default_white_texture());
        material.set_texture(1u, default_normal_texture());
        material.set_texture(2u, default_metal_rough_texture());
        material.set_texture(3u, default_black_texture());

        return material;
    }

} // namespace OM3D
