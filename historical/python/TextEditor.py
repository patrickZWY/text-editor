import tkinter as tk
from tkinter import filedialog, simpledialog, messagebox
import PieceTable as pt
import re
import TextEditorStack as tes

"""
Syntax highlighting?
Indentation?
Copy and Paste reenable?
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

        # bind unused keys to do nothing to prevent inputting garbage 
        unused_keys = ['a', 'b', 'c', 'd', 'e', 'g', 'i', 'j', 'k'
                    , 'l', 'm', 'n', 'p', 'r', 't', 'v', 'w', 'x', 'y', 'z']
        for key in unused_keys:
            self.text_space.bind(f'<Control-{key}>', self.do_nothing)
        

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
        edit_menu.add_command(label="Insert Text", command=self.text_block_insertion)

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
        elif event.keysym == "Tab":
            character = '\t'
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
        """fix: view after rerender should not be top of the file"""
        self.text_editor.insert(plain_index, character)
        updated_text = self.text_editor.form_text()
        self.text_space.delete("1.0", tk.END)
        self.text_space.insert("1.0", updated_text)
        new_cursor_index = self.convert_to_tk_index(plain_index + 1)
        self.text_space.mark_set(tk.INSERT, new_cursor_index)

    def on_backspace_key(self, event):
        """add feature to delete single char or a group of text"""
        cursor_index = self.text_space.index(tk.INSERT)
        # delete one char when not selecting a group of text and not at the beginning 
        if cursor_index != "1.0" and not self.text_space.tag_ranges(tk.SEL):
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)

            plain_index = self.convert_to_plain_index(cursor_index)
            self.text_editor.delete(plain_index - 1, 1)

            updated_text = self.text_editor.form_text()
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", updated_text)

            new_cursor_index = self.convert_to_tk_index(plain_index - 1)
            self.text_space.mark_set(tk.INSERT, new_cursor_index)
        elif self.text_space.tag_ranges(tk.SEL):
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)
            
            start_index = self.text_space.index(tk.SEL_FIRST)
            end_index = self.text_space.index(tk.SEL_LAST)
            plain_start = self.convert_to_plain_index(start_index)
            plain_end = self.convert_to_plain_index(end_index)
            length_to_delete = plain_end - plain_start

            self.text_editor.delete(plain_start, length_to_delete)
            updated_text = self.text_editor.form_text()
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", updated_text)
            # cursor is the at the same position as the start
            # because we are deleting a block of text
            new_cursor_index = self.convert_to_tk_index(plain_start)
            self.text_space.mark_set(tk.INSERT, new_cursor_index)
        # prevent default backspace behavior
        return "break"

    def on_delete_key(self, event):
        """fix: need group deletion!"""
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
        """undo once available for this version"""
        # if there is something to undo
        if not self.undo_stack.is_empty():
            # pop it out, store it in previous_text
            previous_text = self.undo_stack.pop()
            # load the text back as new original
            self.text_editor.load(previous_text)
            # refresh screen
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", previous_text)
            # readjust the cursor position
            if previous_text.endswith("\n"):
                lines = previous_text.splitlines()
                if lines:                    
                    cursor_position = f"{len(lines)}.{len(lines[-1])}"
                else:
                    cursor_position = "1.0"
            else:
                cursor_position = self.text_space.index(tk.INSERT)
            self.text_space.mark_set(tk.INSERT, cursor_position)

    def text_block_insertion(self):
        """
        insert a block of text through menu
        need to add precautions against tab and enter and return key presses
        unicodes?
        """
        text_to_insert = simpledialog.askstring("Input", "Enter text to insert:")
        # range of insertion determined by Tkinter
        if text_to_insert is not None:
            # save text to stack
            current_text = self.text_space.get("1.0", tk.END)
            self.undo_stack.push(current_text)
            # get the index at the cursor
            cursor_index = self.text_space.index(tk.INSERT)
            plain_index = self.convert_to_plain_index(cursor_index)
            # insert text to piece table then to screen
            self.text_editor.insert(plain_index, text_to_insert)
            updated_text = self.text_editor.form_text()
            # refresh screen
            self.text_space.delete("1.0", tk.END)
            self.text_space.insert("1.0", updated_text)
            # readjust the cursor position
            new_cursor_index = self.convert_to_tk_index(plain_index + len(text_to_insert))
            self.text_space.mark_set(tk.INSERT, new_cursor_index)

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
        """fix: get control back from intensive operations"""
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
    
    def do_nothing(self, event):
        return "break"


# Main application setup
root = tk.Tk()
root.title("Mini Text Editor III")
text_editor_app = TextEditor(root)
root.mainloop()