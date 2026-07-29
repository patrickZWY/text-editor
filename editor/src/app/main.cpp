#include "view/editor_view.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>

int main(int argc, char* argv[])
{
    QApplication application{argc, argv};
    QMainWindow window;
    auto* editor = new editor::EditorView{&window};
    window.setCentralWidget(editor);
    window.resize(900, 650);
    window.setWindowTitle("Text Editor");

    auto* file_menu = window.menuBar()->addMenu("&File");
    auto* open = file_menu->addAction("&Open...");
    auto* save = file_menu->addAction("&Save As...");
    QObject::connect(open, &QAction::triggered, [&] {
        const QString path = QFileDialog::getOpenFileName(&window);
        if (!path.isEmpty()) {
            if (const auto result = editor->load_file(path); !result) {
                QMessageBox::critical(&window, "Open failed", QString::fromStdString(result.error()));
            }
        }
    });
    QObject::connect(save, &QAction::triggered, [&] {
        const QString path = QFileDialog::getSaveFileName(&window);
        if (!path.isEmpty()) {
            if (const auto result = editor->save_file(path); !result) {
                QMessageBox::critical(&window, "Save failed", QString::fromStdString(result.error()));
            }
        }
    });
    file_menu->addSeparator();
    file_menu->addAction("E&xit", &window, &QWidget::close);

    window.show();
    return application.exec();
}
