
#include "BufferMapping.h"

#include <glad/gl.h>

namespace OM3D {

BufferMappingBase::BufferMappingBase(BufferMappingBase&& other) {
    swap(other);
}

BufferMappingBase& BufferMappingBase::operator=(BufferMappingBase&& other) {
    swap(other);
    return *this;
}

BufferMappingBase::~BufferMappingBase() {
    if(_handle.is_valid()) {
        glUnmapNamedBuffer(_handle.get());
    }
}

void BufferMappingBase::swap(BufferMappingBase& other) {
    std::swap(_handle, other._handle);
    std::swap(_byte_size, other._byte_size);
    std::swap(_data, other._data);
}

}

