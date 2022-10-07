#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <graphics.h>
#include <BufferMapping.h>

namespace OM3D {

class ByteBuffer : NonCopyable {

    public:
        ByteBuffer() = default;
        ByteBuffer(ByteBuffer&&) = default;
        ByteBuffer& operator=(ByteBuffer&&) = default;

        ByteBuffer(const void* data, size_t size);
        ~ByteBuffer();

        void bind(BufferUsage usage) const;
        void bind(BufferUsage usage, u32 index) const;

        size_t byte_size() const;

        BufferMapping<byte> map_bytes(AccessType access = AccessType::ReadWrite);

    protected:
        void* map_internal(AccessType access);
        const GLHandle& handle() const;

    private:
        GLHandle _handle;
        size_t _size = 0;
};

}

#endif // BYTEBUFFER_H
