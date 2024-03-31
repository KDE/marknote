// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notesmodel.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QTextDocument>
#include <QUrl>

#if __has_include(<md4c-html.h>)
#include <md4c-html.h>
#endif

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NotesModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() || m_path.isEmpty() ? 0 : directory.entryList(QDir::Files).count();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Role::Path:
        return QUrl::fromLocalFile(directory.entryInfoList(QDir::Files).at(index.row()).filePath());
    case Role::Date:
        return directory.entryInfoList(QDir::Files).at(index.row()).lastModified(QTimeZone::LocalTime);
    case Role::Name:
        return directory.entryInfoList(QDir::Files).at(index.row()).fileName().replace(QStringLiteral(".md"), QString());
    }

    Q_UNREACHABLE();

    return {};
}

QHash<int, QByteArray> NotesModel::roleNames() const
{
    return {{Role::Date, "date"}, {Role::Path, "path"}, {Role::Name, "name"}};
}

QString NotesModel::addNote(const QString &name)
{
    beginResetModel();
    const QString path = m_path + QDir::separator() + name + QStringLiteral(".md");
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write("# " + name.toUtf8());
    } else {
        qDebug() << "Failed to create file at" << m_path;
    }
    endResetModel();
    return path;
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
    m_path = newPath;
    directory = QDir(m_path);
    endResetModel();
    Q_EMIT pathChanged();
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
