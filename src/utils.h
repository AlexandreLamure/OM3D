#ifndef UTILS_H
#define UTILS_H

#include <defines.h>

#include <cstdint>
#include <utility>
#include <string>
#include <array>

#define FWD(var) std::forward<decltype(var)>(var)
#define HASH(str) ([] { static constexpr u32 result = ::OM3D::str_hash(str); return result; }())

// Execute expr at scope exit
#define DEFER(expr) auto CREATE_UNIQUE_NAME_WITH_PREFIX(defer) = ::OM3D::ScopeGuard([&] { expr; })
// Print message and terminate immediatly
#define FATAL(msg) ::OM3D::fatal((msg), __FILE__, __LINE__)
// Assert in debug and release
#define ALWAYS_ASSERT(cond, msg) do { if(!(cond)) { FATAL(msg); } } while(false)

// Assert in debug only
#ifdef NDEBUG
#define DEBUG_ASSERT(cond) do { /*(void)(cond);*/ } while(false)
#else
#define OM3D_DEBUG
#define DEBUG_ASSERT(cond) ALWAYS_ASSERT(cond, "Assert failed: " #cond)
#endif

namespace OM3D {

#ifdef OS_WIN
bool running_in_debugger();
#endif

void break_in_debugger();
[[noreturn]] void fatal(const char* msg, const char* file = nullptr, int line = 0);

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = std::byte;

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

    const T& value_or(const T& default_val) const {
        return is_ok ? value : default_val;
    }
};

template<>
struct [[nodiscard]] Result<void> {
    bool is_ok;
};

template<typename T>
class ScopeGuard {
    public:
        inline ScopeGuard(T&& t) : _ex(FWD(t)) {
        }

        inline ScopeGuard(ScopeGuard&& other) : _ex(std::move(other._ex)) {
        }

        inline ~ScopeGuard() {
            _ex();
        }

    private:
        T _ex;
};

// Similar to std::span
template<typename T>
class Span {
    template<typename U>
    static constexpr bool is_compat = std::is_constructible_v<T*, U>;

    template<typename C>
    using data_type = decltype(std::declval<C>().data());

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        inline constexpr Span() = default;
        inline constexpr Span(const Span&) = default;
        inline constexpr Span& operator=(const Span&) = default;

        inline constexpr Span(std::nullptr_t) {}
        inline constexpr Span(T& t) : _data(&t), _size(1) {}
        inline constexpr Span(T* data, size_t size) : _data(data), _size(size) {}

        template<size_t N>
        inline constexpr Span(T (&arr)[N]) : _data(arr), _size(N) {}

        template<size_t N>
        inline constexpr Span(std::array<T, N>& arr) : _data(arr.data()), _size(N) {}

        template<typename C, typename = std::enable_if_t<is_compat<data_type<C>>>>
        inline constexpr Span(C&& vec) : _data(vec.data()), _size(std::distance(vec.begin(), vec.end())) {}

        inline constexpr size_t size() const { return _size; }
        inline constexpr bool is_empty() const { return !_size; }
        inline constexpr T* data() { return _data; }
        inline constexpr const T* data() const { return _data; }
        inline constexpr const_iterator begin() const { return _data; }
        inline constexpr const_iterator end() const { return _data + _size; }

        bool operator==(const Span& other) const { return size() == other.size() && std::equal(begin(), end(), other.begin(), other.end()); }
        bool operator!=(const Span& other) const { return !operator==(other); }

        inline constexpr T& operator[](size_t i) const {
            DEBUG_ASSERT(i < size());
            return _data[i];
        }

    private:
        T* _data = nullptr;
        size_t _size = 0;

};

template<typename T>
inline constexpr void hash_combine(T& seed, T value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T, typename V = typename T::value_type>
struct CollectionHasher : std::hash<V> {
    size_t operator()(const T& t) const noexcept {
        size_t h = size_t(0xd5a7de585d2af52b);
        for(const auto& e : t) {
            hash_combine(h, std::hash<V>::operator()(e));
        }
        return h;
    }
};


inline constexpr u32 str_hash(std::string_view str, u32 seed = 0xCAFECAFE) {
    constexpr u32 lut[256] = {
        0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
        0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
        0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
        0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
        0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
        0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
        0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
        0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
        0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
        0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
        0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
        0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
        0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
        0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
        0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
        0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
    };
    u32 crc = ~seed;
    for(const u8 c : str) {
        crc = (crc >> 8) ^ lut[(crc & 0xFF) ^ c];
    }
    return ~crc;
}

template<typename T>
inline constexpr T to_rad(T deg) {
    return deg * T(0.01745329251994329576923690768489);
}

template<typename T>
inline constexpr T to_deg(T rad) {
    return rad * T(57.295779513082320876798154814105);
}

double program_time();
Result<std::string> read_text_file(const std::string& file_name);

bool ends_with(std::string_view str, std::string_view suffix);

}

#endif // UTILS_H
