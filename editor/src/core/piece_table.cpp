#include "core/piece_table.hpp"

#include "core/fail_fast.hpp"

#include <algorithm>
#include <limits>

namespace editor
{
namespace
{

[[nodiscard]] bool fits_in_buffer(Piece piece, std::string_view buffer) noexcept
{
    return piece.buffer_start.value <= buffer.size() && piece.length.value <= buffer.size() - piece.buffer_start.value;
}

[[nodiscard]] bool is_contiguous(Piece left, Piece right) noexcept
{
    return left.source == right.source &&
           left.buffer_start.value <= std::numeric_limits<std::size_t>::max() - left.length.value &&
           left.buffer_start.value + left.length.value == right.buffer_start.value;
}

} // namespace

PieceTable::PieceTable(std::string original)
    : buffers_(std::make_shared<Buffers>(Buffers{std::move(original), {}})), size_{buffers_->original.size()}
{
    if (!buffers_->original.empty())
    {
        pieces_.push_back(Piece{BufferSource::original, ByteOffset{0}, ByteLength{buffers_->original.size()}});
    }
    validate();
}

void PieceTable::insert(ByteOffset position, std::string_view text)
{
    EDITOR_CHECK(position.value <= size_.value, "insert position outside document");
    if (text.empty())
    {
        return;
    }

    EDITOR_CHECK(text.size() <= std::numeric_limits<std::size_t>::max() - buffers_->added.size(),
                 "add buffer overflow");
    EDITOR_CHECK(text.size() <= std::numeric_limits<std::size_t>::max() - size_.value, "document size overflow");

    const Piece inserted{BufferSource::added, ByteOffset{buffers_->added.size()}, ByteLength{text.size()}};
    buffers_->added.append(text);

    std::vector<Piece> updated;
    updated.reserve(pieces_.size() + 2);

    std::size_t document_position = 0;
    bool did_insert = false;
    for (const Piece piece : pieces_)
    {
        const std::size_t piece_end = document_position + piece.length.value;
        if (!did_insert && position.value <= piece_end)
        {
            const std::size_t split = position.value - document_position;
            if (split > 0)
            {
                append_normalized(updated, Piece{piece.source, piece.buffer_start, ByteLength{split}});
            }
            append_normalized(updated, inserted);
            if (split < piece.length.value)
            {
                append_normalized(updated, Piece{
                                               piece.source,
                                               ByteOffset{piece.buffer_start.value + split},
                                               ByteLength{piece.length.value - split},
                                           });
            }
            did_insert = true;
        }
        else
        {
            append_normalized(updated, piece);
        }
        document_position = piece_end;
    }

    if (!did_insert)
    {
        append_normalized(updated, inserted);
    }

    pieces_ = std::move(updated);
    size_.value += text.size();
    validate();
}

void PieceTable::erase(ByteRange range)
{
    check_range(range);
    if (range.length.value == 0)
    {
        return;
    }

    const std::size_t range_end = range.start.value + range.length.value;
    std::vector<Piece> updated;
    updated.reserve(pieces_.size());

    std::size_t document_position = 0;
    for (const Piece piece : pieces_)
    {
        const std::size_t piece_end = document_position + piece.length.value;
        const std::size_t kept_prefix =
            range.start.value > document_position ? std::min(range.start.value, piece_end) - document_position : 0;
        const std::size_t suffix_start =
            range_end < piece_end ? std::max(range_end, document_position) - document_position : piece.length.value;

        if (kept_prefix > 0)
        {
            append_normalized(updated, Piece{piece.source, piece.buffer_start, ByteLength{kept_prefix}});
        }
        if (suffix_start < piece.length.value)
        {
            append_normalized(updated, Piece{
                                           piece.source,
                                           ByteOffset{piece.buffer_start.value + suffix_start},
                                           ByteLength{piece.length.value - suffix_start},
                                       });
        }

        document_position = piece_end;
    }

    pieces_ = std::move(updated);
    size_.value -= range.length.value;
    validate();
}

ByteLength PieceTable::size() const noexcept
{
    return size_;
}

char PieceTable::at(ByteOffset position) const
{
    EDITOR_CHECK(position.value < size_.value, "character position outside document");
    std::size_t document_position = 0;
    for (const Piece piece : pieces_)
    {
        const std::size_t piece_end = document_position + piece.length.value;
        if (position.value < piece_end)
        {
            return buffer_for(piece.source)[piece.buffer_start.value + position.value - document_position];
        }
        document_position = piece_end;
    }
    EDITOR_CHECK(false, "piece table has no piece at a valid position");
}

std::string PieceTable::copy(ByteRange range) const
{
    check_range(range);
    std::string result;
    result.reserve(range.length.value);
    for_each_chunk(range, [&result](std::string_view chunk) { result.append(chunk); });
    return result;
}

std::string PieceTable::to_string() const
{
    return copy(ByteRange{ByteOffset{0}, size_});
}

void PieceTable::for_each_chunk(ByteRange range, const std::function<void(std::string_view)> &visitor) const
{
    check_range(range);
    const std::size_t range_end = range.start.value + range.length.value;
    std::size_t document_position = 0;
    for (const Piece piece : pieces_)
    {
        const std::size_t piece_end = document_position + piece.length.value;
        const std::size_t chunk_start = std::max(document_position, range.start.value);
        const std::size_t chunk_end = std::min(piece_end, range_end);
        if (chunk_start < chunk_end)
        {
            const auto buffer = buffer_for(piece.source);
            const std::size_t buffer_start = piece.buffer_start.value + chunk_start - document_position;
            visitor(buffer.substr(buffer_start, chunk_end - chunk_start));
        }
        document_position = piece_end;
        if (document_position >= range_end)
        {
            break;
        }
    }
}

PieceTable::Snapshot PieceTable::snapshot() const
{
    return Snapshot{pieces_, size_, buffers_};
}

void PieceTable::restore(const Snapshot &snapshot)
{
    EDITOR_CHECK(snapshot.buffers, "snapshot has no backing buffers");
    buffers_ = snapshot.buffers;
    validate_pieces(snapshot.pieces, snapshot.size);
    pieces_ = snapshot.pieces;
    size_ = snapshot.size;
}

void PieceTable::validate() const
{
    validate_pieces(pieces_, size_);
}

void PieceTable::validate_pieces(const std::vector<Piece> &pieces, ByteLength size) const
{
    std::size_t accumulated = 0;
    for (std::size_t index = 0; index < pieces.size(); ++index)
    {
        const Piece piece = pieces[index];
        EDITOR_CHECK(piece.length.value > 0, "piece has zero length");
        EDITOR_CHECK(fits_in_buffer(piece, buffer_for(piece.source)), "piece points outside its buffer");
        EDITOR_CHECK(piece.length.value <= std::numeric_limits<std::size_t>::max() - accumulated,
                     "piece lengths overflow");
        accumulated += piece.length.value;

        if (index > 0)
        {
            EDITOR_CHECK(!is_contiguous(pieces[index - 1], piece), "adjacent pieces were not normalized");
        }
    }
    EDITOR_CHECK(accumulated == size.value, "piece lengths do not equal document size");
}

std::string_view PieceTable::buffer_for(BufferSource source) const noexcept
{
    return source == BufferSource::original ? buffers_->original : buffers_->added;
}

void PieceTable::check_range(ByteRange range) const
{
    EDITOR_CHECK(range.start.value <= size_.value, "range starts outside document");
    EDITOR_CHECK(range.length.value <= size_.value - range.start.value, "range ends outside document");
}

void PieceTable::append_normalized(std::vector<Piece> &destination, Piece piece) const
{
    if (piece.length.value == 0)
    {
        return;
    }
    if (!destination.empty() && is_contiguous(destination.back(), piece))
    {
        destination.back().length.value += piece.length.value;
        return;
    }
    destination.push_back(piece);
}

} // namespace editor
