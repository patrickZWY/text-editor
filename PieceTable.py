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

        # the newly added text begins at add_buffer_index within add buffer
        add_buffer_index = len(self.add_buffer)
        self.add_buffer += text

        # to avoid affecting the index counting, we append to a new list
        # instead of modifying the original list
        new_pieces = []
        text_inserted = False

        # iterate through every piece in the list of pieces
        for piece in self.pieces:

            # SOMETHING HERE IS WRONG WHEN ADD BUFFER IS NOT SUPPOSED TO BE SPLIT, IT WAS SPLIT
            # FOR MANY, MANY WAS RIGHT (3, 5, 'add') BUT IT WAS SPLIT WHICH SHOULDN'T HAPPEN
            # essentially the problem was that checking index <= start + length will lead to
            # checking not the ending of a line 
            # check if current piece is affected by insertion
            # this is actually ok, bc it does check that many is affected
            # the problem is what next?
            """
            piece.start + piece.length doesn't check for the end of a piece, 
            in fact, every time the length will be decremented by a certain amount, 
            and when it goes to check for next piece, the remaining amount will be less
            than piece start and length together, therefore making program think that we
            are dealing with inserting in the middle instead of inserting in the end
            """
            if not text_inserted and index <= piece.length + piece.start:
                if index != 0 and index == piece.length:
                    new_pieces.append(piece)
                    new_pieces.append(Piece(add_buffer_index, len(text), "add"))
                    text_inserted = True
                    break

                # split_index is the location of insertion
                split_index = index - piece.start
                # if we are not inserting from the beginning
                if split_index > 0:
                # BUT WHERE IS THE CHECK FOR ENDING, ENDING IS DIFF FROM MIDDLE???
                # when the sample is small, like only one original, it is easy to add
                # to ending, but when multiple adds are present, using split_index is no
                # longer feasible, partly due to the inaccurate measuring of index
                # when inserting " friends." it split many which should not have happened
                    
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