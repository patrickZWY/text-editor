import PieceTable as pt
from Piece import Piece

def test_insert():
    # insert into an empty string
    a_pieceTable = pt.PieceTable("")
    a_pieceTable.insert(0, "Hello")
    assert a_pieceTable.form_text() == "Hello"
    print("empty string inserted")

    # insert into the beginning
    second_pieceTable = pt.PieceTable(" World!")
    second_pieceTable.insert(0, "Hello,")
    assert second_pieceTable.form_text() == "Hello, World!"
    print("beginning inserted")

    # insert into the middle
    third_pieceTable = pt.PieceTable("It's a big world.")
    third_pieceTable.insert(11, "big ")
    assert third_pieceTable.form_text() == "It's a big big world."
    print("middle inserted")

    # insert into the ending
    fourth_pieceTable = pt.PieceTable("I like ice")
    fourth_pieceTable.insert(10, " cream.")
    assert fourth_pieceTable.form_text() == "I like ice cream."
    print("ending inserted")

    # test multiple insertions
    multiple_pieceTable = pt.PieceTable("I have")
    multiple_pieceTable.insert(0, "So ")
    assert multiple_pieceTable.form_text() == "So I have", "1 failed"
    multiple_pieceTable.insert(9, " many")
    assert multiple_pieceTable.form_text() == "So I have many", "2 failed"
    multiple_pieceTable.insert(14, " friends.")
    assert multiple_pieceTable.form_text() == "So I have many friends.", "3 failed"
    multiple_pieceTable.insert(5, "don't ")
    assert multiple_pieceTable.form_text() == "So I don't have many friends.", "4 failed"
    print("multi passed")

def test_delete():
    # delete from beginning
    a_pieceTable = pt.PieceTable("Meow!")
    a_pieceTable.delete(0,1)
    assert a_pieceTable.form_text() == "eow!"
    print("beginning deleted")
    a_pieceTable.delete(1,1)
    assert a_pieceTable.form_text() == "ew!"
    print("middle deleted")

def main():
    test_insert()
    test_delete()

if __name__ == "__main__":
    main()