#include "Material.h"

#include <glad/glad.h>

namespace OM3D {

Material::Material() {
}

void Material::bind() const {
    switch(_blend_mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
        break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    }

    switch(_depth_test_mode) {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
        break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
        break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
        break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
        break;
    }

    for(const auto& texture : _textures) {
        texture.second->bind(texture.first);
    }
    _program->bind();
}

std::shared_ptr<Material> Material::empty_material() {
    static std::shared_ptr<Material> material;
    if(!material) {
        material = std::make_shared<Material>();
        material->_program = Program::from_files("lit.frag", "basic.vert");
    }
    return material;
}

}
