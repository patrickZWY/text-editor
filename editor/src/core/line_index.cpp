#include "core/line_index.hpp"

#include "core/fail_fast.hpp"

#include <algorithm>

namespace editor {

LineIndex::LineIndex(const PieceTable& document)
{
    rebuild(document);
}

void LineIndex::rebuild(const PieceTable& document)
{
    starts_.clear();
    starts_.push_back(ByteOffset{0});
    document_size_ = document.size();
    std::size_t position = 0;
    document.for_each_chunk(ByteRange{ByteOffset{0}, document_size_}, [&](std::string_view chunk) {
        for (const char character : chunk) {
            ++position;
            if (character == '\n') {
                starts_.push_back(ByteOffset{position});
            }
        }
    });
}

std::size_t LineIndex::line_count() const noexcept
{
    return starts_.size();
}

std::size_t LineIndex::line_of(ByteOffset offset) const
{
    EDITOR_CHECK(offset.value <= document_size_.value, "offset outside line index");
    return static_cast<std::size_t>(std::upper_bound(
        starts_.begin(), starts_.end(), offset,
        [](ByteOffset value, ByteOffset start) { return value.value < start.value; }) - starts_.begin() - 1);
}

ByteOffset LineIndex::line_start(std::size_t line) const
{
    EDITOR_CHECK(line < starts_.size(), "line outside line index");
    return starts_[line];
}

ByteRange LineIndex::line_range(std::size_t line) const
{
    const ByteOffset start = line_start(line);
    const std::size_t end = line + 1 < starts_.size()
        ? starts_[line + 1].value - 1
        : document_size_.value;
    return ByteRange{start, ByteLength{end - start.value}};
}

} // namespace editor
