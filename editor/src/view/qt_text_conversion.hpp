#pragma once

#include <QString>

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace editor::qt {

inline QString to_qstring(std::string_view utf8)
{
    return QString::fromUtf8(utf8.data(), static_cast<qsizetype>(utf8.size()));
}

inline std::size_t utf8_sequence_length(unsigned char lead)
{
    if ((lead & 0x80U) == 0) return 1;
    if ((lead & 0xE0U) == 0xC0U) return 2;
    if ((lead & 0xF0U) == 0xE0U) return 3;
    if ((lead & 0xF8U) == 0xF0U) return 4;
    return 1;
}

inline int utf16_offset_for_byte(std::string_view utf8, std::size_t byte_offset)
{
    byte_offset = std::min(byte_offset, utf8.size());
    int utf16 = 0;
    for (std::size_t index = 0; index < byte_offset;) {
        const auto lead = static_cast<unsigned char>(utf8[index]);
        const std::size_t width = std::min(utf8_sequence_length(lead), byte_offset - index);
        utf16 += width == 4 ? 2 : 1;
        index += width;
    }
    return utf16;
}

inline std::size_t byte_offset_for_utf16(std::string_view utf8, int utf16_offset)
{
    int utf16 = 0;
    std::size_t index = 0;
    while (index < utf8.size() && utf16 < utf16_offset) {
        const auto lead = static_cast<unsigned char>(utf8[index]);
        const std::size_t width = std::min(utf8_sequence_length(lead), utf8.size() - index);
        const int units = width == 4 ? 2 : 1;
        if (utf16 + units > utf16_offset) {
            break;
        }
        utf16 += units;
        index += width;
    }
    return index;
}

} // namespace editor::qt
