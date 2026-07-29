import tkinter as tk
import PieceTable as pt
import Piece as Piece
from tkinter import filedialog, simpledialog, messagebox

"""
This version uses PieceTable for special insertion and deletion
"""
# create main window
root = tk.Tk()
root.title("Mini Text Editor")

# area where text is entered
text_space = tk.Text(root)
# expand and fill all window
text_space.pack(expand=True, fill=tk.BOTH)

def open_file():
    """open a file"""
    # user selects a file and this returns a string for path
    path = filedialog.askopenfilename()
    # empty if user cancels selection
    if path:
        with open(path, "r") as file:
            content = file.read()
            # clear screen
            text_space.delete(1.0, tk.END)
            text_space.insert(1.0, content)
            # load content to data structure
            text_editor.load(content)

def save_file():
    """save a file"""
    path = filedialog.asksaveasfilename()
    if path:
        with open(path, "w") as file:
            # write entire text to file
            file.write(text_space.get(1.0, tk.END))

def insert_text():
    """insert a text"""
    text_to_insert = simpledialog.askstring("Input", "Enter text to insert:")
    # range of insertion determined by Tkinter
    if text_to_insert is not None:
        # get the index at the cursor
        cursor_index = text_space.index(tk.INSERT)

        # Tkinter Text indices conversion
        plain_index = convert_to_plain_index(cursor_index)

        # insert text to piece table then to screen
        text_editor.insert(plain_index, text_to_insert)
        text_space.insert(cursor_index, text_to_insert)

def delete_text():
    """delete a text"""
    if text_space.tag_ranges(tk.SEL):
        # range of deletion determined by Tkinter
        start_index = text_space.index(tk.SEL_FIRST)
        end_index = text_space.index(tk.SEL_LAST)
        
        # get the index for data structure
        plain_start, plain_end = convert_to_plain_index(start_index), convert_to_plain_index(end_index)
        length_to_delete = plain_end - plain_start
        
        # delete text from piece table then to screen
        text_editor.delete(plain_start, length_to_delete)
        text_space.delete(start_index, end_index)
    else:
        messagebox.showinfo("Info", "No text selected")


def convert_to_plain_index(tk_index):
    """convert cursor index to piece table index"""
    line, char = map(int, tk_index.split('.'))
    return calculate_plain_index(line, char)

def calculate_plain_index(line, char):
    """convert piece table index to cursor index"""
    # get all the text from beginning to the beginning of selected line
    content_up_to_line = text_space.get("1.0", f"{line}.0")
    # add the len of it and the number of chars
    plain_index = len(content_up_to_line) + char
    # python starting from index 0
    return plain_index - 1
    
def load_text(text):
    """load a text"""
    text_editor.load(text)
    text_space.delete(1.0, tk.END)
    text_space.insert(1.0, text)

# create a piece table object for our text editor
text_editor = pt.PieceTable("")

# create a menu bar
menu_bar = tk.Menu(root)
# add menu bar to window
root.config(menu=menu_bar)

# create file menu
file_menu = tk.Menu(menu_bar, tearoff=0)
# add submenu
menu_bar.add_cascade(label="File", menu=file_menu)
file_menu.add_command(label="Open", command=open_file)
file_menu.add_command(label="Save As...", command=save_file)
file_menu.add_separator()
file_menu.add_command(label="Exit", command=root.destroy)

menu_bar.add_command(label="Insert Text", command=insert_text)
menu_bar.add_command(label="Delete Text", command=delete_text)

# start event loop
root.mainloop()
