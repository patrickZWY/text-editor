#pragma once

#include "core/editor_model.hpp"
#include "view/line_layout_cache.hpp"

#include <QAbstractScrollArea>

#include <expected>
#include <string>

namespace editor
{

class EditorView final : public QAbstractScrollArea
{
  public:
    explicit EditorView(QWidget *parent = nullptr);

    [[nodiscard]] EditorModel &model() noexcept;
    [[nodiscard]] const EditorModel &model() const noexcept;
    [[nodiscard]] std::expected<void, std::string> load_file(const QString &path);
    [[nodiscard]] std::expected<void, std::string> save_file(const QString &path) const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

  private:
    [[nodiscard]] int line_height() const;
    [[nodiscard]] std::size_t line_at_y(qreal y) const;
    [[nodiscard]] ByteOffset offset_at(const QPointF &point) const;
    [[nodiscard]] QRect cursor_rect() const;
    [[nodiscard]] QSharedPointer<QTextLayout> layout_for_line(std::size_t line) const;
    void update_scrollbars();
    void document_changed();

    EditorModel model_;
    mutable LineLayoutCache layouts_;
    QString preedit_;
    bool selecting_{};
};

} // namespace editor
