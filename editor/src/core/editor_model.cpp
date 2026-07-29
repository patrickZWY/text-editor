#include "core/editor_model.hpp"

#include "core/fail_fast.hpp"

#include <algorithm>

namespace editor {

EditorModel::EditorModel(std::string original)
    : document_(std::move(original))
    , line_index_(document_)
{
}

const PieceTable& EditorModel::document() const noexcept { return document_; }
const LineIndex& EditorModel::line_index() const noexcept { return line_index_; }
Selection EditorModel::selection() const noexcept { return selection_; }
DocumentRevision EditorModel::revision() const noexcept { return revision_; }

void EditorModel::load(std::string original)
{
    document_ = PieceTable{std::move(original)};
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
    preferred_column_ = selection.cursor.value - line_index_.line_start(line_index_.line_of(selection.cursor)).value;
}

void EditorModel::insert_text(std::string_view text)
{
    replace_selection(text, selection_.empty() ? EditKind::insert : EditKind::replace);
}

void EditorModel::delete_selection()
{
    if (!selection_.empty()) {
        replace_selection({}, EditKind::erase);
    }
}

void EditorModel::backspace()
{
    break_undo_coalescing();
    if (!selection_.empty()) {
        delete_selection();
    } else if (selection_.cursor.value > 0) {
        set_selection(Selection{ByteOffset{selection_.cursor.value - 1}, selection_.cursor});
        delete_selection();
    }
}

void EditorModel::delete_forward()
{
    break_undo_coalescing();
    if (!selection_.empty()) {
        delete_selection();
    } else if (selection_.cursor.value < document_.size().value) {
        set_selection(Selection{selection_.cursor, ByteOffset{selection_.cursor.value + 1}});
        delete_selection();
    }
}

void EditorModel::move_left(bool extend_selection)
{
    const std::size_t target = selection_.cursor.value == 0 ? 0 : selection_.cursor.value - 1;
    move_to(ByteOffset{target}, extend_selection);
}

void EditorModel::move_right(bool extend_selection)
{
    const std::size_t target = std::min(selection_.cursor.value + 1, document_.size().value);
    move_to(ByteOffset{target}, extend_selection);
}

void EditorModel::move_up(bool extend_selection)
{
    const std::size_t line = line_index_.line_of(selection_.cursor);
    if (line == 0) {
        return;
    }
    const ByteRange target_line = line_index_.line_range(line - 1);
    move_to(ByteOffset{target_line.start.value + std::min(preferred_column_, target_line.length.value)}, extend_selection);
}

void EditorModel::move_down(bool extend_selection)
{
    const std::size_t line = line_index_.line_of(selection_.cursor);
    if (line + 1 >= line_index_.line_count()) {
        return;
    }
    const ByteRange target_line = line_index_.line_range(line + 1);
    move_to(ByteOffset{target_line.start.value + std::min(preferred_column_, target_line.length.value)}, extend_selection);
}

void EditorModel::undo()
{
    if (const auto command = history_.undo()) {
        document_.restore(command->before);
        selection_ = command->selection_before;
        rebuild_line_index();
        ++revision_.value;
    }
}

void EditorModel::redo()
{
    if (const auto command = history_.redo()) {
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
    selection_ = Selection{ByteOffset{selected.start.value + text.size()}, ByteOffset{selected.start.value + text.size()}};
    rebuild_line_index();
    preferred_column_ = text.size();
    const PieceTable::Snapshot after = document_.snapshot();
    if (before.pieces != after.pieces) {
        history_.record(EditCommand{kind, before, after, before_selection, selection_});
        ++revision_.value;
    }
}

void EditorModel::validate_selection(Selection selection) const
{
    EDITOR_CHECK(selection.anchor.value <= document_.size().value, "selection anchor outside document");
    EDITOR_CHECK(selection.cursor.value <= document_.size().value, "selection cursor outside document");
}

void EditorModel::move_to(ByteOffset target, bool extend_selection)
{
    EDITOR_CHECK(target.value <= document_.size().value, "cursor target outside document");
    selection_.cursor = target;
    break_undo_coalescing();
    if (!extend_selection) {
        selection_.anchor = target;
    }
    preferred_column_ = target.value - line_index_.line_start(line_index_.line_of(target)).value;
}

void EditorModel::rebuild_line_index()
{
    line_index_.rebuild(document_);
}

} // namespace editor
