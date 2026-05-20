// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notesmodel.h"
#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPdfWriter>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QUrl>

#if __has_include(<md4c-html.h>)
#include <md4c-html.h>
#endif

using namespace Qt::StringLiterals;

NotesModel::NotesModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , m_fsModel(new QFileSystemModel(this))
{
    m_fsModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    QMimeDatabase mimeDb;
    QMimeType mdMime = mimeDb.mimeTypeForName(u"text/markdown"_s);
    QStringList mdFilters = mdMime.globPatterns();
    if (mdFilters.isEmpty()) {
        mdFilters << u"*.md"_s;
    }

    m_fsModel->setNameFilters(mdFilters);
    m_fsModel->setNameFilterDisables(false);

    // Set the filesystem model as the source for this proxy
    setSourceModel(m_fsModel);

    connect(m_fsModel, &QFileSystemModel::directoryLoaded, this, [this](const QString &path) {
        if (QDir::cleanPath(path) == QDir::cleanPath(m_path)) {
            auto newRoot = m_fsModel->index(m_path);

            // Only reset if the root index has actually changed or was invalid
            if (newRoot != m_rootIndex) {
                beginResetModel();
                m_rootIndex = newRoot;
                endResetModel();
                Q_EMIT rootIndexChanged();
            }
        }
    });
}

QModelIndex NotesModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return m_rootIndex;
    return QIdentityProxyModel::mapToSource(proxyIndex);
}

QModelIndex NotesModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (sourceIndex == m_rootIndex)
        return QModelIndex();
    return QIdentityProxyModel::mapFromSource(sourceIndex);
}

int NotesModel::rowCount(const QModelIndex &parent) const
{
    // Prevent the UI from attempting to draw OS drives before our folder is loaded
    // Ensure we don't query a null source model during destruction or init
    if (!sourceModel() || (!parent.isValid() && !m_rootIndex.isValid())) {
        return 0;
    }
    return QIdentityProxyModel::rowCount(parent);
}

bool NotesModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid() && !m_rootIndex.isValid())
        return false;
    return QIdentityProxyModel::hasChildren(parent);
}

QModelIndex NotesModel::rootIndex() const
{
    // QML now explicitly receives the proxy's root, which is mapped to m_path
    return QModelIndex();
}

QString NotesModel::path() const
{
    return m_path;
}

void NotesModel::setPath(const QString &newPath)
{
    QString cleanPath = QUrl::fromUserInput(newPath).toLocalFile();
    if (cleanPath.isEmpty())
        cleanPath = QDir::cleanPath(newPath);

    if (m_path == cleanPath || cleanPath.isEmpty() || cleanPath == u"/"_s)
        return;

    beginResetModel();
    m_path = cleanPath;
    m_rootIndex = m_fsModel->setRootPath(m_path);
    updateColor();
    endResetModel();

    Q_EMIT pathChanged();
    if (m_rootIndex.isValid()) {
        Q_EMIT rootIndexChanged();
    }
}

void NotesModel::fetchMore(const QString &path)
{
    QString clean = QUrl::fromUserInput(path).toLocalFile();
    if (clean.isEmpty())
        clean = QDir::cleanPath(path);

    // Bypass the proxy to command the filesystem directly
    QModelIndex srcIdx = m_fsModel->index(clean);
    if (srcIdx.isValid() && m_fsModel->canFetchMore(srcIdx)) {
        m_fsModel->fetchMore(srcIdx);
    }
}

QVariant NotesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    QModelIndex srcIndex = mapToSource(index);

    switch (role) {
    case Qt::DisplayRole:
    case Role::Name: {
        QString name = m_fsModel->fileInfo(srcIndex).fileName();
        if (!m_fsModel->isDir(srcIndex) && name.endsWith(u".md"_s)) {
            name.chop(3);
        }
        return name;
    }
    case Role::FileUrl:
        return QUrl::fromLocalFile(m_fsModel->filePath(srcIndex));
    case Role::Path:
        return m_fsModel->filePath(srcIndex);
    case Role::Date:
        return m_fsModel->lastModified(srcIndex);
    case Role::Month:
        return m_fsModel->lastModified(srcIndex).toString(u"MMMM yyyy"_s);
    case Role::Color:
        return m_color;
    case Role::IsFolder:
        return m_fsModel->fileInfo(srcIndex).isDir();
    }

    return QIdentityProxyModel::data(index, role);
}

QHash<int, QByteArray> NotesModel::roleNames() const
{
    auto roles = QIdentityProxyModel::roleNames();
    roles[Role::Date] = "date";
    roles[Role::Path] = "path";
    roles[Role::FileUrl] = "fileUrl";
    roles[Role::Name] = "name";
    roles[Role::Color] = "color";
    roles[Role::Month] = "month";
    roles[Role::IsFolder] = "isFolder";
    return roles;
}

QString NotesModel::addNote(const QString &name)
{
    const QString path = m_path + u'/' + name + u".md"_s;
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write("# " + name.toUtf8());
    } else {
        qDebug() << "Failed to create file at" << path;
    }
    return name;
}

void NotesModel::deleteNote(const QUrl &path)
{
    if (path.isLocalFile()) {
        QFile::remove(path.toLocalFile());
    }
}

void NotesModel::renameNote(const QUrl &path, const QString &name)
{
    QFileInfo info(path.toLocalFile());
    QString cleanName = name;
    if (cleanName.endsWith(u".md"_s))
        cleanName.chop(3);

    QString newPath = info.absolutePath() + u'/' + cleanName + u".md"_s;

    if (QFile::exists(newPath)) {
        Q_EMIT errorOccurred(i18nc("@info:status", "Unable to rename note. A note already exists with the same name."));
        return;
    }
    QFile::rename(path.toLocalFile(), newPath);
}

void NotesModel::duplicateNote(const QUrl &path)
{
    const QString originalFilePath = path.toLocalFile();
    if (!QFile::exists(originalFilePath)) {
        Q_EMIT errorOccurred(tr("Original note file does not exist."));
        return;
    }

    const QFileInfo originalInfo(originalFilePath);
    const QDir dir = originalInfo.absoluteDir();

    const QString suffix = originalInfo.suffix();

    const QString copyBase = originalInfo.completeBaseName() % QLatin1String(" Copy");
    QString finalFileName = copyBase + u'.' + suffix;

    int counter = 1;
    while (dir.exists(finalFileName)) {
        finalFileName = copyBase + u' ' + QString::number(counter) + u'.' + suffix;
        counter++;
    }

    QString finalFilePath = dir.filePath(finalFileName);

    if (!QFile::copy(originalFilePath, finalFilePath)) {
        Q_EMIT errorOccurred(tr("Failed to copy the note file."));
    }
}

void NotesModel::copyWholeNote(const QUrl &path)
{
    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return;

    const QString markdown = QString::fromUtf8(file.readAll());

    QTextDocument doc;
    doc.setMarkdown(markdown);
    const QString html = doc.toHtml();

    QMimeData *mime = new QMimeData();
    mime->setText(markdown);
    mime->setHtml(html);
    mime->setData(QStringLiteral("text/markdown"), markdown.toUtf8());
    mime->setData(QStringLiteral("text/plain"), markdown.toUtf8());

    QGuiApplication::clipboard()->setMimeData(mime);
}

bool NotesModel::moveEntry(const QUrl &source, const QUrl &destination)
{
    QFileInfo sourceInfo(source.toLocalFile());
    QFileInfo destInfo(destination.toLocalFile());

    if (!sourceInfo.exists() || !destInfo.isDir()) {
        Q_EMIT errorOccurred(i18nc("@info:status", "Invalid move operation."));
        return false;
    }

    QString newPath = destInfo.absoluteFilePath() + QDir::separator() + sourceInfo.fileName();

    if (destInfo.absoluteFilePath().startsWith(sourceInfo.absoluteFilePath())) {
        Q_EMIT errorOccurred(i18nc("@info:status", "Cannot move a folder into itself."));
        return false;
    }

    bool success = QFile::rename(sourceInfo.absoluteFilePath(), newPath);
    if (!success) {
        Q_EMIT errorOccurred(i18nc("@info:status", "Failed to move file."));
    }
    return success;
}

void NotesModel::updateColor()
{
    const QString dotDirectory = m_path + u'/' + u".directory"_s;
    if (QFile::exists(dotDirectory)) {
        m_color = KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-Color");
    } else {
        m_color = u"#00000000"_s;
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
                        cursorPositionsToSkip.insert(pos);
                    }
                }
            }
        }
        currentBlock = currentBlock.next();
    }
}

bool NotesModel::exportToPdf(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile()))
        return false;
    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QPdfWriter writer(destination.toLocalFile());
    writer.setTitle(path.toLocalFile().split(QLatin1Char('/')).constLast());

    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(data));

    cleanupImageInDocument(doc);
    doc.print(&writer);
    return true;
}

bool NotesModel::exportToHtml(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile()))
        return false;
    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return false;

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
    if (!exportFile.open(QFile::WriteOnly))
        return false;

    exportFile.write(output);
    return true;
}

bool NotesModel::exportToOdt(const QUrl &path, const QUrl &destination)
{
    if (!QFile::exists(path.toLocalFile()))
        return false;
    QFile file(path.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();

    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(data));
    cleanupImageInDocument(doc, true);

    QTextDocumentWriter writer(destination.toLocalFile(), "odf");
    writer.write(&doc);
    return true;
}

bool NotesModel::noteExists(const QString &noteName) const
{
    if (m_path.isEmpty() || noteName.isEmpty()) {
        return false;
    }

    QString fileName = noteName;
    if (!fileName.endsWith(QStringLiteral(".md"))) {
        fileName += QStringLiteral(".md");
    }

    return QFile::exists(m_path + u'/' + fileName);
}

#include "moc_notesmodel.cpp"
