// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QCache>
#include <QMutex>
#include <QSize>
#include <QString>
#include <memory>

struct CachedSvgDiagram
{
    QString flatSvg;
    QSize naturalSize;

    [[nodiscard]] int cost() const { return static_cast<int>(sizeof(CachedSvgDiagram)) + (flatSvg.size() * static_cast<int>(sizeof(QChar))); }
};

class MermaidSvgCache
{
public:
    static MermaidSvgCache *instance();

    // Default max cost: 50 MB (50 * 1024 * 1024 bytes)
    static constexpr int DefaultMaxCost = 50 * 1024 * 1024;

    bool contains(const QString &key) const;
    std::shared_ptr<CachedSvgDiagram> get(const QString &key);
    void insert(const QString &key, const QString &flatSvg, const QSize &naturalSize);
    void clear();

    int totalCost() const;
    int maxCost() const;
    void setMaxCost(int bytes);
    int count() const;

private:
    MermaidSvgCache();
    ~MermaidSvgCache() = default;

    mutable QMutex m_mutex;
    QCache<QString, CachedSvgDiagram> m_cache;
};
