#include "core/piece_scanner.hpp"

#include "core/fail_fast.hpp"

namespace editor
{

PieceScanner::PieceScanner(const PieceTable &document, ByteRange permitted_range)
    : document_(&document), permitted_range_(permitted_range), position_(permitted_range.start)
{
    document_->for_each_chunk(permitted_range_, [](std::string_view) {});
}

bool PieceScanner::done() const noexcept
{
    return position_.value == permitted_range_.start.value + permitted_range_.length.value;
}

ByteOffset PieceScanner::position() const noexcept
{
    return position_;
}

std::optional<char> PieceScanner::peek() const
{
    if (done())
    {
        return std::nullopt;
    }
    return character_at(position_);
}

bool PieceScanner::accept(char expected)
{
    if (peek() != expected)
    {
        return false;
    }
    ++position_.value;
    return true;
}

bool PieceScanner::accept(std::string_view expected)
{
    EDITOR_CHECK(!expected.empty(), "an empty delimiter is ambiguous");
    if (!can_read(ByteLength{expected.size()}))
    {
        return false;
    }
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        if (character_at(ByteOffset{position_.value + index}) != expected[index])
        {
            return false;
        }
    }
    position_.value += expected.size();
    return true;
}

TextSpan PieceScanner::accept_while(const CharacterPredicate &predicate)
{
    EDITOR_CHECK(static_cast<bool>(predicate), "scanner predicate is empty");
    const ByteOffset start = position_;
    while (const auto character = peek())
    {
        if (!predicate(*character))
        {
            break;
        }
        ++position_.value;
    }
    return span_from(start);
}

TextSpan PieceScanner::accept_until(char delimiter)
{
    const ByteOffset start = position_;
    while (const auto character = peek())
    {
        if (*character == delimiter)
        {
            break;
        }
        ++position_.value;
    }
    return span_from(start);
}

TextSpan PieceScanner::accept_until(std::string_view delimiter)
{
    EDITOR_CHECK(!delimiter.empty(), "an empty delimiter is ambiguous");
    const ByteOffset start = position_;
    while (!done())
    {
        const ByteOffset candidate = position_;
        if (accept(delimiter))
        {
            position_ = candidate;
            break;
        }
        ++position_.value;
    }
    return span_from(start);
}

TextSpan PieceScanner::accept_all()
{
    const ByteOffset start = position_;
    position_.value = permitted_range_.start.value + permitted_range_.length.value;
    return span_from(start);
}

PieceScanner PieceScanner::subscanner(const TextSpan &span) const
{
    EDITOR_CHECK(&span.document() == document_, "subscanner span belongs to another document");
    const ByteRange range = span.range();
    const std::size_t permitted_end = permitted_range_.start.value + permitted_range_.length.value;
    EDITOR_CHECK(range.start.value >= permitted_range_.start.value, "subscanner starts outside permitted range");
    EDITOR_CHECK(range.length.value <= permitted_end - range.start.value, "subscanner ends outside permitted range");
    return PieceScanner{*document_, range};
}

bool PieceScanner::can_read(ByteLength length) const noexcept
{
    const std::size_t end = permitted_range_.start.value + permitted_range_.length.value;
    return length.value <= end - position_.value;
}

char PieceScanner::character_at(ByteOffset offset) const
{
    return document_->at(offset);
}

TextSpan PieceScanner::span_from(ByteOffset start) const
{
    return TextSpan{*document_, ByteRange{start, ByteLength{position_.value - start.value}}};
}

} // namespace editor
