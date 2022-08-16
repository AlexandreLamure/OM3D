#include "Texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cmath>

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
    data.channels = 4;
    data.data = std::make_unique<u8[]>(bytes);
    std::copy_n(img, bytes, data.data.get());

    return {true, std::move(data)};
}

static GLenum channels_to_gl_internal_format(u8 channels) {
    const GLenum gl[] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
    return gl[channels - 1];
}

static GLenum channels_to_gl_format(u8 channels) {
    const GLenum gl[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
    return gl[channels - 1];
}

static GLuint create_texture_handle() {
    GLuint handle = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    return handle;
}

Texture::Texture(const TextureData& data) :
    _handle(create_texture_handle()),
    _size(data.size) {

    glTextureStorage2D(_handle.get(), 1, channels_to_gl_internal_format(data.channels), _size.x, _size.y);
    glTextureSubImage2D(_handle.get(), 0, 0, 0, _size.x, _size.y, channels_to_gl_format(data.channels), GL_UNSIGNED_BYTE, data.data.get());
    glGenerateTextureMipmap(_handle.get());
}

Texture::~Texture() {
    if(auto handle = _handle.get()) {
        glDeleteTextures(1, &handle);
    }
}

void Texture::bind(u32 index) const {
    glBindTextureUnit(index, _handle.get());
}

u32 Texture::mip_levels(glm::uvec2 size) {
    const float side = float(std::max(size.x, size.y));
    return 1 + u32(std::floor(std::log2(side)));
}

