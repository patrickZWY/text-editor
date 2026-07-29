#include "core/piece_table.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <random>
#include <string>

namespace {

using editor::ByteLength;
using editor::ByteOffset;
using editor::ByteRange;
using editor::PieceTable;

ByteOffset offset(std::size_t value)
{
    return ByteOffset{value};
}

ByteRange range(std::size_t start, std::size_t length)
{
    return ByteRange{offset(start), ByteLength{length}};
}

TEST_CASE("a piece table starts with and preserves original text")
{
    PieceTable document{"original text"};

    REQUIRE(document.size().value == 13);
    REQUIRE(document.to_string() == "original text");
    REQUIRE(document.copy(range(2, 5)) == "igina");
    document.validate();
}

TEST_CASE("insertion handles empty, boundary, and interior positions")
{
    PieceTable empty;
    empty.insert(offset(0), "hello");
    REQUIRE(empty.to_string() == "hello");

    PieceTable document{"world"};
    document.insert(offset(0), "hello ");
    document.insert(offset(document.size().value), "!");
    document.insert(offset(6), "wide ");

    REQUIRE(document.to_string() == "hello wide world!");
    document.validate();
}

TEST_CASE("erasure handles one piece, multiple pieces, and an entire document")
{
    PieceTable document{"abcdef"};
    document.insert(offset(3), "XYZ");
    document.erase(range(2, 5));

    REQUIRE(document.to_string() == "abef");
    document.erase(range(0, document.size().value));
    REQUIRE(document.to_string().empty());
    document.erase(range(0, 0));
    document.validate();
}

TEST_CASE("empty edits are no-ops")
{
    PieceTable document{"text"};
    document.insert(offset(2), "");
    document.erase(range(4, 0));

    REQUIRE(document.to_string() == "text");
    document.validate();
}

TEST_CASE("random edit sequences agree with std::string after every operation")
{
    std::minstd_rand generator{0xC0FFEE};
    constexpr std::string_view alphabet{"abc def\\nXYZ"};

    for (int sequence = 0; sequence < 250; ++sequence) {
        PieceTable document;
        std::string reference;

        for (int operation = 0; operation < 300; ++operation) {
            const bool should_insert = reference.empty() || generator() % 2 == 0;
            if (should_insert) {
                const std::size_t position = generator() % (reference.size() + 1);
                const std::size_t length = generator() % 8;
                std::string text;
                text.reserve(length);
                for (std::size_t character = 0; character < length; ++character) {
                    text.push_back(alphabet[generator() % alphabet.size()]);
                }

                document.insert(offset(position), text);
                reference.insert(position, text);
            } else {
                const std::size_t start = generator() % (reference.size() + 1);
                const std::size_t length = generator() % (reference.size() - start + 1);

                document.erase(range(start, length));
                reference.erase(start, length);
            }

            REQUIRE(document.to_string() == reference);
            REQUIRE(document.size().value == reference.size());
            document.validate();
        }
    }
}

} // namespace
