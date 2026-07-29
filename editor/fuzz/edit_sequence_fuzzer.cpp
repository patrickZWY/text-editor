#include "core/editor_model.hpp"
#include "core/utf8.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
    editor::EditorModel model;
    std::string reference;
    std::vector<std::string> undo;
    std::vector<std::string> redo;
    constexpr std::array<std::string_view, 6> text_fragments{
        "a", "Z", " ", "\n", "\xC3\xA9", "\xF0\x9F\x99\x82",
    };

    std::size_t index = 0;
    while (index < size)
    {
        const std::uint8_t opcode = data[index++] % 5;
        if (opcode == 0 && index < size)
        {
            const std::size_t code_point_count = editor::utf8::code_point_count(reference);
            const std::size_t position =
                editor::utf8::byte_offset_for_code_point(reference, data[index++] % (code_point_count + 1));
            const std::size_t length = index < size ? data[index++] % 8 : 0;
            std::string text;
            for (std::size_t count = 0; count < length && index < size; ++count)
            {
                text.append(text_fragments[data[index++] % text_fragments.size()]);
            }
            if (!text.empty())
            {
                undo.push_back(reference);
                redo.clear();
            }
            model.set_selection({editor::ByteOffset{position}, editor::ByteOffset{position}});
            model.insert_text(text);
            reference.insert(position, text);
        }
        else if (opcode == 1 && !reference.empty() && index < size)
        {
            const std::size_t code_point_count = editor::utf8::code_point_count(reference);
            const std::size_t start_code_point = data[index++] % code_point_count;
            const std::size_t length = index < size ? data[index++] % (code_point_count - start_code_point + 1) : 0;
            const std::size_t start = editor::utf8::byte_offset_for_code_point(reference, start_code_point);
            const std::size_t end = editor::utf8::byte_offset_for_code_point(reference, start_code_point + length);
            if (length > 0)
            {
                undo.push_back(reference);
                redo.clear();
            }
            model.set_selection({editor::ByteOffset{start}, editor::ByteOffset{end}});
            model.delete_selection();
            reference.erase(start, end - start);
        }
        else if (opcode == 2 && !undo.empty())
        {
            redo.push_back(reference);
            reference = std::move(undo.back());
            undo.pop_back();
            model.undo();
        }
        else if (opcode == 3 && !redo.empty())
        {
            undo.push_back(reference);
            reference = std::move(redo.back());
            redo.pop_back();
            model.redo();
        }
        else if (opcode == 4)
        {
            model.document().validate();
        }
        model.document().validate();
        if (model.document().to_string() != reference)
        {
            __builtin_trap();
        }
    }
    return 0;
}
