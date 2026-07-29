#pragma once

#include "core/piece_table.hpp"
#include "core/selection.hpp"

#include <optional>
#include <vector>

namespace editor {

enum class EditKind {
    insert,
    erase,
    replace,
};

struct EditCommand {
    EditKind kind;
    PieceTable::Snapshot before;
    PieceTable::Snapshot after;
    Selection selection_before;
    Selection selection_after;
};

class UndoHistory {
public:
    void begin_transaction();
    void record(EditCommand command);
    void commit_transaction();
    void clear();
    void break_insert_coalescing() noexcept;

    [[nodiscard]] bool can_undo() const noexcept;
    [[nodiscard]] bool can_redo() const noexcept;
    [[nodiscard]] std::optional<EditCommand> undo();
    [[nodiscard]] std::optional<EditCommand> redo();

private:
    void append(EditCommand command);

    std::vector<EditCommand> undo_;
    std::vector<EditCommand> redo_;
    std::optional<EditCommand> pending_transaction_;
    bool transaction_active_ {};
    bool allow_insert_coalescing_ {true};
};

} // namespace editor
