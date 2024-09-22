#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <utils.h>

#include <string_view>

namespace OM3D {

static constexpr std::string_view shader_path = "../../shaders/";
static constexpr std::string_view data_path = "../../data/";

class GLHandle : NonCopyable {
    public:
        GLHandle() = default;

        explicit GLHandle(u32 handle) : _handle(handle) {
        }

        GLHandle(GLHandle&& other) {
            swap(other);
        }

        GLHandle& operator=(GLHandle&& other) {
            swap(other);
            return *this;
        }

        void swap(GLHandle& other) {
            std::swap(_handle, other._handle);
        }

        u32 get() const {
            return _handle;
        }

        bool is_valid() const {
            return _handle;
        }

    private:
        u32 _handle = 0;
};

enum class BufferUsage {
    Attribute,
    Index,
    Uniform,
    Storage,
};

enum class AccessType {
    WriteOnly,
    ReadOnly,
    ReadWrite
};

u32 buffer_usage_to_gl(BufferUsage usage);
u32 access_type_to_gl(AccessType access);

u32 align_up_to(u32 val, u32 up_to);

void init_graphics();

bool bindless_enabled();

void audit_bindings();

}

#endif // GRAPHICS_H
