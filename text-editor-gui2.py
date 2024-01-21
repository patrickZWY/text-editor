import tkinter as tk
from tkinter import filedialog, simpledialog, messagebox
import PieceTable as pt

def convert_to_plain_index(tk_index):
    line, char = map(int, tk_index.split('.'))
    content_up_to_line = text_space.get("1.0", f"{line}.0")
    plain_index = len(content_up_to_line) + char
    return plain_index - 1

def on_key_press(event):
    if event.char and event.keysym not in ["BackSpace", "Delete"]:
        insert_text_at_cursor(event.char)

def insert_text_at_cursor(character):
    cursor_index = text_space.index(tk.INSERT)
    plain_index = convert_to_plain_index(cursor_index)
    text_editor.insert(plain_index, character)

def on_backspace_key(event):
    cursor_index = text_space.index(tk.INSERT)
    if cursor_index != "1.0":
        plain_index = convert_to_plain_index(cursor_index)
        text_editor.delete(plain_index - 1, 1)
        text_space.delete("insert -1 chars", cursor_index)
    return "break"

def on_delete_key(event):
    cursor_index = text_space.index(tk.INSERT)
    plain_index = convert_to_plain_index(cursor_index)
    text_editor.delete(plain_index, 1)
    text_space.delete(cursor_index, "insert +1 chars")
    return "break"

def open_file():
    path = filedialog.askopenfilename()
    if path:
        with open(path, "r") as file:
            content = file.read()
            text_space.delete(1.0, tk.END)
            text_space.insert(1.0, content)
            text_editor.load(content)

def save_file():
    path = filedialog.asksaveasfilename()
    if path:
        with open(path, "w") as file:
            file.write(text_space.get(1.0, tk.END))

root = tk.Tk()
root.title("Mini Text Editor II")

text_space = tk.Text(root)
text_space.pack(expand=True, fill=tk.BOTH)

text_space.bind("<KeyPress>", on_key_press)
text_space.bind("<BackSpace>", on_backspace_key)
text_space.bind("<Delete>", on_delete_key)

menu_bar = tk.Menu(root)
root.config(menu=menu_bar)

file_menu = tk.Menu(menu_bar, tearoff=0)
menu_bar.add_cascade(label="File", menu=file_menu)
file_menu.add_command(label="Open", command=open_file)
file_menu.add_command(label="Save As...", command=save_file)
file_menu.add_separator()
file_menu.add_command(label="Exit", command=root.destroy)

text_editor = pt.PieceTable("")
root.mainloop()