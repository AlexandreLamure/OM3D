#include "Material.h"

#include <algorithm>
#include <glad/gl.h>

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

    void Material::set_depth_test_mode(DepthTestMode depth)
    {
        _depth_test_mode = depth;
    }

    void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex)
    {
        if (const auto it =
                std::find_if(_textures.begin(), _textures.end(),
                             [&](const auto& t) { return t.second == tex; });
            it != _textures.end())
        {
            it->second = std::move(tex);
        }
        else
        {
            _textures.emplace_back(slot, std::move(tex));
        }
    }

    void Material::bind() const
    {
        switch (_blend_mode)
        {
        case BlendMode::None:
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            // glFrontFace(GL_CW);
            break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_CULL_FACE);
            break;

        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }

        switch (_depth_test_mode)
        {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
            break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            glDepthMask(GL_TRUE);
            break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
            glDepthMask(GL_TRUE);
            break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
            break;
        }

        for (const auto& texture : _textures)
        {
            texture.second->bind(texture.first);
        }
        _program->bind();
    }

    std::shared_ptr<Material> Material::empty_material()
    {
        static std::weak_ptr<Material> weak_material;
        auto material = weak_material.lock();
        if (!material)
        {
            material = std::make_shared<Material>();
            material->_program =
                Program::from_files("g_buffer.frag", "basic.vert");
            weak_material = material;
        }
        return material;
    }

    Material Material::textured_material()
    {
        Material material;
        material._program =
            Program::from_files("g_buffer.frag", "basic.vert", { "TEXTURED" });
        return material;
    }

    Material Material::textured_normal_mapped_material()
    {
        Material material;
        material._program = Program::from_files(
            "g_buffer.frag", "basic.vert",
            std::array<std::string, 2>{ "TEXTURED", "NORMAL_MAPPED" });
        return material;
    }

} // namespace OM3D
