#include "ByteBuffer.h"

#include <glad/gl.h>

#include <iostream>

namespace OM3D {

static GLuint create_buffer_handle() {
    GLuint handle = 0;
    glCreateBuffers(1, &handle);
    return handle;
}

ByteBuffer::ByteBuffer(const void* data, size_t size) : _handle(create_buffer_handle()), _size(size) {
    ALWAYS_ASSERT(_size, "Buffer size can not be 0");
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
    ALWAYS_ASSERT(usage == BufferUsage::Uniform || usage == BufferUsage::Storage, "Index bind is only available for uniform and storage buffers");
    glBindBufferBase(buffer_usage_to_gl(usage), index, _handle.get());
}

size_t ByteBuffer::byte_size() const {
    return _size;
}

BufferMapping<byte> ByteBuffer::map_bytes(AccessType access) {
    return BufferMapping<byte>(map_internal(access), byte_size(), handle());
}

void* ByteBuffer::map_internal(AccessType access) {
    DEBUG_ASSERT(_handle.is_valid() && _size);
    return glMapNamedBuffer(_handle.get(), access_type_to_gl(access));
}

const GLHandle& ByteBuffer::handle() const {
    return _handle;
}

}
