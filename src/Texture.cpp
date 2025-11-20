#include "Texture.h"

#include <glad/gl.h>

#include "Program.h"

#define STB_IMAGE_IMPLEMENTATION
#include <algorithm>
#include <cmath>
#include <stb/stb_image.h>

namespace OM3D
{

    Result<TextureData> TextureData::from_file(const std::string &file)
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        u8 *img = stbi_load(file.c_str(), &width, &height, &channels, 4);
        DEFER(stbi_image_free(img));
        if (!img || width <= 0 || height <= 0 || channels <= 0)
        {
            return { false, {} };
        }

        const size_t bytes = width * height * 4;

        TextureData data;
        data.size = glm::uvec2(width, height);
        data.format = ImageFormat::RGBA8_UNORM;
        data.data = std::make_unique<u8[]>(bytes);
        std::copy_n(img, bytes, data.data.get());

        return { true, std::move(data) };
    }

    static GLuint create_texture_handle(GLenum type)
    {
        GLuint handle = 0;
        glCreateTextures(type, 1, &handle);
        return handle;
    }

    Texture::Texture(const TextureData &data)
        : _handle(create_texture_handle(GL_TEXTURE_2D))
        , _size(data.size)
        , _format(data.format)
        , _texture_type(GL_TEXTURE_2D)
    {
        const ImageFormatGL gl_format = image_format_to_gl(_format);
        glTextureStorage2D(_handle.get(), mip_levels(_size),
                           gl_format.internal_format, _size.x, _size.y);
        glTextureSubImage2D(_handle.get(), 0, 0, 0, _size.x, _size.y,
                            gl_format.format, gl_format.component_type,
                            data.data.get());

        glGenerateTextureMipmap(_handle.get());

        if (bindless_enabled())
        {
            _bindless = glGetTextureHandleARB(_handle.get());
            glMakeTextureHandleResidentARB(_bindless);
        }
    }

    Texture::Texture(const glm::uvec2 &size, ImageFormat format, WrapMode wrap)
        : _handle(create_texture_handle(GL_TEXTURE_2D))
        , _size(size)
        , _format(format)
        , _texture_type(GL_TEXTURE_2D)
    {
        const ImageFormatGL gl_format = image_format_to_gl(_format);
        glTextureStorage2D(_handle.get(), 1, gl_format.internal_format, _size.x,
                           _size.y);

        const GLenum gl_wrap =
            (wrap == WrapMode::Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTextureParameteri(_handle.get(), GL_TEXTURE_WRAP_R, gl_wrap);
        glTextureParameteri(_handle.get(), GL_TEXTURE_WRAP_S, gl_wrap);
        glTextureParameteri(_handle.get(), GL_TEXTURE_WRAP_T, gl_wrap);

        if (bindless_enabled())
        {
            _bindless = glGetTextureHandleARB(_handle.get());
            glMakeTextureHandleResidentARB(_bindless);
        }
    }

    Texture Texture::empty_cubemap(u32 size, ImageFormat format, u32 mipmaps)
    {
        Texture cube;
        {
            cube._handle = GLHandle(create_texture_handle(GL_TEXTURE_CUBE_MAP));
            cube._texture_type = GL_TEXTURE_CUBE_MAP;
            cube._size = glm::uvec2(size);
            cube._format = format;
        }

        const ImageFormatGL gl_format = image_format_to_gl(cube._format);
        glTextureStorage2D(
            cube._handle.get(), std::min(mipmaps, mip_levels(cube._size)),
            gl_format.internal_format, cube._size.x, cube._size.y);

        if (bindless_enabled())
        {
            cube._bindless = glGetTextureHandleARB(cube._handle.get());
            glMakeTextureHandleResidentARB(cube._bindless);
        }

        return cube;
    }

    Texture Texture::cubemap_from_equirec(const Texture &equirec)
    {
        const size_t px = equirec.size().x * equirec.size().y;
        const size_t per_face = px / 6;

        u32 face_size = 8;
        while (face_size * face_size < per_face)
        {
            face_size *= 2;
        }

        Texture cube =
            empty_cubemap(face_size, ImageFormat::RGBA16_FLOAT, 9999);

        {
            equirec.bind(0);
            cube.bind_as_image(1, AccessType::WriteOnly);

            Program::from_file("equirec_cube.comp")->bind();
            glDispatchCompute(face_size / 8, face_size / 8, 6);

            glGenerateTextureMipmap(cube._handle.get());
        }

        return cube;
    }

    Texture::~Texture()
    {
        if (auto handle = _handle.get())
        {
            glDeleteTextures(1, &handle);
        }
    }

    bool Texture::is_null() const
    {
        return !_handle.is_valid();
    }

    void Texture::bind(u32 index) const
    {
        glBindTextureUnit(index, _handle.get());
    }

    void Texture::bind_as_image(u32 index, AccessType access)
    {
        glBindImageTexture(index, _handle.get(), 0,
                           texture_type() != GL_TEXTURE_2D, 0,
                           access_type_to_gl(access),
                           image_format_to_gl(_format).internal_format);
    }

    u64 Texture::bindless_handle() const
    {
        return _bindless;
    }

    u32 Texture::texture_type() const
    {
        return _texture_type;
    }

    glm::uvec2 Texture::size() const
    {
        return _size;
    }

    // Return number of mip levels needed
    u32 Texture::mip_levels(glm::uvec2 size)
    {
        const float side = float(std::max(size.x, size.y));
        return 1 + u32(std::floor(std::log2(side)));
    }

} // namespace OM3D
