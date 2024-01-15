class Piece:
    def __init__(self, start : int, length : int, source : str):
        self.start = start
        self.length = length
        self.source = source

    def __str__(self) -> str:
        return f"Piece(start={self.start}, length={self.length}, source={self.source})"