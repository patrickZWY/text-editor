#pragma once

#include "core/text_span.hpp"

#include <functional>
#include <optional>
#include <string_view>

namespace editor {

// Bounded scanner design informed by https://bkaradzic.github.io/posts/scanner/
class PieceScanner {
public:
    using CharacterPredicate = std::function<bool(char)>;

    PieceScanner(const PieceTable& document, ByteRange permitted_range);

    [[nodiscard]] bool done() const noexcept;
    [[nodiscard]] ByteOffset position() const noexcept;
    [[nodiscard]] std::optional<char> peek() const;

    bool accept(char expected);
    bool accept(std::string_view expected);

    [[nodiscard]] TextSpan accept_while(const CharacterPredicate& predicate);
    [[nodiscard]] TextSpan accept_until(char delimiter);
    [[nodiscard]] TextSpan accept_until(std::string_view delimiter);
    [[nodiscard]] TextSpan accept_all();
    [[nodiscard]] PieceScanner subscanner(const TextSpan& span) const;

private:
    [[nodiscard]] bool can_read(ByteLength length) const noexcept;
    [[nodiscard]] char character_at(ByteOffset offset) const;
    [[nodiscard]] TextSpan span_from(ByteOffset start) const;

    const PieceTable* document_;
    ByteRange permitted_range_;
    ByteOffset position_;
};

} // namespace editor
