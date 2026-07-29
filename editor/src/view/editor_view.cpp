#include "view/editor_view.hpp"

#include "view/qt_text_conversion.hpp"

#include <QFile>
#include <QFontDatabase>
#include <QInputMethodEvent>
#include <QKeySequence>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

#include <algorithm>

namespace editor {

EditorView::EditorView(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    update_scrollbars();
}

EditorModel& EditorView::model() noexcept { return model_; }
const EditorModel& EditorView::model() const noexcept { return model_; }

std::expected<void, std::string> EditorView::load_file(const QString& path)
{
    QFile file{path};
    if (!file.open(QIODevice::ReadOnly)) {
        return std::unexpected(file.errorString().toStdString());
    }
    const QByteArray bytes = file.readAll();
    model_.load(std::string{bytes.constData(), static_cast<std::size_t>(bytes.size())});
    document_changed();
    return {};
}

std::expected<void, std::string> EditorView::save_file(const QString& path) const
{
    QFile file{path};
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return std::unexpected(file.errorString().toStdString());
    }
    const std::string text = model_.document().to_string();
    if (file.write(text.data(), static_cast<qint64>(text.size())) != static_cast<qint64>(text.size())) {
        return std::unexpected(file.errorString().toStdString());
    }
    return {};
}

void EditorView::paintEvent(QPaintEvent*)
{
    QPainter painter{viewport()};
    painter.fillRect(viewport()->rect(), palette().base());
    const int height = line_height();
    const std::size_t first = static_cast<std::size_t>(verticalScrollBar()->value() / height);
    const std::size_t count = static_cast<std::size_t>(viewport()->height() / height + 2);
    const Selection selection = model_.selection();
    const ByteRange selected = selection.range();

    for (std::size_t line = first; line < model_.line_index().line_count() && line < first + count; ++line) {
        const ByteRange range = model_.line_index().line_range(line);
        const auto layout = layout_for_line(line);
        QVector<QTextLayout::FormatRange> formats;
        const std::size_t selection_start = std::max(range.start.value, selected.start.value);
        const std::size_t selection_end = std::min(range.start.value + range.length.value, selected.start.value + selected.length.value);
        if (selection_start < selection_end) {
            const std::string bytes = model_.document().copy(range);
            QTextLayout::FormatRange format;
            format.start = qt::utf16_offset_for_byte(bytes, selection_start - range.start.value);
            format.length = qt::utf16_offset_for_byte(bytes, selection_end - range.start.value) - format.start;
            format.format.setBackground(palette().highlight());
            format.format.setForeground(palette().highlightedText());
            formats.push_back(format);
        }
        const qreal y = static_cast<qreal>(line * height - verticalScrollBar()->value());
        layout->draw(&painter, QPointF{0, y}, formats);
    }

    if (hasFocus() && preedit_.isEmpty()) {
        painter.fillRect(cursor_rect(), palette().text());
    }
    if (!preedit_.isEmpty()) {
        painter.drawText(cursor_rect().bottomLeft(), preedit_);
    }
}

void EditorView::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    update_scrollbars();
}

void EditorView::keyPressEvent(QKeyEvent* event)
{
    const bool extend = event->modifiers().testFlag(Qt::ShiftModifier);
    if (event->matches(QKeySequence::Undo)) {
        model_.undo();
    } else if (event->matches(QKeySequence::Redo)) {
        model_.redo();
    } else if (event->key() == Qt::Key_Left) {
        model_.move_left(extend);
    } else if (event->key() == Qt::Key_Right) {
        model_.move_right(extend);
    } else if (event->key() == Qt::Key_Up) {
        model_.move_up(extend);
    } else if (event->key() == Qt::Key_Down) {
        model_.move_down(extend);
    } else if (event->key() == Qt::Key_Backspace) {
        model_.backspace();
    } else if (event->key() == Qt::Key_Delete) {
        model_.delete_forward();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        model_.insert_text("\n");
    } else if (event->key() == Qt::Key_Tab) {
        model_.insert_text("\t");
    } else if (!event->text().isEmpty() && !event->modifiers().testFlag(Qt::ControlModifier) && !event->modifiers().testFlag(Qt::MetaModifier)) {
        const QByteArray utf8 = event->text().toUtf8();
        model_.insert_text(std::string_view{utf8.constData(), static_cast<std::size_t>(utf8.size())});
    } else {
        QAbstractScrollArea::keyPressEvent(event);
        return;
    }
    document_changed();
}

void EditorView::mousePressEvent(QMouseEvent* event)
{
    selecting_ = true;
    const ByteOffset offset = offset_at(event->position());
    model_.set_selection({offset, offset});
    setFocus();
    viewport()->update();
}

void EditorView::mouseMoveEvent(QMouseEvent* event)
{
    if (!selecting_ || !(event->buttons() & Qt::LeftButton)) {
        return;
    }
    auto selection = model_.selection();
    selection.cursor = offset_at(event->position());
    model_.set_selection(selection);
    viewport()->update();
}

void EditorView::inputMethodEvent(QInputMethodEvent* event)
{
    preedit_ = event->preeditString();
    if (!event->commitString().isEmpty()) {
        const QByteArray utf8 = event->commitString().toUtf8();
        model_.insert_text(std::string_view{utf8.constData(), static_cast<std::size_t>(utf8.size())});
        document_changed();
    } else {
        viewport()->update();
    }
    event->accept();
}

QVariant EditorView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch (query) {
    case Qt::ImCursorRectangle: return cursor_rect();
    case Qt::ImSurroundingText: return qt::to_qstring(model_.document().to_string());
    case Qt::ImCursorPosition: return qt::utf16_offset_for_byte(model_.document().to_string(), model_.selection().cursor.value);
    case Qt::ImAnchorPosition: return qt::utf16_offset_for_byte(model_.document().to_string(), model_.selection().anchor.value);
    default: return QAbstractScrollArea::inputMethodQuery(query);
    }
}

int EditorView::line_height() const { return fontMetrics().height(); }

std::size_t EditorView::line_at_y(qreal y) const
{
    const std::size_t line = static_cast<std::size_t>((y + verticalScrollBar()->value()) / line_height());
    return std::min(line, model_.line_index().line_count() - 1);
}

ByteOffset EditorView::offset_at(const QPointF& point) const
{
    const std::size_t line = line_at_y(point.y());
    const ByteRange range = model_.line_index().line_range(line);
    const auto layout = layout_for_line(line);
    const QTextLine text_line = layout->lineAt(0);
    const int utf16 = text_line.xToCursor(point.x() + horizontalScrollBar()->value());
    const std::string text = model_.document().copy(range);
    return ByteOffset{range.start.value + qt::byte_offset_for_utf16(text, utf16)};
}

QRect EditorView::cursor_rect() const
{
    const std::size_t line = model_.line_index().line_of(model_.selection().cursor);
    const ByteRange range = model_.line_index().line_range(line);
    const std::string text = model_.document().copy(range);
    const int utf16 = qt::utf16_offset_for_byte(text, model_.selection().cursor.value - range.start.value);
    const qreal x = layout_for_line(line)->lineAt(0).cursorToX(utf16) - horizontalScrollBar()->value();
    const qreal y = line * line_height() - verticalScrollBar()->value();
    return QRect{static_cast<int>(x), static_cast<int>(y), 1, line_height()};
}

QSharedPointer<QTextLayout> EditorView::layout_for_line(std::size_t line) const
{
    const ByteRange range = model_.line_index().line_range(line);
    return layouts_.layout_for(model_.revision(), line, qt::to_qstring(model_.document().copy(range)), font());
}

void EditorView::update_scrollbars()
{
    const int height = line_height();
    verticalScrollBar()->setRange(0, std::max(0, static_cast<int>(model_.line_index().line_count()) * height - viewport()->height()));
    verticalScrollBar()->setPageStep(viewport()->height());
    horizontalScrollBar()->setRange(0, 0);
}

void EditorView::document_changed()
{
    layouts_.clear();
    update_scrollbars();
    viewport()->update();
    updateMicroFocus();
}

} // namespace editor
