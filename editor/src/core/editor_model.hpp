#pragma once

#include "core/line_index.hpp"
#include "core/selection.hpp"
#include "core/undo_history.hpp"

#include <string_view>
#include <string>

namespace editor {

class EditorModel {
public:
    explicit EditorModel(std::string original = {});

    [[nodiscard]] const PieceTable& document() const noexcept;
    [[nodiscard]] const LineIndex& line_index() const noexcept;
    [[nodiscard]] Selection selection() const noexcept;
    [[nodiscard]] DocumentRevision revision() const noexcept;

    void load(std::string original);
    void break_undo_coalescing() noexcept;
    void set_selection(Selection selection);
    void insert_text(std::string_view text);
    void delete_selection();
    void backspace();
    void delete_forward();

    void move_left(bool extend_selection);
    void move_right(bool extend_selection);
    void move_up(bool extend_selection);
    void move_down(bool extend_selection);

    void undo();
    void redo();

private:
    void replace_selection(std::string_view text, EditKind kind);
    void validate_selection(Selection selection) const;
    void move_to(ByteOffset target, bool extend_selection);
    void rebuild_line_index();

    PieceTable document_;
    UndoHistory history_;
    LineIndex line_index_;
    Selection selection_;
    std::size_t preferred_column_ {};
    DocumentRevision revision_ {};
};

} // namespace editor
