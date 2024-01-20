import tkinter as tk
import PieceTable as pt
import Piece as Piece
from tkinter import filedialog
from tkinter import simpledialog
from tkinter import messagebox

root = tk.Tk()
root.title("Mini Text Editor")

text_space = tk.Text(root)
text_space.pack(expand=True, fill=tk.BOTH)

menu_bar = tk.Menu(root)
root.config(menu=menu_bar)

file_menu = tk.Menu(menu_bar, tearoff=0)
menu_bar.add_cascade(label="File", menu=file_menu)
file_menu.add_command(label="Open", command=open_file)
file_menu.add_command(label="Save As...", command=save_file)
file_menu.add_separator()
file_menu.add_command(label="Exit", command=root.destroy)

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

def insert_text():
    text_to_insert = simpledialog.askstring("Input", "Enter text to insert:")
    # range of insertion determined by Tkinter
    if text_to_insert is not None:
        cursor_index = text_space.index(tk.INSERT)

        # Tkinter Text indices conversion
        plain_index = convert_to_plain_index(cursor_index)

        text_editor.insert(plain_index, text_to_insert)
        text_space.insert(cursor_index, text_to_insert)

def delete_text():
    if text_space.tag_ranges(tk.SEL):
        # range of deletion determined by Tkinter
        start_index = text_space.index(tk.SEL_FIRST)
        end_index = text_space.index(tk.SEL_LAST)
        
        plain_start, plain_end = convert_to_plain_index(start_index), convert_to_plain_index(end_index)
        length_to_delete = plain_end - plain_start
        
        text_editor.delete(plain_start, length_to_delete)
        text_space.delete(start_index, end_index)
    else:
        messagebox.showinfo("Info", "No text selected")


def convert_to_plain_index(tk_index):
    line, char = map(int, tk_index.split('.'))
    return calculate_plain_index(line, char)

def calculate_plain_index(line, char):
    content_up_to_line = text_space.get("1.0", f"{line}.0")
    plain_index = len(content_up_to_line) + char
    return plain_index - 1
    
def load_text(text):
    text_editor.load(text)
    text_space.delete(1.0, tk.END)
    text_space.insert(1.0, text)

text_editor = pt.PieceTable("")

menu_bar.add_command(label="Insert Text", command=insert_text)
menu_bar.add_command(label="Delete Text", command=delete_text)

root.mainloop()
