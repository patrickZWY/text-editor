#include "core/editor_model.hpp"

#include "core/fail_fast.hpp"
#include "core/utf8.hpp"

#include <algorithm>

namespace editor
{

EditorModel::EditorModel(std::string original) : document_(checked_utf8(std::move(original))), line_index_(document_)
{
}

const PieceTable &EditorModel::document() const noexcept
{
    return document_;
}
const LineIndex &EditorModel::line_index() const noexcept
{
    return line_index_;
}
Selection EditorModel::selection() const noexcept
{
    return selection_;
}
DocumentRevision EditorModel::revision() const noexcept
{
    return revision_;
}

void EditorModel::load(std::string original)
{
    document_ = PieceTable{checked_utf8(std::move(original))};
    history_.clear();
    selection_ = {};
    preferred_column_ = 0;
    rebuild_line_index();
    ++revision_.value;
}

void EditorModel::break_undo_coalescing() noexcept
{
    history_.break_insert_coalescing();
}

void EditorModel::set_selection(Selection selection)
{
    validate_selection(selection);
    selection_ = selection;
    break_undo_coalescing();
    preferred_column_ = code_point_column(selection.cursor);
}

void EditorModel::insert_text(std::string_view text)
{
    EDITOR_CHECK(utf8::is_valid(text), "inserted text is not valid UTF-8");
    replace_selection(text, selection_.empty() ? EditKind::insert : EditKind::replace);
}

void EditorModel::delete_selection()
{
    if (!selection_.empty())
    {
        replace_selection({}, EditKind::erase);
    }
}

void EditorModel::backspace()
{
    break_undo_coalescing();
    if (!selection_.empty())
    {
        delete_selection();
    }
    else if (selection_.cursor.value > 0)
    {
        set_selection(Selection{previous_code_point(selection_.cursor), selection_.cursor});
        delete_selection();
    }
}

void EditorModel::delete_forward()
{
    break_undo_coalescing();
    if (!selection_.empty())
    {
        delete_selection();
    }
    else if (selection_.cursor.value < document_.size().value)
    {
        set_selection(Selection{selection_.cursor, next_code_point(selection_.cursor)});
        delete_selection();
    }
}

void EditorModel::move_left(bool extend_selection)
{
    move_to(previous_code_point(selection_.cursor), extend_selection);
}

void EditorModel::move_right(bool extend_selection)
{
    move_to(next_code_point(selection_.cursor), extend_selection);
}

void EditorModel::move_up(bool extend_selection)
{
    const std::size_t line = line_index_.line_of(selection_.cursor);
    if (line == 0)
    {
        return;
    }
    const ByteRange target_line = line_index_.line_range(line - 1);
    move_to(offset_at_column(target_line, preferred_column_), extend_selection);
}

void EditorModel::move_down(bool extend_selection)
{
    const std::size_t line = line_index_.line_of(selection_.cursor);
    if (line + 1 >= line_index_.line_count())
    {
        return;
    }
    const ByteRange target_line = line_index_.line_range(line + 1);
    move_to(offset_at_column(target_line, preferred_column_), extend_selection);
}

void EditorModel::undo()
{
    if (const auto command = history_.undo())
    {
        document_.restore(command->before);
        selection_ = command->selection_before;
        rebuild_line_index();
        ++revision_.value;
    }
}

void EditorModel::redo()
{
    if (const auto command = history_.redo())
    {
        document_.restore(command->after);
        selection_ = command->selection_after;
        rebuild_line_index();
        ++revision_.value;
    }
}

void EditorModel::replace_selection(std::string_view text, EditKind kind)
{
    const Selection before_selection = selection_;
    const PieceTable::Snapshot before = document_.snapshot();
    const ByteRange selected = selection_.range();
    document_.erase(selected);
    document_.insert(selected.start, text);
    selection_ =
        Selection{ByteOffset{selected.start.value + text.size()}, ByteOffset{selected.start.value + text.size()}};
    rebuild_line_index();
    preferred_column_ = code_point_column(selection_.cursor);
    const PieceTable::Snapshot after = document_.snapshot();
    if (before.pieces != after.pieces)
    {
        history_.record(EditCommand{kind, before, after, before_selection, selection_});
        ++revision_.value;
    }
}

void EditorModel::validate_selection(Selection selection) const
{
    EDITOR_CHECK(selection.anchor.value <= document_.size().value, "selection anchor outside document");
    EDITOR_CHECK(selection.cursor.value <= document_.size().value, "selection cursor outside document");
    if (selection.anchor.value < document_.size().value)
    {
        EDITOR_CHECK(!utf8::is_continuation_byte(static_cast<unsigned char>(document_.at(selection.anchor))),
                     "selection anchor is inside a UTF-8 code point");
    }
    if (selection.cursor.value < document_.size().value)
    {
        EDITOR_CHECK(!utf8::is_continuation_byte(static_cast<unsigned char>(document_.at(selection.cursor))),
                     "selection cursor is inside a UTF-8 code point");
    }
}

void EditorModel::move_to(ByteOffset target, bool extend_selection)
{
    EDITOR_CHECK(target.value <= document_.size().value, "cursor target outside document");
    selection_.cursor = target;
    break_undo_coalescing();
    if (!extend_selection)
    {
        selection_.anchor = target;
    }
    preferred_column_ = code_point_column(target);
}

std::string EditorModel::checked_utf8(std::string text)
{
    EDITOR_CHECK(utf8::is_valid(text), "document text is not valid UTF-8");
    return text;
}

ByteOffset EditorModel::previous_code_point(ByteOffset offset) const
{
    if (offset.value == 0)
    {
        return offset;
    }

    std::size_t previous = offset.value - 1;
    while (previous > 0 && utf8::is_continuation_byte(static_cast<unsigned char>(document_.at(ByteOffset{previous}))))
    {
        --previous;
    }
    return ByteOffset{previous};
}

ByteOffset EditorModel::next_code_point(ByteOffset offset) const
{
    if (offset.value == document_.size().value)
    {
        return offset;
    }
    return ByteOffset{offset.value + utf8::sequence_length(static_cast<unsigned char>(document_.at(offset)))};
}

std::size_t EditorModel::code_point_column(ByteOffset offset) const
{
    const ByteOffset line_start = line_index_.line_start(line_index_.line_of(offset));
    return utf8::code_point_count(document_.copy(ByteRange{line_start, ByteLength{offset.value - line_start.value}}));
}

ByteOffset EditorModel::offset_at_column(ByteRange line, std::size_t column) const
{
    const std::string text = document_.copy(line);
    return ByteOffset{line.start.value + utf8::byte_offset_for_code_point(text, column)};
}

void EditorModel::rebuild_line_index()
{
    line_index_.rebuild(document_);
}

} // namespace editor
