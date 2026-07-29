#include "core/piece_scanner.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cctype>

namespace
{

using editor::ByteLength;
using editor::ByteOffset;
using editor::ByteRange;
using editor::PieceScanner;
using editor::PieceTable;
using editor::TextSpan;

ByteRange range(std::size_t start, std::size_t length)
{
    return ByteRange{ByteOffset{start}, ByteLength{length}};
}

TEST_CASE("scanner reads structural delimiters without returning copied tokens")
{
    PieceTable document{"key=value"};
    document.insert(ByteOffset{3}, "");
    PieceScanner scanner{document, range(0, document.size().value)};

    const TextSpan key = scanner.accept_until('=');
    REQUIRE(key.equals("key"));
    REQUIRE(scanner.accept('='));
    REQUIRE(scanner.accept_all().equals("value"));
    REQUIRE(scanner.done());
}

TEST_CASE("scanner finds delimiters crossing piece boundaries")
{
    PieceTable document{"https"};
    document.insert(ByteOffset{5}, "://example.com");
    PieceScanner scanner{document, range(0, document.size().value)};

    REQUIRE(scanner.accept_until("://").equals("https"));
    REQUIRE(scanner.accept("://"));
    REQUIRE(scanner.accept_all().equals("example.com"));
}

TEST_CASE("scanner never reads outside its permitted range")
{
    PieceTable document{"[broken\nnext=value"};
    PieceScanner whole{document, range(0, document.size().value)};
    const TextSpan broken = whole.accept_until('\n');
    PieceScanner bounded = whole.subscanner(broken);

    REQUIRE(bounded.accept_until(']').equals("[broken"));
    REQUIRE_FALSE(bounded.accept(']'));
    REQUIRE(bounded.done());
}

TEST_CASE("scanner predicate and failed accepts preserve its cursor")
{
    PieceTable document{"name123"};
    PieceScanner scanner{document, range(0, document.size().value)};

    const auto identifier =
        scanner.accept_while([](char character) { return std::isalpha(static_cast<unsigned char>(character)) != 0; });
    REQUIRE(identifier.equals("name"));
    const auto position = scanner.position();
    REQUIRE_FALSE(scanner.accept("xyz"));
    REQUIRE(scanner.position() == position);
    REQUIRE(scanner.accept_all().equals("123"));
}

} // namespace
