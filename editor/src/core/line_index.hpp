#pragma once

#include "core/piece_table.hpp"

#include <vector>

namespace editor {

class LineIndex {
public:
    explicit LineIndex(const PieceTable& document);

    void rebuild(const PieceTable& document);

    [[nodiscard]] std::size_t line_count() const noexcept;
    [[nodiscard]] std::size_t line_of(ByteOffset offset) const;
    [[nodiscard]] ByteOffset line_start(std::size_t line) const;
    [[nodiscard]] ByteRange line_range(std::size_t line) const;

private:
    std::vector<ByteOffset> starts_;
    ByteLength document_size_ {};
};

} // namespace editor
