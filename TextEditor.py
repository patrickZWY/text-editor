import tkinter as tk
from tkinter import filedialog, simpledialog, messagebox
import PieceTable as pt
import re
import TextEditorStack as tes

"""
features to implement, redo, need to handle other special keys, like control + others
And maybe fix undo cursor index after undoing?
"""

class TextEditor:
    def __init__(self, root) -> None:
        self.root = root
        self.text_space = tk.Text(root)
        self.text_space.pack(expand=True, fill=tk.BOTH)
        self.undo_stack = tes.TextEditorStack()
        self.text_editor = pt.PieceTable("")

        self.text_space.bind("<KeyPress>", self.on_key_press)
        self.text_space.bind("<BackSpace>", self.on_backspace_key)
        self.text_space.bind("<Delete>", self.on_delete_key)

        self.setup_menu()

        # must bind keyboard shortcuts to text_space instead of root, or it won't work
        self.text_space.bind('<Control-u>', lambda event: self.undo_once())
        self.text_space.bind('<Control-o>', lambda event: self.open_file())
        self.text_space.bind('<Control-s>', lambda event: self.save_file())
        self.text_space.bind('<Control-f>', lambda event: self.search_text())
        self.text_space.bind('<Control-h>', lambda event: self.clear_highlights())
        self.text_space.bind('<Control-q>', lambda event: self.root.destroy())

    def setup_menu(self):
        menu_bar = tk.Menu(self.root)
        self.root.config(menu=menu_bar)

        file_menu = tk.Menu(menu_bar, tearoff=0)
        menu_bar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save As...", command=self.save_file)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.destroy)

        edit_menu = tk.Menu(menu_bar, tearoff=0)
        menu_bar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Search", command=self.search_text)

        clear_button = tk.Button(self.root, text="Clear Highlights", command=self.clear_highlights)
        clear_button.pack()

    def convert_to_plain_index(self, tk_index):
        line, char = map(int, tk_index.split('.'))
        content_up_to_line = self.text_space.get("1.0", f"{line}.0")
        plain_index = len(content_up_to_line) + char
        return plain_index

    def convert_to_tk_index(self, plain_index):
        text_up_to_index = self.text_editor.form_text()[:plain_index]
        line = text_up_to_index.count("\n") + 1
        last_newline = text_up_to_index.rfind("\n")
        char = plain_index - (last_newline + 1)
        return f"{line}.{char}"

    def on_key_press(self, event):
        if event.keysym == "Return" or event.keysym == "Enter":
            character = '\n'
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)

            cursor_index = self.text_space.index(tk.INSERT)
            plain_index = self.convert_to_plain_index(cursor_index)
            self.insert_text_at_cursor(character, plain_index)
            return "break"
        elif event.char and event.keysym not in ["BackSpace", "Delete"]:
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)

            cursor_index = self.text_space.index(tk.INSERT)
            plain_index = self.convert_to_plain_index(cursor_index)
            self.insert_text_at_cursor(event.char, plain_index)
            return "break"

    def insert_text_at_cursor(self, character, plain_index):
        self.text_editor.insert(plain_index, character)
        updated_text = self.text_editor.form_text()
        self.text_space.delete("1.0", tk.END)
        self.text_space.insert("1.0", updated_text)
        new_cursor_index = self.convert_to_tk_index(plain_index + 1)
        self.text_space.mark_set(tk.INSERT, new_cursor_index)

    def on_backspace_key(self, event):
        cursor_index = self.text_space.index(tk.INSERT)
        if cursor_index != "1.0":
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)
            plain_index = self.convert_to_plain_index(cursor_index)
            self.text_editor.delete(plain_index - 1, 1)
            updated_text = self.text_editor.form_text()
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", updated_text)
            new_cursor_index = self.convert_to_tk_index(plain_index - 1)
            self.text_space.mark_set(tk.INSERT, new_cursor_index)
        return "break"

    def on_delete_key(self, event):
        cursor_index = self.text_space.index(tk.INSERT)
        
        current_text = self.text_space.get("1.0", tk.END)
        self.undo_stack.push(current_text)

        plain_index = self.convert_to_plain_index(cursor_index)
        self.text_editor.delete(plain_index, 1)
        updated_text = self.text_editor.form_text()
        self.text_space.delete("1.0", tk.END)
        self.text_space.insert("1.0", updated_text)
        new_cursor_index = self.convert_to_tk_index(plain_index)
        self.text_space.mark_set(tk.INSERT, new_cursor_index)
        return "break"

    def undo_once(self, event=None):
        if not self.undo_stack.is_empty():
            previous_text = self.undo_stack.pop()
            self.text_editor.load(previous_text)
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", previous_text)

            if previous_text.endswith("\n"):
                lines = previous_text.splitlines()
                if lines:                    
                    cursor_position = f"{len(lines)}.{len(lines[-1])}"
                else:
                    cursor_position = "1.0"
            else:
                cursor_position = self.text_space.index(tk.INSERT)
            self.text_space.mark_set(tk.INSERT, cursor_position)

    def open_file(self, event=None):
        path = filedialog.askopenfilename()
        if path:
            with open(path, "r") as file:
                content = file.read()
                self.text_space.delete(1.0, tk.END)
                self.text_space.insert(1.0, content)
                self.text_editor.load(content)
        else:
            messagebox.showerror("Error", "File Selection Failed.")

    def save_file(self, event=None):
        path = filedialog.asksaveasfilename()
        if path:
            with open(path, "w") as file:
                file.write(self.text_space.get(1.0, tk.END))
        else:
            messagebox.showerror("Error", "File Selection Failed.")

    def search_text(self, event=None):
        self.text_space.tag_remove("found", "1.0", tk.END)
        pattern = simpledialog.askstring("Search", "Enter the search/pattern: ")
        if pattern:
            regex = re.compile(pattern)
            start_index = "1.0"
            while True:
                match = regex.search(self.text_space.get(start_index, tk.END))
                if not match:
                    break
                start_char = match.start()
                end_char = match.end()
                start_pos = f"{start_index}+{start_char}c"
                end_pos = f"{start_index}+{end_char}c"
                self.text_space.tag_add("found", start_pos, end_pos)
                start_index = end_pos
            self.text_space.tag_config("found", foreground="red")

    def clear_highlights(self):
        self.text_space.tag_remove("found", "1.0", tk.END)

# Main application setup
root = tk.Tk()
root.title("Mini Text Editor III")
text_editor_app = TextEditor(root)
root.mainloop()