#pragma once

#include "core/coordinates.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace editor {

enum class BufferSource {
    original,
    added,
};

struct Piece {
    BufferSource source;
    ByteOffset buffer_start;
    ByteLength length;

    auto operator<=>(const Piece&) const = default;
};

class PieceTable {
public:
    struct Snapshot {
        std::vector<Piece> pieces;
        ByteLength size;
    };

    explicit PieceTable(std::string original = {});

    void insert(ByteOffset position, std::string_view text);
    void erase(ByteRange range);

    [[nodiscard]] ByteLength size() const noexcept;
    [[nodiscard]] std::string copy(ByteRange range) const;
    [[nodiscard]] std::string to_string() const;
    void for_each_chunk(ByteRange range, const std::function<void(std::string_view)>& visitor) const;

    [[nodiscard]] Snapshot snapshot() const;
    void restore(const Snapshot& snapshot);

    void validate() const;

private:
    [[nodiscard]] std::string_view buffer_for(BufferSource source) const noexcept;
    void check_range(ByteRange range) const;
    void append_normalized(std::vector<Piece>& destination, Piece piece) const;
    void validate_pieces(const std::vector<Piece>& pieces, ByteLength size) const;

    std::string original_;
    std::string added_;
    std::vector<Piece> pieces_;
    ByteLength size_ {};
};

} // namespace editor
