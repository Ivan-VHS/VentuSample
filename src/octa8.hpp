#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ventu {
enum class Octa8Kind : std::uint8_t { Null=0, Boolean=1, Integer=2, Number=3, Text=4, Bytes=5, Sequence=6, Record=7 };
struct Octa8Value;
using Octa8Bytes = std::vector<std::uint8_t>;
using Octa8Sequence = std::vector<Octa8Value>;
using Octa8Record = std::map<std::string, Octa8Value, std::less<>>;
using Octa8SequencePtr = std::shared_ptr<Octa8Sequence>;
using Octa8RecordPtr = std::shared_ptr<Octa8Record>;

struct Octa8Value {
    using Storage = std::variant<std::monostate, bool, std::int64_t, double, std::string,
                                 Octa8Bytes, Octa8SequencePtr, Octa8RecordPtr>;
    Storage data{};
    Octa8Kind kind() const noexcept { return static_cast<Octa8Kind>(data.index()); }
};

inline constexpr std::size_t OCTA8_MAX_ITEMS = 100000;
inline constexpr std::size_t OCTA8_MAX_TEXT_BYTES = 1u << 20;
inline constexpr std::size_t OCTA8_MAX_BINARY_BYTES = 16u << 20;
inline constexpr unsigned OCTA8_MAX_DEPTH = 64;

inline std::string_view octa8_kind_name(Octa8Kind kind) noexcept {
    constexpr std::string_view names[] = {
        "null", "boolean", "integer", "number", "text", "bytes", "sequence", "record"
    };
    const auto index = static_cast<std::size_t>(kind);
    return index < 8 ? names[index] : std::string_view{};
}

inline bool octa8_validate_node(const Octa8Value& value, unsigned depth,
                                std::size_t& budget) noexcept {
    if (depth > OCTA8_MAX_DEPTH || ++budget > OCTA8_MAX_ITEMS) return false;
    switch (value.data.index()) {
    case 0: case 1: case 2: return true;
    case 3: return std::isfinite(std::get<double>(value.data));
    case 4: return std::get<std::string>(value.data).size() <= OCTA8_MAX_TEXT_BYTES;
    case 5: return std::get<Octa8Bytes>(value.data).size() <= OCTA8_MAX_BINARY_BYTES;
    case 6: {
        const auto& ptr = std::get<Octa8SequencePtr>(value.data);
        if (!ptr || ptr->size() > OCTA8_MAX_ITEMS) return false;
        for (const auto& child : *ptr) {
            if (!octa8_validate_node(child, depth + 1, budget)) return false;
        }
        return true;
    }
    case 7: {
        const auto& ptr = std::get<Octa8RecordPtr>(value.data);
        if (!ptr || ptr->size() > OCTA8_MAX_ITEMS) return false;
        for (const auto& [key, child] : *ptr) {
            if (key.size() > OCTA8_MAX_TEXT_BYTES ||
                !octa8_validate_node(child, depth + 1, budget)) return false;
        }
        return true;
    }
    default: return false;
    }
}

inline bool octa8_validate(const Octa8Value& value) noexcept {
    std::size_t budget = 0;
    return octa8_validate_node(value, 0, budget);
}
} // namespace ventu
