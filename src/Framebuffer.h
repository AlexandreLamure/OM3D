#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <Texture.h>

#include <array>

namespace OM3D {

class Framebuffer : NonCopyable {
    public:
        template<size_t N>
        Framebuffer(Texture* depth, std::array<Texture*, N> colors) : Framebuffer(depth, colors.data(), colors.size()) {
        }


        Framebuffer();
        Framebuffer(Texture* depth);

        Framebuffer(Framebuffer&&) = default;
        Framebuffer& operator=(Framebuffer&&) = default;

        ~Framebuffer();

        void bind(bool clear_depth, bool clear_color) const;
        void blit(bool depth = false) const;

        const glm::uvec2& size() const;

    private:
        Framebuffer(Texture* depth, Texture** colors, size_t count);

        GLHandle _handle;
        glm::uvec2 _size = {};
};

}

#endif // FRAMEBUFFER_H
