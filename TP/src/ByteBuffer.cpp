#include "ByteBuffer.h"

#include <glad/glad.h>

#include <iostream>

namespace OM3D {

static GLuint mapping_to_gl(MappingType mapping) {
    switch(mapping) {
        case MappingType::WriteOnly:
            return GL_WRITE_ONLY;

        case MappingType::ReadOnly:
            return GL_READ_ONLY;

        case MappingType::ReadWrite:
            return GL_READ_WRITE;
    }

    FATAL("Unknown mapping value");
}

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

BufferMapping<u8> ByteBuffer::map_bytes(MappingType mapping) {
    return BufferMapping<u8>(map_internal(mapping), byte_size(), handle());
}

void* ByteBuffer::map_internal(MappingType mapping) {
    DEBUG_ASSERT(_handle.is_valid() && _size);
    return glMapNamedBuffer(_handle.get(), mapping_to_gl(mapping));;
}

const GLHandle& ByteBuffer::handle() const {
    return _handle;
}

}
