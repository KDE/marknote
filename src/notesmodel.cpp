// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notesmodel.h"
#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QPdfWriter>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QUrl>

#if __has_include(<md4c-html.h>)
#include <md4c-html.h>
#endif

using namespace Qt::StringLiterals;

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
        if (!m_watcher.files().contains(path)) {
            m_watcher.addPath(path);
        }

        updateColor();
        Q_EMIT dataChanged(index(0, 0), index(rowCount({}) - 1, 0), {Role::Color});
    });
}

int NotesModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() || m_path.isEmpty() ? 0 : directory.entryList(QDir::Files).count();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Role::FileUrl:
        return QUrl::fromLocalFile(directory.entryInfoList(QDir::Files).at(index.row()).filePath());
    case Role::Path:
        return directory.entryInfoList(QDir::Files).at(index.row()).fileName();
    case Role::Date:
        return directory.entryInfoList(QDir::Files).at(index.row()).lastModified(QTimeZone::LocalTime);
    case Role::Name:
        return directory.entryInfoList(QDir::Files).at(index.row()).fileName().replace(QStringLiteral(".md"), QString());
    case Role::Color:
        return m_color;
    }

    Q_UNREACHABLE();

    return {};
}

QHash<int, QByteArray> NotesModel::roleNames() const
{
    return {
        {Role::Date, "date"},
        {Role::Path, "path"},
        {Role::FileUrl, "fileUrl"},
        {Role::Name, "name"},
        {Role::Color, "color"},
    };
}

QString NotesModel::addNote(const QString &name)
{
    beginResetModel();
    const QString path = directory.path() + QDir::separator() + name + QStringLiteral(".md");
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write("# " + name.toUtf8());
    } else {
        qDebug() << "Failed to create file at" << path;
    }
    endResetModel();
    return name;
}

void NotesModel::deleteNote(const QUrl &path)
{
    beginResetModel();
    QFile::remove(path.toLocalFile());
    endResetModel();
}

void NotesModel::renameNote(const QUrl &path, const QString &name)
{
    QString newPath = directory.path() + QDir::separator() + name + QStringLiteral(".md");
    if (QFile::exists(newPath)) {
        Q_EMIT errorOccured(i18nc("@info:status", "Unable to rename note. A note already exists with the same name."));
        return;
    }
    beginResetModel();
    QFile::rename(path.toLocalFile(), newPath);
    endResetModel();
}

QString NotesModel::path() const
{
    return m_path;
}

void NotesModel::setPath(const QString &newPath)
{
    if (m_path == newPath)
        return;

    beginResetModel();
    if (!m_path.isEmpty()) {
        m_watcher.removePath(m_path + QDir::separator() + QStringLiteral(".directory"));
    }
    m_path = newPath;
    directory = QDir(m_path);
    endResetModel();
    Q_EMIT pathChanged();

    updateColor();

    if (!m_path.isEmpty()) {
        m_watcher.addPath(m_path + QDir::separator() + QStringLiteral(".directory"));
    }
}

void NotesModel::updateColor()
{
    const QString dotDirectory = directory.path() + QDir::separator() + QStringLiteral(".directory");
    if (QFile::exists(dotDirectory)) {
        m_color = KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-Color");
    } else {
        m_color = QStringLiteral("#00000000");
    }
}

static void cleanupImageInDocument(QTextDocument &doc, bool setHeight = false)
{
    QSet<int> cursorPositionsToSkip;
    QTextBlock currentBlock = doc.begin();
    QTextBlock::iterator it;
    while (currentBlock.isValid()) {
        for (it = currentBlock.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
                if (imageFormat.isValid()) {
                    int pos = fragment.position();
                    if (!cursorPositionsToSkip.contains(pos)) {
                        QTextCursor cursor(&doc);
                        cursor.setPosition(pos);
                        cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
                        cursor.removeSelectedText();

                        int width = 620;
                        QImage image(imageFormat.name());
                        if (image.width() < width) {
                            width = image.width();
                        }

                        if (setHeight) {
                            const int height = double(image.height()) / double(image.width()) * double(width);
                            cursor.insertHtml(u"<img width=\"" + QString::number(width) + u"\" height=\"" + QString::number(height) + u"\" src=\""_s
                                              + imageFormat.name() + u"\"\\>"_s);
                        } else {
                            cursor.insertHtml(u"<img width=\"" + QString::number(width) + u"\" src=\""_s + imageFormat.name() + u"\"\\>"_s);
                        }

                        // The textfragment iterator is now invalid, restart from the beginning
                        // Take care not to replace the same fragment again, or we would be in
                        // an infinite loop.
                        cursorPositionsToSkip.insert(pos);
                        // it = currentBlock.begin();
                    }
                }
            }
        }

        currentBlock = currentBlock.next();
    }
}

void NotesModel::exportToPdf(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile())) {
        return;
    }

    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    QPdfWriter writer(destination.toLocalFile());
    writer.setTitle(path.toLocalFile().split(QLatin1Char('/')).constLast());

    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(data));

    cleanupImageInDocument(doc);
    doc.print(&writer);
}

void NotesModel::exportToHtml(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile())) {
        return;
    }

    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    QByteArray output;

#if __has_include(<md4c-html.h>)
    md_html(
        data.constData(),
        data.size(),
        [](const MD_CHAR *data, MD_SIZE size, void *output) {
            auto out = static_cast<QByteArray *>(output);
            *out += QByteArray(data, size);
        },
        &output,
        MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH | MD_FLAG_LATEXMATHSPANS | MD_FLAG_TABLES | MD_FLAG_COLLAPSEWHITESPACE,
        0);
#else
    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(data));
    output = doc.toHtml().toUtf8();
#endif

    QFile exportFile(destination.toLocalFile());
    if (!exportFile.open(QFile::WriteOnly)) {
        return;
    }

    QByteArray content = R"(
<!doctype>
<html>
<head>
<meta charset="utf-8">
<title>)"
        + path.toLocalFile().split(QLatin1Char('/')).constLast().toUtf8() + R"(</title>
<style>
body {
  max-width:800px;
  margin:40px auto;
  padding:0 10px;
  font:18px/1.5 -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji";
  color:#222
}
h1,
h2,
h3 {
  line-height:1.2
}
img {
    max-width: 100%;
}
@media (prefers-color-scheme: dark) {
  body {
    color:#c9d1d9;
    background:#0d1117
  }
  a:link {
    color:#58a6ff
  }
  a:visited {
    color:#8e96f0
  }
}
</style>
</head>
<body>
)" + output
        + R"(
</body>
</html>
)";

    exportFile.write(content);
}

void NotesModel::exportToOdt(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile())) {
        return;
    }

    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();

    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(data));
    cleanupImageInDocument(doc, true);

    QTextDocumentWriter writer(destination.toLocalFile(), "odf");
    writer.write(&doc);
}
