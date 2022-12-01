#include "Texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cmath>
#include <algorithm>

namespace OM3D {

Result<TextureData> TextureData::from_file(const std::string& file) {
    int width = 0;
    int height = 0;
    int channels = 0;
    u8* img = stbi_load(file.c_str(), &width, &height, &channels, 4);
    DEFER(stbi_image_free(img));
    if(!img || width <= 0 || height <= 0 || channels <= 0) {
        return {false, {}};
    }

    const size_t bytes = width * height * 4;

    TextureData data;
    data.size = glm::uvec2(width, height);
    data.format = ImageFormat::RGBA8_UNORM;
    data.data = std::make_unique<u8[]>(bytes);
    std::copy_n(img, bytes, data.data.get());

    return {true, std::move(data)};
}



static GLuint create_texture_handle() {
    GLuint handle = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    return handle;
}

Texture::Texture(const TextureData& data) :
    _handle(create_texture_handle()),
    _size(data.size),
    _format(data.format) {

    const ImageFormatGL gl_format = image_format_to_gl(_format);
    glTextureStorage2D(_handle.get(), mip_levels(_size), gl_format.internal_format, _size.x, _size.y);
    glTextureSubImage2D(_handle.get(), 0, 0, 0, _size.x, _size.y, gl_format.format, gl_format.component_type, data.data.get());
    glGenerateTextureMipmap(_handle.get());
}

Texture::Texture(const glm::uvec2 &size, ImageFormat format) :
    _handle(create_texture_handle()),
    _size(size),
    _format(format) {

    const ImageFormatGL gl_format = image_format_to_gl(_format);
    glTextureStorage2D(_handle.get(), 1, gl_format.internal_format, _size.x, _size.y);
}

Texture::~Texture() {
    if(auto handle = _handle.get()) {
        glDeleteTextures(1, &handle);
    }
}

void Texture::bind(u32 index) const {
    glBindTextureUnit(index, _handle.get());
}

void Texture::bind_as_image(u32 index, AccessType access) {
    glBindImageTexture(index, _handle.get(), 0, false, 0, access_type_to_gl(access), image_format_to_gl(_format).internal_format);
}

const glm::uvec2& Texture::size() const {
    return _size;
}

// Return number of mip levels needed
u32 Texture::mip_levels(glm::uvec2 size) {
    const float side = float(std::max(size.x, size.y));
    return 1 + u32(std::floor(std::log2(side)));
}

}
