from Piece import Piece

class PieceTable:
    """A PieceTable has pieces of Piece instances, original and add buffers"""
    """Documentation for insertion needed"""

    def __init__(self, text):
        self.original_buffer = text
        self.add_buffer = ""
        # a list to store each piece
        # initialize with an original piece
        self.pieces = [Piece(0, len(text), "original")]

    def insert(self, index, text):
        add_buffer_index = len(self.add_buffer)
        self.add_buffer += text

        new_pieces = []
        text_inserted = False
        current_index = 0  # Keeps track of the index within the combined text

        for piece in self.pieces:
            if text_inserted or current_index + piece.length < index:
                # If text already inserted or the current piece is before the insertion point
                new_pieces.append(piece)
            else:
                # We are at the piece where insertion happens
                # Calculate where to split this piece (if needed)
                split_index = index - current_index

                # Add the first part of the split piece
                if split_index > 0:
                    new_pieces.append(Piece(piece.start, split_index, piece.source))

                # Add the new piece
                new_pieces.append(Piece(add_buffer_index, len(text), "add"))
                text_inserted = True

                # Add the second part of the split piece (if there's any part left)
                remaining_length = piece.length - split_index
                if remaining_length > 0:
                    new_pieces.append(Piece(piece.start + split_index, remaining_length, piece.source))

            current_index += piece.length

        # Handle the case where insertion is at the very end
        if not text_inserted:
            new_pieces.append(Piece(add_buffer_index, len(text), "add"))

        self.pieces = new_pieces



    def delete(self, start : int, length : int):
        """Delete a text at index"""
        # print(f"Before deletion: {self.form_text()}")

        # save the pieces in a list
        new_pieces = []
        remaining_length_to_delete = length
        current = 0

        for piece in self.pieces:
            # if delete by zero length
            if remaining_length_to_delete <= 0:
                new_pieces.append(piece)
                continue

            # deletion length longer than length of the piece means no deletion
            if current + piece.length < start:
                new_pieces.append(piece)
            else:
                # eff start should be able to target the effective start no matter
                # it is an already modified string or unmodified
                effective_start = max(start - current, 0)
                effective_end = min(effective_start + remaining_length_to_delete, piece.length)
                # deleting from beginning won't trigger this
                # but deleting in the middle will and splits the original piece on the
                # left side to effective_start
                if effective_start > 0:
                    new_pieces.append(Piece(piece.start, effective_start, piece.source))
                # applies for both deleting from beginning and middle
                # the right side piece that has modified start and length
                if effective_end < piece.length:
                    new_start = piece.start + effective_end
                    new_length = piece.length - effective_end
                    new_pieces.append(Piece(new_start, new_length, piece.source))
                remaining_length_to_delete -= effective_end - effective_start
            current += piece.length

        self.pieces = new_pieces
        # print(f"After deletion: {self.form_text()}")
        # print(f"Current pieces: {[str(piece) for piece in self.pieces]}")

    def form_text(self):
        """form a text based on its pieces"""
        text = ""
        for piece in self.pieces:
            if piece.source == "original":
                buffer = self.original_buffer
            else:
                buffer = self.add_buffer
            text += buffer[piece.start : (piece.start + piece.length)]
        return text
    
    def load(self, text):
        """load a text into the buffer"""
        self.original_buffer = text
        self.add_buffer = ""
        self.pieces = [Piece(0, len(text), "original")]