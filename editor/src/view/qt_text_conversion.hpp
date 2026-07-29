#pragma once

#include "core/utf8.hpp"

#include <QString>

#include <cstddef>
#include <string_view>

namespace editor::qt
{

inline QString to_qstring(std::string_view utf8)
{
    return QString::fromUtf8(utf8.data(), static_cast<qsizetype>(utf8.size()));
}

inline int utf16_offset_for_byte(std::string_view utf8, std::size_t byte_offset)
{
    return editor::utf8::utf16_offset_for_byte(utf8, byte_offset);
}

inline std::size_t byte_offset_for_utf16(std::string_view utf8, int utf16_offset)
{
    return editor::utf8::byte_offset_for_utf16(utf8, utf16_offset);
}

} // namespace editor::qt
