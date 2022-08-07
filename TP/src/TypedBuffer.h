#ifndef TYPEDBUFFER_H
#define TYPEDBUFFER_H

#include <ByteBuffer.h>

#include <vector>

template<typename T>
class TypedBuffer : public ByteBuffer {
    public:
        TypedBuffer() = default;

        TypedBuffer(const std::vector<T>& data) : TypedBuffer(data.data(), data.size()) {
        }

        TypedBuffer(const T* data, size_t count) : ByteBuffer(data, count * sizeof(T)) {
        }

        size_t element_count() const {
            DEBUG_ASSERT(byte_size() % sizeof(T) == 0);
            return byte_size() / sizeof(T);
        }
};

#endif // TYPEDBUFFER_H
