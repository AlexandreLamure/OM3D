#include "Texture.h"

#include <glad/gl.h>

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


Result<CubeMapData> CubeMapData::from_files(const std::string& prefix, const std::string& suffix) {
    CubeMapData data;

    const std::string_view face_names[] = {
        "_px", "_nx",
        "_py", "_ny",
        "_pz", "_nz"
    };
    for(size_t i = 0; i != 6; ++i) {
        auto face = TextureData::from_file(prefix + std::string(face_names[i]) + suffix);
        if(!face.is_ok) {
            return {false, {}};
        }
        data.faces[i] = std::move(face.value);
    }

    return {true, std::move(data)};
}




static GLuint create_texture_handle(GLenum type) {
    GLuint handle = 0;
    glCreateTextures(type, 1, &handle);
    return handle;
}

Texture::Texture(const TextureData& data) :
    _handle(create_texture_handle(GL_TEXTURE_2D)),
    _size(data.size),
    _format(data.format),
    _texture_type(GL_TEXTURE_2D) {

    const ImageFormatGL gl_format = image_format_to_gl(_format);
    glTextureStorage2D(_handle.get(), mip_levels(_size), gl_format.internal_format, _size.x, _size.y);
    glTextureSubImage2D(_handle.get(), 0, 0, 0, _size.x, _size.y, gl_format.format, gl_format.component_type, data.data.get());

    // glGenerateTextureMipmap(_handle.get());

    if(bindless_enabled()) {
        _bindless = glGetTextureHandleARB(_handle.get());
        glMakeTextureHandleResidentARB(_bindless);
    }
}

Texture::Texture(const CubeMapData& data) :
    _handle(create_texture_handle(GL_TEXTURE_CUBE_MAP)),
    _size(data.faces[0].size),
    _format(data.faces[0].format),
    _texture_type(GL_TEXTURE_CUBE_MAP) {

    for(const TextureData& face : data.faces) {
        ALWAYS_ASSERT(face.format == _format, "Invalid cube map face format");
        ALWAYS_ASSERT(face.size == _size, "Invalid cube map face size");
    }

    const ImageFormatGL gl_format = image_format_to_gl(_format);
    glTextureStorage2D(_handle.get(), mip_levels(_size), gl_format.internal_format, _size.x, _size.y);

    for(u32 i = 0; i != 6; ++i) {
        glTextureSubImage3D(
            _handle.get(), 0,
            0, 0, i,
            _size.x, _size.y, 1,
            gl_format.format, gl_format.component_type, data.faces[i].data.get());
    }

    glGenerateTextureMipmap(_handle.get());

    if(bindless_enabled()) {
        _bindless = glGetTextureHandleARB(_handle.get());
        glMakeTextureHandleResidentARB(_bindless);
    }
}


Texture::Texture(const glm::uvec2 &size, ImageFormat format) :
    _handle(create_texture_handle(GL_TEXTURE_2D)),
    _size(size),
    _format(format),
    _texture_type(GL_TEXTURE_2D) {

    const ImageFormatGL gl_format = image_format_to_gl(_format);
    glTextureStorage2D(_handle.get(), 1, gl_format.internal_format, _size.x, _size.y);

    if(bindless_enabled()) {
        _bindless = glGetTextureHandleARB(_handle.get());
        glMakeTextureHandleResidentARB(_bindless);
    }
}

Texture::~Texture() {
    if(auto handle = _handle.get()) {
        glDeleteTextures(1, &handle);
    }
}

bool Texture::is_null() const {
    return !_handle.is_valid();
}

void Texture::bind(u32 index) const {
    glBindTextureUnit(index, _handle.get());
}

void Texture::bind_as_image(u32 index, AccessType access) {
    glBindImageTexture(index, _handle.get(), 0, false, 0, access_type_to_gl(access), image_format_to_gl(_format).internal_format);
}

u64 Texture::bindless_handle() const {
    return _bindless;
}

u32 Texture::texture_type() const {
    return _texture_type;
}

glm::uvec2 Texture::size() const {
    return _size;
}

// Return number of mip levels needed
u32 Texture::mip_levels(glm::uvec2 size) {
    const float side = float(std::max(size.x, size.y));
    return 1 + u32(std::floor(std::log2(side)));
}

}
