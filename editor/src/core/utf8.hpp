#pragma once

#include <cstddef>
#include <string_view>

namespace editor::utf8
{

[[nodiscard]] constexpr bool is_continuation_byte(unsigned char byte) noexcept
{
    return (byte & 0xC0U) == 0x80U;
}

[[nodiscard]] constexpr std::size_t sequence_length(unsigned char lead) noexcept
{
    if ((lead & 0x80U) == 0)
    {
        return 1;
    }
    if ((lead & 0xE0U) == 0xC0U)
    {
        return 2;
    }
    if ((lead & 0xF0U) == 0xE0U)
    {
        return 3;
    }
    if ((lead & 0xF8U) == 0xF0U)
    {
        return 4;
    }
    return 1;
}

[[nodiscard]] inline bool is_valid(std::string_view text) noexcept
{
    for (std::size_t offset = 0; offset < text.size();)
    {
        const auto lead = static_cast<unsigned char>(text[offset]);
        if (lead <= 0x7FU)
        {
            ++offset;
            continue;
        }

        const std::size_t width = sequence_length(lead);
        if (width == 1 || offset + width > text.size())
        {
            return false;
        }
        if ((width == 2 && lead < 0xC2U) || (width == 4 && lead > 0xF4U))
        {
            return false;
        }

        const auto next = static_cast<unsigned char>(text[offset + 1]);
        if (!is_continuation_byte(next) || (lead == 0xE0U && next < 0xA0U) || (lead == 0xEDU && next > 0x9FU) ||
            (lead == 0xF0U && next < 0x90U) || (lead == 0xF4U && next > 0x8FU))
        {
            return false;
        }
        for (std::size_t index = 2; index < width; ++index)
        {
            if (!is_continuation_byte(static_cast<unsigned char>(text[offset + index])))
            {
                return false;
            }
        }
        offset += width;
    }
    return true;
}

[[nodiscard]] inline std::size_t code_point_count(std::string_view text) noexcept
{
    std::size_t count = 0;
    for (std::size_t offset = 0; offset < text.size();
         offset += sequence_length(static_cast<unsigned char>(text[offset])))
    {
        ++count;
    }
    return count;
}

[[nodiscard]] inline std::size_t byte_offset_for_code_point(std::string_view text, std::size_t code_point) noexcept
{
    std::size_t offset = 0;
    while (offset < text.size() && code_point > 0)
    {
        offset += sequence_length(static_cast<unsigned char>(text[offset]));
        --code_point;
    }
    return offset;
}

[[nodiscard]] inline int utf16_offset_for_byte(std::string_view text, std::size_t byte_offset) noexcept
{
    int utf16_offset = 0;
    for (std::size_t offset = 0; offset < byte_offset;)
    {
        const std::size_t width = sequence_length(static_cast<unsigned char>(text[offset]));
        utf16_offset += width == 4 ? 2 : 1;
        offset += width;
    }
    return utf16_offset;
}

[[nodiscard]] inline std::size_t byte_offset_for_utf16(std::string_view text, int utf16_offset) noexcept
{
    int consumed_utf16 = 0;
    std::size_t offset = 0;
    while (offset < text.size())
    {
        const std::size_t width = sequence_length(static_cast<unsigned char>(text[offset]));
        const int units = width == 4 ? 2 : 1;
        if (consumed_utf16 + units > utf16_offset)
        {
            break;
        }
        consumed_utf16 += units;
        offset += width;
    }
    return offset;
}

} // namespace editor::utf8
