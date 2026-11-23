// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidsvgcache.h"
#include <QMutexLocker>

MermaidSvgCache::MermaidSvgCache()
{
    m_cache.setMaxCost(DefaultMaxCost);
}

MermaidSvgCache *MermaidSvgCache::instance()
{
    static MermaidSvgCache s_instance;
    return &s_instance;
}

bool MermaidSvgCache::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.contains(key);
}

std::shared_ptr<CachedSvgDiagram> MermaidSvgCache::get(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    CachedSvgDiagram *item = m_cache.object(key);
    if (!item) {
        return nullptr;
    }
    return std::make_shared<CachedSvgDiagram>(*item);
}

void MermaidSvgCache::insert(const QString &key, const QString &flatSvg, const QSize &naturalSize)
{
    auto *item = new CachedSvgDiagram{flatSvg, naturalSize};
    const int cost = item->cost();

    QMutexLocker locker(&m_mutex);
    m_cache.insert(key, item, cost);
}

void MermaidSvgCache::clear()
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
}

int MermaidSvgCache::count() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.count();
}
