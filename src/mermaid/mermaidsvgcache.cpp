// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidsvgcache.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSaveFile>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

namespace
{
constexpr QLatin1StringView s_diskCachePrefix = "v1_"_L1;
constexpr QLatin1StringView s_diskCacheExt = ".cache"_L1;

QString diskCacheFilter()
{
    return QString(s_diskCachePrefix) + QLatin1Char('*') + s_diskCacheExt;
}
} // namespace

MermaidSvgCache::MermaidSvgCache()
{
    m_cache.setMaxCost(DefaultMaxCost);

    const QString baseCache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!baseCache.isEmpty()) {
        m_diskCacheDir = baseCache + u"/mermaid"_s;
    }
}

MermaidSvgCache *MermaidSvgCache::instance()
{
    static MermaidSvgCache s_instance;
    return &s_instance;
}

void MermaidSvgCache::setDiskCacheDir(const QString &dirPath)
{
    QMutexLocker locker(&m_mutex);
    m_diskCacheDir = dirPath;
    m_diskBytes = -1;
}

QString MermaidSvgCache::diskFilePath(const QString &key) const
{
    const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha256).toHex().left(24);
    return m_diskCacheDir + QLatin1Char('/') + s_diskCachePrefix + QString::fromLatin1(hash) + s_diskCacheExt;
}

void MermaidSvgCache::trimDiskCache()
{
    if (m_diskCacheDir.isEmpty()) {
        return;
    }

    QDir dir(m_diskCacheDir);
    if (!dir.exists()) {
        m_diskBytes = 0;
        return;
    }

    if (m_diskBytes < 0) {
        qint64 total = 0;
        const QFileInfoList allFiles = dir.entryInfoList({diskCacheFilter()}, QDir::Files);
        for (const QFileInfo &fi : allFiles) {
            total += fi.size();
        }
        m_diskBytes = total;
    }

    const qint64 limit = static_cast<qint64>(m_cache.maxCost());
    if (m_diskBytes <= limit) {
        return;
    }

    const QFileInfoList files = dir.entryInfoList({diskCacheFilter()}, QDir::Files, QDir::Time | QDir::Reversed);
    for (const QFileInfo &fi : files) {
        if (m_diskBytes <= limit) {
            break;
        }
        const qint64 sz = fi.size();
        if (QFile::remove(fi.absoluteFilePath())) {
            m_diskBytes = qMax<qint64>(0, m_diskBytes - sz);
        }
    }
}

bool MermaidSvgCache::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.contains(key);
}

std::shared_ptr<CachedSvgDiagram> MermaidSvgCache::get(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    if (CachedSvgDiagram *item = m_cache.object(key)) {
        return std::make_shared<CachedSvgDiagram>(*item);
    }

    if (m_diskCacheDir.isEmpty()) {
        return nullptr;
    }

    QFile file(diskFilePath(key));
    const bool writable = file.open(QIODevice::ReadWrite | QIODevice::ExistingOnly);
    if (!writable && !file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    const QList<QByteArray> dims = file.readLine().trimmed().split(' ');
    if (dims.size() != 2) {
        return nullptr;
    }

    bool okW = false;
    bool okH = false;
    const int w = dims.at(0).toInt(&okW);
    const int h = dims.at(1).toInt(&okH);
    if (!okW || !okH) {
        return nullptr;
    }

    const QString flatSvg = QString::fromUtf8(file.readAll());
    if (flatSvg.isEmpty()) {
        return nullptr;
    }

    if (writable) {
        file.setFileTime(QDateTime::currentDateTimeUtc(), QFileDevice::FileModificationTime);
    }

    const QSize naturalSize(w, h);
    auto *item = new CachedSvgDiagram{flatSvg, naturalSize};
    m_cache.insert(key, item, item->cost());

    return std::make_shared<CachedSvgDiagram>(flatSvg, naturalSize);
}

void MermaidSvgCache::insert(const QString &key, const QString &flatSvg, const QSize &naturalSize)
{
    auto *item = new CachedSvgDiagram{flatSvg, naturalSize};
    const int cost = item->cost();

    QMutexLocker locker(&m_mutex);
    m_cache.insert(key, item, cost);

    if (m_diskCacheDir.isEmpty() || cost > m_cache.maxCost()) {
        return;
    }

    QDir dir(m_diskCacheDir);
    if (!dir.exists() && !dir.mkpath(u"."_s)) {
        return;
    }

    const QString path = diskFilePath(key);
    const qint64 oldSize = QFileInfo::exists(path) ? QFileInfo(path).size() : 0;

    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QByteArray::number(naturalSize.width()) + ' ' + QByteArray::number(naturalSize.height()) + '\n');
        file.write(flatSvg.toUtf8());
        if (file.commit()) {
            if (m_diskBytes >= 0) {
                m_diskBytes = qMax<qint64>(0, m_diskBytes - oldSize) + QFileInfo(path).size();
            }
            trimDiskCache();
        }
    }
}

void MermaidSvgCache::clear()
{
    QMutexLocker locker(&m_mutex);
    m_cache.clear();

    if (!m_diskCacheDir.isEmpty()) {
        QDir dir(m_diskCacheDir);
        if (dir.exists()) {
            const QFileInfoList files = dir.entryInfoList({diskCacheFilter()}, QDir::Files);
            for (const QFileInfo &fi : files) {
                QFile::remove(fi.absoluteFilePath());
            }
        }
        m_diskBytes = 0;
    }
}

void MermaidSvgCache::clearMemory()
{
    QMutexLocker locker(&m_mutex);
    m_cache.clear();
}

int MermaidSvgCache::totalCost() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.totalCost();
}

int MermaidSvgCache::maxCost() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.maxCost();
}

void MermaidSvgCache::setMaxCost(int bytes)
{
    QMutexLocker locker(&m_mutex);
    m_cache.setMaxCost(bytes);
    trimDiskCache();
}

int MermaidSvgCache::count() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.count();
}
