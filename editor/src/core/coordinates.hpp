#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

namespace editor {

struct ByteOffset {
    std::size_t value {};

    auto operator<=>(const ByteOffset&) const = default;
};

struct ByteLength {
    std::size_t value {};

    auto operator<=>(const ByteLength&) const = default;
};

struct ByteRange {
    ByteOffset start {};
    ByteLength length {};
};

struct DocumentRevision {
    std::uint64_t value {};

    auto operator<=>(const DocumentRevision&) const = default;
};

} // namespace editor
