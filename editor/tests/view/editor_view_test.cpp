#include "view/editor_view.hpp"

#include <QTest>

class EditorViewTest final : public QObject
{
    Q_OBJECT

  private slots:
    void typing_and_undo_updates_the_model()
    {
        editor::EditorView view;
        view.resize(640, 480);
        view.show();
        view.setFocus();

        QTest::keyClicks(&view, "hello");
        QCOMPARE(QString::fromStdString(view.model().document().to_string()), QString{"hello"});
        QTest::keyClick(&view, Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(QString::fromStdString(view.model().document().to_string()), QString{});
    }

    void typing_replaces_the_selection()
    {
        editor::EditorView view;
        view.model().load("abc");
        view.model().set_selection({editor::ByteOffset{1}, editor::ByteOffset{3}});
        view.show();
        view.setFocus();

        QTest::keyClicks(&view, "X");
        QCOMPARE(QString::fromStdString(view.model().document().to_string()), QString{"aX"});
    }
};

QTEST_MAIN(EditorViewTest)
#include "editor_view_test.moc"
