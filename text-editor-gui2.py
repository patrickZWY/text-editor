import tkinter as tk
from tkinter import filedialog, simpledialog, messagebox
import PieceTable as pt
import re

"""
This version uses Piece Table for insertion and deletion
Has features such as delete, backspace, keyboard shortcuts managed by piece table.
"""

def convert_to_plain_index(tk_index):
    # tkinter text index is "line.char"
    line, char = map(int, tk_index.split('.'))
    # get the text from the beginning to the beginning of selected line
    content_up_to_line = text_space.get("1.0", f"{line}.0")
    # that length plus the number of chars finish locating the text
    plain_index = len(content_up_to_line) + char
    # python strings start from zero, not one as in tkinter
    return plain_index - 1

def on_key_press(event):
    # event is the info about key pressed.
    # char is the key pressed, keysym is the symbolic name of the key
    # insert the text if it is valid and not deleting commands
    if event.char and event.keysym not in ["BackSpace", "Delete"]:
        insert_text_at_cursor(event.char)

def insert_text_at_cursor(character):
    # get the current cursor index tk.INSERT
    cursor_index = text_space.index(tk.INSERT)
    # convert this to python's index
    plain_index = convert_to_plain_index(cursor_index)
    # use our data structure to process it
    text_editor.insert(plain_index, character)

def on_backspace_key(_): # event parameter not used, _ as placeholder
    """backspace key deletes the word before cursor"""
    # get the current cursor index
    cursor_index = text_space.index(tk.INSERT)
    # if not beginning
    if cursor_index != "1.0":
        # get the python index
        plain_index = convert_to_plain_index(cursor_index)
        # deletion means to start from one char before
        text_editor.delete(plain_index - 1, 1)
        # the string calculates one char before current cursor index
        text_space.delete("insert -1 chars", cursor_index)
    # stop the vent from continuing because we handled the deletion manually
    return "break"

def on_delete_key(_): # event parameter not used, _ as placeholder
    """delete key deletes the word after cursor"""
    cursor_index = text_space.index(tk.INSERT)
    plain_index = convert_to_plain_index(cursor_index)
    # deletion starts from one char after
    text_editor.delete(plain_index, 1)
    # the string calculates the index of char after the cursor 
    text_space.delete(cursor_index, "insert +1 chars")
    return "break"

def open_file():
    """open a new file"""
    # user can select a file and this returns a string of a path
    path = filedialog.askopenfilename()
    # path will be empty if user cancel selection
    if path:
        with open(path, "r") as file:
            content = file.read()
            # clear the screen and load the text to screen
            text_space.delete(1.0, tk.END)
            text_space.insert(1.0, content)
            # load the content to our data structure
            text_editor.load(content)
    else:
        messagebox.showerror("Error", "File Selection Failed.")

def save_file():
    """save a file"""
    path = filedialog.asksaveasfilename()
    if path:
        with open(path, "w") as file:
            # write the entire text to file
            file.write(text_space.get(1.0, tk.END))
    else:
        messagebox.showerror("Error", "File Selection Failed.")

def search_text():
    """find selected text with string/pattern"""
    # remove previous highlights, used if re-searching 
    text_space.tag_remove("found", "1.0", tk.END)

    pattern = simpledialog.askstring("Search", "Enter the search/pattern: ")
    if pattern:
        regex = re.compile(pattern)

        start_index = "1.0"
        while True:
            match = regex.search(text_space.get(start_index, tk.END))
            if not match:
                break
            
            # get the starting and ending index
            start_char = match.start()
            end_char = match.end()

            # convert index into tkinter index
            start_pos = f"{start_index}+{start_char}c"
            end_pos = f"{start_index}+{end_char}c"

            text_space.tag_add("found", start_pos, end_pos)
            # next part of the text for next search
            start_index = end_pos
        # highlighting searches
        text_space.tag_config("found", foreground="red")

def clear_highlights():
    text_space.tag_remove("found", "1.0", tk.END)

# create main window
root = tk.Tk()
root.title("Mini Text Editor II")

# an area where text is entered is added to window
text_space = tk.Text(root)
# expand and fill all window
text_space.pack(expand=True, fill=tk.BOTH)

# binding keypress events
text_space.bind("<KeyPress>", on_key_press)
text_space.bind("<BackSpace>", on_backspace_key)
text_space.bind("<Delete>", on_delete_key)

# create a menu bar
menu_bar = tk.Menu(root)
# add menu bar to window
root.config(menu=menu_bar)

# file menu created, tear off feature (new window) disabled 
file_menu = tk.Menu(menu_bar, tearoff=0)
# add_cascade adds submenu
menu_bar.add_cascade(label="File", menu=file_menu)
file_menu.add_command(label="Open", command=open_file)
file_menu.add_command(label="Save As...", command=save_file)
file_menu.add_separator()
file_menu.add_command(label="Exit", command=root.destroy)

# edit menu created and added
edit_menu = tk.Menu(menu_bar, tearoff=0)
menu_bar.add_cascade(label="Edit", menu=edit_menu)
edit_menu.add_command(label="Search", command=search_text)

# create a button and add it
clear_button = tk.Button(root, text="Clear Highlights", command=clear_highlights)
clear_button.pack()

# add keyboard shortcuts
root.bind('<Control-o>', lambda event: open_file())
root.bind('<Control-s>', lambda event: save_file())
root.bind('<Control-f>', lambda event: search_text())
root.bind('<Control-q>', lambda event: root.destroy())
root.bind('<Control-h>', lambda event: clear_highlights())

text_editor = pt.PieceTable("")
# start event loop
root.mainloop()
