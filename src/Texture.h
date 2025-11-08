#ifndef TEXTURE_H
#define TEXTURE_H

#include <graphics.h>
#include <ImageFormat.h>

#include <glm/vec2.hpp>

#include <vector>
#include <memory>


namespace OM3D {

struct TextureData {
    std::unique_ptr<u8[]> data;
    glm::uvec2 size = {};
    ImageFormat format;

    static Result<TextureData> from_file(const std::string& file_name);
};



enum class WrapMode {
    Repeat,
    Clamp,
};

class Texture {

    public:
        Texture() = default;

        Texture(Texture&&) = default;
        Texture& operator=(Texture&&) = default;

        ~Texture();

        Texture(const TextureData& data);

        Texture(const glm::uvec2 &size, ImageFormat format, WrapMode wrap = WrapMode::Repeat);

        static Texture empty_cubemap(u32 size, ImageFormat format, u32 mipmaps = 1);
        static Texture cubemap_from_equirec(const Texture& equirec);

        bool is_null() const;

        void bind(u32 index) const;
        void bind_as_image(u32 index, AccessType access);

        u64 bindless_handle() const;

        u32 texture_type() const;

        glm::uvec2 size() const;

        static u32 mip_levels(glm::uvec2 size);

    private:
        friend class Framebuffer;
        friend class Program;

        GLHandle _handle;
        glm::uvec2 _size = {};
        u64 _bindless = {};
        ImageFormat _format;

        u32 _texture_type = {};
};

}

#endif // TEXTURE_H
