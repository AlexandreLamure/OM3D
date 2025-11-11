#include "Framebuffer.h"

#include <glm/vec4.hpp>

#include <glad/gl.h>

namespace OM3D {

struct WriteMask {
    GLboolean color[4] = {};
    GLboolean depth = {};

    static WriteMask get() {
        WriteMask mask = {};
        glGetBooleanv(GL_DEPTH_WRITEMASK, &mask.depth);
        glGetBooleanv(GL_COLOR_WRITEMASK, mask.color);
        return mask;
    }

    static void set(WriteMask mask) {
        glColorMask(mask.color[0], mask.color[1], mask.color[2], mask.color[3]);
        glDepthMask(mask.depth);
    }

    static void set_all() {
        glColorMask(true, true, true, true);
        glDepthMask(true);
    }
};

static GLuint create_framebuffer_handle() {
    GLuint handle = 0;
    glCreateFramebuffers(1, &handle);
    return handle;
}



Framebuffer::Framebuffer() {
}

Framebuffer::Framebuffer(Texture* depth) : Framebuffer(depth, nullptr, 0) {
}

Framebuffer::Framebuffer(Texture* depth, Texture** colors, size_t count) : _handle(create_framebuffer_handle()) {
    if(depth) {
        glNamedFramebufferTexture(_handle.get(), GL_DEPTH_ATTACHMENT, depth->_handle.get(), 0);
        _size = depth->size();
    }

    ALWAYS_ASSERT(count <= 8, "Too many render targets");

    for(size_t i = 0; i != count; ++i) {
        DEBUG_ASSERT(colors[i]);
        glNamedFramebufferTexture(_handle.get(), GLenum(GL_COLOR_ATTACHMENT0 + i), colors[i]->_handle.get(), 0);
        _size = colors[i]->size();
    }

    const GLenum draw_buffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5,
        GL_COLOR_ATTACHMENT6,
        GL_COLOR_ATTACHMENT7
    };
    glNamedFramebufferDrawBuffers(_handle.get(), GLsizei(count), draw_buffers);

    ALWAYS_ASSERT(glCheckNamedFramebufferStatus(_handle.get(), GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid framebuffer");
}

Framebuffer::~Framebuffer() {
    if(u32 handle = _handle.get()) {
        glDeleteFramebuffers(1, &handle);
    }
}


void Framebuffer::bind(bool clear_depth, bool clear_color) const {
    glBindFramebuffer(GL_FRAMEBUFFER, _handle.get());
    glViewport(0, 0, _size.x, _size.y);

    GLenum clear_mask = 0;
    if(clear_color) {
        clear_mask |= GL_COLOR_BUFFER_BIT;
    }
    if(clear_depth) {
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }

    if(clear_mask) {
        const WriteMask mask = WriteMask::get();
        DEFER(WriteMask::set(mask));
        WriteMask::set_all();

        glClear(clear_mask);
    }
}

void Framebuffer::blit(bool depth) const {
    const WriteMask mask = WriteMask::get();
    DEFER(WriteMask::set(mask));
    WriteMask::set_all();

    i32 binding = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
    ALWAYS_ASSERT(u32(binding) != _handle.get(), "Framebuffer is bound");

    int viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBlitNamedFramebuffer(
        _handle.get(), binding,
        0, 0, _size.x, _size.y,
        0, 0, viewport[2], viewport[3],
        GL_COLOR_BUFFER_BIT | (depth ? GL_DEPTH_BUFFER_BIT : 0), GL_NEAREST);
}

const glm::uvec2& Framebuffer::size() const {
    return _size;
}

}
