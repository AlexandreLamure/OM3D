
#include "BufferMapping.h"

#include <glad/glad.h>

BufferMappingBase::~BufferMappingBase() {
    if(_handle.is_valid()) {
        glUnmapNamedBuffer(_handle.get());
    }
}

