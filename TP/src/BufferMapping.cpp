
#include "BufferMapping.h"

#include <glad/glad.h>

namespace OM3D {

BufferMappingBase::~BufferMappingBase() {
    if(_handle.is_valid()) {
        glUnmapNamedBuffer(_handle.get());
    }
}

}

