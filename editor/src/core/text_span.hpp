#pragma once

#include "core/piece_table.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace editor {

class TextSpan {
public:
    TextSpan(const PieceTable& document, ByteRange range)
        : document_(&document)
        , range_(range)
    {
        document.for_each_chunk(range, [](std::string_view) {});
    }

    [[nodiscard]] ByteRange range() const noexcept { return range_; }
    [[nodiscard]] const PieceTable& document() const noexcept { return *document_; }
    [[nodiscard]] bool empty() const noexcept { return range_.length.value == 0; }

    void for_each_chunk(const std::function<void(std::string_view)>& visitor) const
    {
        document_->for_each_chunk(range_, visitor);
    }

    [[nodiscard]] bool equals(std::string_view expected) const
    {
        if (expected.size() != range_.length.value) {
            return false;
        }
        std::size_t compared = 0;
        bool equal = true;
        for_each_chunk([&](std::string_view chunk) {
            equal = equal && chunk == expected.substr(compared, chunk.size());
            compared += chunk.size();
        });
        return equal;
    }

    [[nodiscard]] std::string to_string() const { return document_->copy(range_); }

private:
    const PieceTable* document_;
    ByteRange range_;
};

} // namespace editor
