// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QCache>
#include <QHash>
#include <QImageReader>
#include <QMutex>
#include <QQuickImageProvider>
#include <QReadWriteLock>
#include <mutex>

/**
 * @brief Protects the shared static cache from concurrent access.
 * Ensures thread safety when images are inserted or retrieved from multiple threads.
 */
inline static QMutex s_cacheMutex;

/**
 * @brief Protects the static path registry.
 * Uses a ReadWrite lock to allow multiple worker threads to resolve paths
 * simultaneously without blocking, while safely allowing batch insertions.
 */
inline static QReadWriteLock s_registryLock;

/**
 * @brief Static RAM cache storing downscaled images.
 * Shared across all provider instances to prevent disk thrashing and repeated
 * allocations. Maps hash ID -> QImage.
 */
inline static QCache<QString, QImage> s_ramCache;

/**
 * @brief Registry mapping image hashes to real file paths.
 * Allows the main thread to register a file path instantly without loading it,
 * deferring the heavy I/O to the background thread in requestImage().
 * Maps hash ID -> "/absolute/path/to/file.jpg".
 */
inline static QHash<QString, QString> s_pathRegistry;

/**
 * @brief Asynchronous image provider for handling Marknote images efficiently.
 * Implements a thread-safe caching mechanism and performs heavy image
 * loading/scaling in a background thread to prevent blocking the main UI
 * thread.
 */
class AsyncImageProvider : public QQuickImageProvider
{
public:
    /**
     * @brief Constructs the provider with forced asynchronous loading.
     * Sets QQmlImageProviderBase::ForceAsynchronousImageLoading to ensure
     * requestImage() runs in a worker thread, preventing "connectFinished"
     * warnings and UI freezes. It also initializes the cache size limit.
     */
    AsyncImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image, QQmlImageProviderBase::ForceAsynchronousImageLoading)
    {
        static std::once_flag flag;
        std::call_once(flag, []() {
            s_ramCache.setMaxCost(64 * 1024 * 1024);
        });
    }

    /**
     * @brief Safely registers a single image path into the registry.
     * Uses a write lock to prevent race conditions when the document handler
     * processes a new image URL.
     * * @param id The hash string identifying the image.
     * @param path The absolute local file path to the image.
     */
    static void registerPath(const QString &id, const QString &path)
    {
        QWriteLocker locker(&s_registryLock);
        s_pathRegistry.insert(id, path);
    }

    /**
     * @brief Safely registers multiple image paths into the registry at once.
     * Useful for batch updates from DocumentHandler to minimize lock contention.
     * * @param paths A QHash mapping image hash IDs to their absolute file paths.
     */
    static void batchRegister(const QHash<QString, QString> &paths)
    {
        if (paths.isEmpty())
            return;
        QWriteLocker locker(&s_registryLock);
        s_pathRegistry.insert(paths);
    }

    /**
     * @brief Clears the entire path registry.
     * Uses a write lock to safely wipe all registered paths.
     */
    static void clear()
    {
        QWriteLocker locker(&s_registryLock);
        s_pathRegistry.clear();
    }

    /**
     * @brief Retrieves or loads the requested image.
     * * The process follows these steps:
     * 1. Checks the thread-safe RAM cache for an existing image.
     * 2. If missing, retrieves the file path from the registry.
     * 3. Loads and downscales (if >1024px) the image from disk.
     * 4. Updates the cache for future requests.
     * * @param id The hash string identifying the image (from s_pathRegistry).
     * @param size Output pointer for the image size (optional).
     * @param requestedSize The requested size.
     * @return The loaded QImage, or a null QImage if loading fails.
     */
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        QImage image;
        bool found = false;

        // RAM Cache
        {
            QMutexLocker locker(&s_cacheMutex);
            if (auto *cachedImg = s_ramCache.object(id)) {
                image = *cachedImg;
                found = true;
            }
        }

        // Disk Load
        if (!found) {
            QString filePath;
            {
                QReadLocker locker(&s_registryLock);
                filePath = s_pathRegistry.value(id);
            }

            if (!filePath.isEmpty()) {
                QImageReader reader(filePath);
                reader.setDecideFormatFromContent(true);
                reader.setAllocationLimit(256);

                if (reader.canRead()) {
                    reader.setAutoTransform(true);

                    // Decode exactly the size needed
                    QSize targetSize;
                    if (requestedSize.isValid() && requestedSize.width() > 0) {
                        targetSize = requestedSize;
                    } else {
                        // Only pay the reader.size() penalty if we don't know the requested size
                        const int targetWidth = 1024;
                        const QSize originalSize = reader.size();
                        if (originalSize.width() > targetWidth) {
                            int newHeight = (originalSize.height() * targetWidth) / originalSize.width();
                            targetSize = QSize(targetWidth, newHeight);
                        }
                    }

                    if (targetSize.isValid()) {
                        reader.setScaledSize(targetSize);
                    }

                    image = reader.read();

                    if (!image.isNull()) {
                        // RGB16 Memory Optimization
                        if (!image.hasAlphaChannel()) {
                            image.convertTo(QImage::Format_RGB16);
                        } else if (image.format() != QImage::Format_ARGB32_Premultiplied) {
                            image.convertTo(QImage::Format_ARGB32_Premultiplied);
                        }

                        // Insert into Cache
                        {
                            QMutexLocker locker(&s_cacheMutex);
                            s_ramCache.insert(id, new QImage(image), static_cast<int>(image.sizeInBytes()));
                        }
                    }
                }
            }
        }

        if (image.isNull()) {
            image = QImage(1, 1, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
        }

        if (size)
            *size = image.size();

        return image;
    }
};
