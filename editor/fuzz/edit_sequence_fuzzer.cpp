#include "core/editor_model.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    editor::EditorModel model;
    std::string reference;
    std::vector<std::string> undo;
    std::vector<std::string> redo;

    std::size_t index = 0;
    while (index < size) {
        const std::uint8_t opcode = data[index++] % 5;
        if (opcode == 0 && index < size) {
            const std::size_t position = data[index++] % (reference.size() + 1);
            const std::size_t length = index < size ? data[index++] % 16 : 0;
            std::string text;
            for (std::size_t count = 0; count < length && index < size; ++count) {
                text.push_back(static_cast<char>(data[index++]));
            }
            if (length > 0) {
                undo.push_back(reference);
                redo.clear();
            }
            model.set_selection({editor::ByteOffset{position}, editor::ByteOffset{position}});
            model.insert_text(text);
            reference.insert(position, text);
        } else if (opcode == 1 && !reference.empty() && index < size) {
            const std::size_t start = data[index++] % reference.size();
            const std::size_t length = index < size ? data[index++] % (reference.size() - start + 1) : 0;
            if (length > 0) {
                undo.push_back(reference);
                redo.clear();
            }
            model.set_selection({editor::ByteOffset{start}, editor::ByteOffset{start + length}});
            model.delete_selection();
            reference.erase(start, length);
        } else if (opcode == 2 && !undo.empty()) {
            redo.push_back(reference);
            reference = std::move(undo.back());
            undo.pop_back();
            model.undo();
        } else if (opcode == 3 && !redo.empty()) {
            undo.push_back(reference);
            reference = std::move(redo.back());
            redo.pop_back();
            model.redo();
        } else if (opcode == 4) {
            model.document().validate();
        }
        model.document().validate();
        if (model.document().to_string() != reference) {
            __builtin_trap();
        }
    }
    return 0;
}
