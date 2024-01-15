from Piece import Piece

class PieceTable:
    """A PieceTable has pieces of Piece instances, original and add buffers"""
    def __init__(self, text):
        self.original_buffer = text
        self.add_buffer = ""
        # a list to store each piece
        # initialize with an original piece
        self.pieces = [Piece(0, len(text), "original")]


    def insert(self, index, text):
        """Insert a text at index"""
        # the newly added text begins at add_buffer_index within add buffer
        add_buffer_index = len(self.add_buffer)
        self.add_buffer += text

        # to avoid affecting the index counting, we append to a new list
        # instead of modifying the original list
        new_pieces = []
        text_inserted = False

        # iterate through every piece in the list of pieces
        for piece in self.pieces:

            # check if current piece is affected by insertion
            if not text_inserted and index <= piece.length + piece.start:
                # if inserting at the ending, simply append piece and the new piece
                if index != 0 and index == piece.length:
                    new_pieces.append(piece)
                    new_pieces.append(Piece(add_buffer_index, len(text), "add"))
                    text_inserted = True
                    break

                # split_index is the location of insertion
                split_index = index - piece.start
                # if we are not inserting from the beginning
                if split_index > 0:
                    
                # Add the first part of the split piece
                # we need to split the current piece at the locus of insertion
                # the original piece now stops at the point of insertion
                    new_pieces.append(Piece(piece.start, split_index, piece.source))

                # Add the new piece
                # the add_buffer_index is the location within the add_buffer string
                # that begins with our added string
                # we need this num because we need to distinguish between different add ops
                new_pieces.append(Piece(add_buffer_index, len(text), "add"))
                text_inserted = True

                # if we are not inserting at the end, this will be true
                # so for both inserting at the start and inserting in-between
                # we need a new piece that comes after the inserted piece
                if split_index < piece.length:
                    # Add the second part of the split piece
                    new_start = piece.start + split_index
                    new_length = piece.length - split_index
                    new_pieces.append(Piece(new_start, new_length, piece.source))
            else:
                new_pieces.append(piece)

            # if the insertion point is beyond original text, move to next piece
            # and take out its equal length
            if not text_inserted:
                index -= piece.length

        self.pieces = new_pieces


    def delete(self, start : int, length : int):
        """Delete a text at index"""
        print(f"Before deletion: {self.form_text()}")

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
                # if deletion in the middle
                # something goes wrong here because it doesn't detect split
                # in the middle, at this point, eff_start is zero, and zero
                # won't pass the first condition, it should be 1 to form a
                # new piece but here it is neglected
                effective_start = max(start - current, 0)
                effective_end = min(effective_start + remaining_length_to_delete, piece.length)
                if effective_start > 0:
                    new_pieces.append(Piece(piece.start, effective_start, piece.source))
                if effective_end < piece.length:
                    new_start = piece.start + effective_end
                    new_length = piece.length - effective_end
                    new_pieces.append(Piece(new_start, new_length, piece.source))
                remaining_length_to_delete -= effective_end - effective_start
            current += piece.length

        self.pieces = new_pieces
        print(f"After deletion: {self.form_text()}")
        print(f"Current pieces: {[str(piece) for piece in self.pieces]}")







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