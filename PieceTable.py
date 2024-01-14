import Piece

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
        # the newly added text begins at add_buffer_index within add buffer
        add_buffer_index = len(self.add_buffer)
        self.add_buffer += text

        # iterate through every piece in the list of pieces
        for i, piece in enumerate(self.pieces):
            # check if current piece is affected by insertion
            if index <= piece.start + piece.length:
                # split_index is the location of insertion
                split_index = index - piece.start
                # if we are not inserting from the beginning
                if split_index > 0:
                    # we need to split the current piece at the locus of insertion
                    # the original piece now stops at the point of insertion
                    self.pieces[i] = Piece(piece.start, split_index, piece.source)
                    # increment i for the new piece later
                    i += 1
                # the add_buffer_index is the location within the add buffer string
                # that begins with our added string
                # we need this because we need to distinguish between different add ops
                new_piece = Piece(add_buffer_index, len(text), "add")
                self.pieces.insert(i, new_piece)
                # if we are not inserting at the end, this will be true
                # so for both inserting at the start and inserting in-between
                # we need a new piece that comes after the inserted piece
                if split_index < piece.length:
                    self.pieces.insert(i + 1, Piece(piece.start + split_index, \
                                                    piece.length - split_index, \
                                                    piece.source))
                # if current piece is affected and done with split, end the loop
                break
            # if the insertion point is beyond original text, move to next piece
            index -= piece.length






    
    def delete(self, start, length):
        pass

    def form_text(self):
        pass