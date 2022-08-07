#include "ByteBuffer.h"

#include <glad/glad.h>


static GLuint create_buffer_handle() {
    GLuint handle = 0;
    glGenBuffers(1, &handle);
    return handle;
}

ByteBuffer::ByteBuffer(const void* data, size_t size) : _handle(create_buffer_handle()), _size(size) {
    glNamedBufferData(_handle.get(), size, data, GL_STATIC_DRAW);
}

ByteBuffer::~ByteBuffer() {
    if(auto handle = _handle.get()) {
        glDeleteBuffers(1, &handle);
    }
}

void ByteBuffer::bind(BufferUsage usage) const {
    glBindBuffer(buffer_usage_to_gl(usage), _handle.get());
}
