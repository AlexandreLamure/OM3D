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

void ByteBuffer::bind(BufferUsage usage, u32 index) const {
    ALWAYS_ASSERT(usage == BufferUsage::Uniform, "index bind is only available for uniform buffers");
    glBindBufferBase(GL_UNIFORM_BUFFER, index, _handle.get());
}

size_t ByteBuffer::byte_size() const {
    return _size;
}

BufferMapping<u8> ByteBuffer::map_bytes() {
    return BufferMapping<u8>(map_internal(), byte_size(), handle());
}

void* ByteBuffer::map_internal() {
    return glMapNamedBuffer(_handle.get(), GL_READ_WRITE);
}

const GLHandle& ByteBuffer::handle() const {
    return _handle;
}
