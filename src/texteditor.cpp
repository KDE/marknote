#include "texteditor.h"

#include <QQuickTextDocument>

TextEditor::TextEditor(QObject *parent) : QObject(parent)
{

}

QQuickTextDocument *TextEditor::document() const
{
    return m_document;
}

void TextEditor::setDocument(QQuickTextDocument *document)
{
    m_document = document;

    m_cursor = QTextCursor(document->textDocument());

    Q_EMIT documentChanged();
}

void TextEditor::makeSelectionItalic()
{

}

void TextEditor::onCursorPositionChanged(int position)
{
    m_cursor.setPosition(position);

    auto format = QTextBlockFormat();
    format.setHeadingLevel(1);

    auto charFormat = QTextCharFormat();
    charFormat.setFontWeight(30);

    //m_cursor.insertBlock(format);
    m_cursor.insertText("moin");
}
