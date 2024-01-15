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
        """Insert text at the index"""
        """
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
            if not text_inserted and index <= piece.start + piece.length:
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
            if not text_inserted:
                index -= piece.length

        self.pieces = new_pieces
        """
        add_buffer_index = len(self.add_buffer)
        self.add_buffer += text

        new_pieces = []
        text_inserted = False

        for piece in self.pieces:
            if not text_inserted:
                if index <= piece.start + piece.length:
                    split_index = index - piece.start
                    if split_index > 0:
                        new_pieces.append(Piece(piece.start, split_index, piece.source))

                    new_pieces.append(Piece(add_buffer_index, len(text), "add"))
                    text_inserted = True

                    remaining_length = piece.length - split_index
                    if remaining_length > 0:
                        new_start = piece.start + split_index
                        new_pieces.append(Piece(new_start, remaining_length, piece.source))
                else:
                    new_pieces.append(piece)
                    index -= (piece.start + piece.length)
            else:
                # Adjust the start position of subsequent pieces in the add_buffer
                if piece.source == "add":
                    new_pieces.append(Piece(piece.start + len(text), piece.length, piece.source))
                else:
                    new_pieces.append(piece)

        self.pieces = new_pieces
        print("After insertion:")
        print("Original Buffer:", self.original_buffer)
        print("Add Buffer:", self.add_buffer)
        print("Pieces:", [(p.start, p.length, p.source) for p in self.pieces])




    
    def delete(self, start, length):
        pass

    def form_text(self):
        text = ""
        for piece in self.pieces:
            if piece.source == "original":
                buffer = self.original_buffer
            else:
                buffer = self.add_buffer
            text += buffer[piece.start : (piece.start + piece.length)]
        return text