import PieceTable as pt

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

    # test insertions in order
    ordered_pieceTable = pt.PieceTable("")
    ordered_pieceTable.insert(0,"1")
    assert ordered_pieceTable.form_text() == "1", "1 failed"
    ordered_pieceTable.insert(1, "2")
    assert ordered_pieceTable.form_text() == "12", "2 failed"
    ordered_pieceTable.insert(2, "3")
    assert ordered_pieceTable.form_text() == "123", "3 failed"
    ordered_pieceTable.insert(3, "4")
    # this somehow output 1243 instead of 1234
    assert ordered_pieceTable.form_text() == "1234", "4 failed"
    ordered_pieceTable.insert(4, "5")
    assert ordered_pieceTable.form_text() == "12345", "5 failed"
    print("ordered passed")

def test_delete():
    # delete from beginning
    a_pieceTable = pt.PieceTable("Meow!")
    a_pieceTable.delete(0,1)
    assert a_pieceTable.form_text() == "eow!"
    print("beginning deleted")

    # delete from middle
    a_pieceTable.delete(1,1)
    assert a_pieceTable.form_text() == "ew!"
    print("middle deleted")

    # multi pieces delete
    a_pieceTable.delete(0,1)
    assert a_pieceTable.form_text() == "w!"
    print("multi deleted")

    # delete from ending
    a_pieceTable.delete(1,1)
    assert a_pieceTable.form_text() == "w"
    print("ending deleted")

    # delete all
    a_pieceTable.delete(0,1)
    assert a_pieceTable.form_text() == ""
    print("all deleted")

    # delete none
    a_pieceTable.delete(0,0)
    assert a_pieceTable.form_text() == ""
    print("delete nothing")

    # failed deletion
    a_pieceTable.delete(2,1)
    assert a_pieceTable.form_text() == ""
    print("unrealized deletion")

    # delete the added
    another_pieceTable = pt.PieceTable("")
    another_pieceTable.insert(0, "I love apple pies!")
    another_pieceTable.delete(7, 6)
    assert another_pieceTable.form_text() == "I love pies!"
    print("delete then added")

def main():
    test_insert()
    test_delete()

if __name__ == "__main__":
    main()