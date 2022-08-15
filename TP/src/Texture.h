#ifndef TEXTURE_H
#define TEXTURE_H

#include <graphics.h>

#include <glm/vec2.hpp>

#include <vector>
#include <memory>

struct TextureData {
    std::unique_ptr<u8[]> data;
    glm::uvec2 size = {};
    u8 channels = 0;

    static Result<TextureData> from_file(const std::string& file_name);
};


class Texture {

    public:
        Texture() = default;
        Texture(Texture&&) = default;
        Texture& operator=(Texture&&) = default;

        ~Texture();

        Texture(const TextureData& data);

        void bind(u32 index) const;

        static Texture from_file(const std::string& file);

    private:
        GLHandle _handle;
        glm::uvec2 _size = {};
};

#endif // TEXTURE_H
