#pragma once

#include "core/coordinates.hpp"

#include <algorithm>

namespace editor
{

struct Selection
{
    ByteOffset anchor{};
    ByteOffset cursor{};

    [[nodiscard]] ByteRange range() const noexcept
    {
        const std::size_t start = std::min(anchor.value, cursor.value);
        const std::size_t end = std::max(anchor.value, cursor.value);
        return ByteRange{ByteOffset{start}, ByteLength{end - start}};
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return anchor == cursor;
    }

    auto operator<=>(const Selection &) const = default;
};

} // namespace editor
