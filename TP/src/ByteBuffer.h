#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <graphics.h>

class ByteBuffer : NonCopyable {

    public:
        ByteBuffer() = default;
        ByteBuffer(ByteBuffer&&) = default;
        ByteBuffer& operator=(ByteBuffer&&) = default;

        ByteBuffer(const void* data, size_t size);
        ~ByteBuffer();

        void bind(BufferUsage usage) const;

        size_t byte_size() const {
            return _size;
        }

    private:
        GLHandle _handle;
        size_t _size = 0;
};

#endif // BYTEBUFFER_H
