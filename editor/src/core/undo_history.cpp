#include "core/undo_history.hpp"

#include "core/fail_fast.hpp"

namespace editor {

void UndoHistory::begin_transaction()
{
    EDITOR_CHECK(!transaction_active_, "a transaction is already active");
    transaction_active_ = true;
    pending_transaction_.reset();
}

void UndoHistory::record(EditCommand command)
{
    if (transaction_active_) {
        if (!pending_transaction_) {
            pending_transaction_ = std::move(command);
        } else {
            pending_transaction_->after = std::move(command.after);
            pending_transaction_->selection_after = command.selection_after;
            pending_transaction_->kind = EditKind::replace;
        }
        return;
    }
    append(std::move(command));
}

void UndoHistory::commit_transaction()
{
    EDITOR_CHECK(transaction_active_, "no transaction is active");
    if (pending_transaction_ && pending_transaction_->before.pieces != pending_transaction_->after.pieces) {
        append(std::move(*pending_transaction_));
    }
    pending_transaction_.reset();
    transaction_active_ = false;
}

void UndoHistory::clear()
{
    EDITOR_CHECK(!transaction_active_, "cannot clear while a transaction is active");
    undo_.clear();
    redo_.clear();
    allow_insert_coalescing_ = true;
}

void UndoHistory::break_insert_coalescing() noexcept
{
    allow_insert_coalescing_ = false;
}

bool UndoHistory::can_undo() const noexcept
{
    return !undo_.empty();
}

bool UndoHistory::can_redo() const noexcept
{
    return !redo_.empty();
}

std::optional<EditCommand> UndoHistory::undo()
{
    EDITOR_CHECK(!transaction_active_, "cannot undo while a transaction is active");
    if (undo_.empty()) {
        return std::nullopt;
    }
    EditCommand command = std::move(undo_.back());
    undo_.pop_back();
    redo_.push_back(command);
    break_insert_coalescing();
    return command;
}

std::optional<EditCommand> UndoHistory::redo()
{
    EDITOR_CHECK(!transaction_active_, "cannot redo while a transaction is active");
    if (redo_.empty()) {
        return std::nullopt;
    }
    EditCommand command = std::move(redo_.back());
    redo_.pop_back();
    undo_.push_back(command);
    break_insert_coalescing();
    return command;
}

void UndoHistory::append(EditCommand command)
{
    if (allow_insert_coalescing_ && command.kind == EditKind::insert && !undo_.empty()) {
        EditCommand& previous = undo_.back();
        if (previous.kind == EditKind::insert && previous.selection_after == command.selection_before) {
            previous.after = std::move(command.after);
            previous.selection_after = command.selection_after;
            redo_.clear();
            allow_insert_coalescing_ = true;
            return;
        }
    }
    undo_.push_back(std::move(command));
    redo_.clear();
    allow_insert_coalescing_ = true;
}

} // namespace editor
