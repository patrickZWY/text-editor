#pragma once

#include "core/coordinates.hpp"

#include <QFont>
#include <QHash>
#include <QSharedPointer>
#include <QTextLayout>

namespace editor
{

class LineLayoutCache
{
  public:
    [[nodiscard]] QSharedPointer<QTextLayout> layout_for(DocumentRevision revision, std::size_t line,
                                                         const QString &text, const QFont &font)
    {
        const quint64 key = (revision.value * 1315423911ULL) ^ static_cast<quint64>(line);
        if (const auto found = entries_.find(key);
            found != entries_.end() && found->text == text && found->font == font)
        {
            return found->layout;
        }

        auto layout = QSharedPointer<QTextLayout>::create(text, font);
        layout->beginLayout();
        if (QTextLine line_layout = layout->createLine(); line_layout.isValid())
        {
            line_layout.setLineWidth(1'000'000.0);
        }
        layout->endLayout();
        entries_.insert(key, Entry{text, font, layout});
        return layout;
    }

    void clear()
    {
        entries_.clear();
    }

  private:
    struct Entry
    {
        QString text;
        QFont font;
        QSharedPointer<QTextLayout> layout;
    };

    QHash<quint64, Entry> entries_;
};

} // namespace editor
