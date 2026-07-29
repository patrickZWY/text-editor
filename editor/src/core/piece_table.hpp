#pragma once

#include "core/coordinates.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace editor
{

enum class BufferSource
{
    original,
    added,
};

struct Piece
{
    BufferSource source;
    ByteOffset buffer_start;
    ByteLength length;

    auto operator<=>(const Piece &) const = default;
};

class PieceTable
{
  private:
    struct Buffers;

  public:
    struct Snapshot
    {
        std::vector<Piece> pieces;
        ByteLength size;

      private:
        friend class PieceTable;

        Snapshot(std::vector<Piece> snapshot_pieces, ByteLength snapshot_size,
                 std::shared_ptr<Buffers> snapshot_buffers)
            : pieces(std::move(snapshot_pieces)), size(snapshot_size), buffers(std::move(snapshot_buffers))
        {
        }

        std::shared_ptr<Buffers> buffers;
    };

    explicit PieceTable(std::string original = {});

    void insert(ByteOffset position, std::string_view text);
    void erase(ByteRange range);

    [[nodiscard]] ByteLength size() const noexcept;
    [[nodiscard]] char at(ByteOffset position) const;
    [[nodiscard]] std::string copy(ByteRange range) const;
    [[nodiscard]] std::string to_string() const;
    void for_each_chunk(ByteRange range, const std::function<void(std::string_view)> &visitor) const;

    [[nodiscard]] Snapshot snapshot() const;
    void restore(const Snapshot &snapshot);

    void validate() const;

  private:
    struct Buffers
    {
        std::string original;
        std::string added;
    };

    [[nodiscard]] std::string_view buffer_for(BufferSource source) const noexcept;
    void check_range(ByteRange range) const;
    void append_normalized(std::vector<Piece> &destination, Piece piece) const;
    void validate_pieces(const std::vector<Piece> &pieces, ByteLength size) const;

    std::shared_ptr<Buffers> buffers_;
    std::vector<Piece> pieces_;
    ByteLength size_{};
};

} // namespace editor
