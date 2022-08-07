#ifndef UTILS_H
#define UTILS_H

#include <defines.h>

#include <cstdint>
#include <utility>
#include <string>

#define FWD(var) std::forward<decltype(var)>(var)
#define DEFER(expr) auto CREATE_UNIQUE_NAME_WITH_PREFIX(defer) = OnExit([&]() { expr; })
#define FATAL(msg) fatal((msg), __FILE__, __LINE__)
#define ALWAYS_ASSERT(cond, msg) do { if(!(cond)) { FATAL(msg); } } while(false)


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

struct NonCopyable {
    inline constexpr NonCopyable() {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonMovable : NonCopyable {
    inline constexpr NonMovable() {}
    NonMovable(const NonMovable&) = delete;
    NonMovable& operator=(const NonMovable&) = delete;

    NonMovable(NonMovable&&) = delete;
};


template<typename T>
struct [[nodiscard]] Result {
    bool is_ok;
    T value;
};

template<>
struct [[nodiscard]] Result<void> {
    bool is_ok;
};

template<typename T>
class OnExit {
    public:
        inline OnExit(T&& t) : _ex(FWD(t)) {
        }

        inline OnExit(OnExit&& other) : _ex(std::move(other._ex)) {
        }

        inline ~OnExit() {
            _ex();
        }

    private:
        T _ex;
};


void break_in_debugger();
[[noreturn]] void fatal(const char* msg, const char* file = nullptr, int line = 0);

double program_time();
Result<std::string> read_text_file(const std::string& file_name);


#endif // UTILS_H
