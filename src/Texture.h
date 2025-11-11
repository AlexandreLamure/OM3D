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

class Texture {

    public:
        Texture() = default;
        Texture(Texture&&) = default;
        Texture& operator=(Texture&&) = default;

        ~Texture();

        Texture(const TextureData& data);
        Texture(const glm::uvec2 &size, ImageFormat format);

        void bind(u32 index) const;
        void bind_as_image(u32 index, AccessType access);

        u64 bindless_handle() const;

        glm::uvec2 size() const;


        static u32 mip_levels(glm::uvec2 size);

    private:
        friend class Framebuffer;

        GLHandle _handle;
        glm::uvec2 _size = {};
        u64 _bindless = {};
        ImageFormat _format;
};

}

#endif // TEXTURE_H
