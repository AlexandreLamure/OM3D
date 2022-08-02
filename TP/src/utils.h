#ifndef UTILS_H
#define UTILS_H

#include <defines.h>

#include <cstdint>
#include <utility>
#include <string_view>
#include <string>

#define FWD(var) std::forward<decltype(var)>(var)
#define DEFER(expr) auto CREATE_UNIQUE_NAME_WITH_PREFIX(defer) = OnExit([&]() { expr; })


using u32 = uint32_t;
using u16 = uint16_t;

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
Result<std::string> read_file(const std::string& file_name);


#endif // UTILS_H
