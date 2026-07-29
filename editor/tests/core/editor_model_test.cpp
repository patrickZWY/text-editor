#include "core/editor_model.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

using editor::ByteOffset;
using editor::EditorModel;
using editor::Selection;

TEST_CASE("model replaces a selection as one undoable edit")
{
    EditorModel model{"hello world"};
    model.set_selection(Selection{ByteOffset{6}, ByteOffset{11}});
    model.insert_text("editor");

    REQUIRE(model.document().to_string() == "hello editor");
    REQUIRE(model.selection().empty());
    REQUIRE(model.selection().cursor == ByteOffset{12});

    model.undo();
    REQUIRE(model.document().to_string() == "hello world");
    REQUIRE(model.selection().range().start == ByteOffset{6});
    REQUIRE(model.selection().range().length.value == 5);

    model.redo();
    REQUIRE(model.document().to_string() == "hello editor");
}

TEST_CASE("model backspace and forward delete update text and cursor")
{
    EditorModel model{"abc"};
    model.set_selection(Selection{ByteOffset{1}, ByteOffset{1}});
    model.backspace();
    REQUIRE(model.document().to_string() == "bc");
    REQUIRE(model.selection().cursor == ByteOffset{0});

    model.delete_forward();
    REQUIRE(model.document().to_string() == "c");
    model.undo();
    REQUIRE(model.document().to_string() == "bc");
}

TEST_CASE("model maintains a line index and byte-column vertical movement")
{
    EditorModel model{"one\nlonger\ntwo"};
    REQUIRE(model.line_index().line_count() == 3);
    model.set_selection(Selection{ByteOffset{2}, ByteOffset{2}});

    model.move_down(false);
    REQUIRE(model.selection().cursor == ByteOffset{6});
    model.move_down(false);
    REQUIRE(model.selection().cursor == ByteOffset{13});
    model.move_up(false);
    REQUIRE(model.selection().cursor == ByteOffset{6});
}

TEST_CASE("selection extension preserves anchor")
{
    EditorModel model{"abc"};
    model.set_selection(Selection{ByteOffset{1}, ByteOffset{1}});
    model.move_right(true);
    model.move_right(true);
    REQUIRE(model.selection().anchor == ByteOffset{1});
    REQUIRE(model.selection().cursor == ByteOffset{3});
    REQUIRE(model.selection().range().length.value == 2);
}

TEST_CASE("loading resets the model state and its undo history")
{
    EditorModel model{"old"};
    model.insert_text(" text");
    const auto revision_before_load = model.revision();

    model.load("new");
    REQUIRE(model.document().to_string() == "new");
    REQUIRE(model.selection().cursor == ByteOffset{0});
    REQUIRE(model.revision() > revision_before_load);

    model.undo();
    REQUIRE(model.document().to_string() == "new");
}

TEST_CASE("adjacent typing coalesces into one undo step")
{
    EditorModel model;
    model.insert_text("h");
    model.insert_text("i");
    model.insert_text("!");
    REQUIRE(model.document().to_string() == "hi!");

    model.undo();
    REQUIRE(model.document().to_string().empty());
    model.redo();
    REQUIRE(model.document().to_string() == "hi!");
}

} // namespace
