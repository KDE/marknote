// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>

/**
 * @brief Protects shared static resources (cache and registry) from concurrent
 * access. Ensures thread safety when images are requested from multiple threads
 * simultaneously.
 */
static QMutex s_mutex;

/**
 * @brief Static RAM cache storing downscaled images.
 * Shared across all provider instances to prevent disk thrashing and repeated
 * allocations. Maps hash ID -> QImage.
 */
static QHash<QString, QImage> s_ramCache;

/**
 * @brief Registry mapping image hashes to real file paths.
 * Allows the main thread to register a file path instantly without loading it,
 * deferring the heavy I/O to the background thread in requestImage().
 * Maps hash ID -> "/absolute/path/to/file.jpg".
 */
static QHash<QString, QString> s_pathRegistry;

/**
 * @brief Asynchronous image provider for handling Marknote images efficiently.
 * Implements a thread-safe caching mechanism and performs heavy image
 * loading/scaling in a background thread to prevent blocking the main UI
 * thread.
 */
class AsyncImageProvider : public QQuickImageProvider {
public:
  /**
   * @brief Constructs the provider with forced asynchronous loading.
   * Sets QQmlImageProviderBase::ForceAsynchronousImageLoading to ensure
   * requestImage() runs in a worker thread, preventing "connectFinished"
   * warnings and UI freezes.
   */
  AsyncImageProvider()
      : QQuickImageProvider(
            QQuickImageProvider::Image,
            QQmlImageProviderBase::ForceAsynchronousImageLoading) {}

  /**
   * @brief Retrieves or loads the requested image.
   * * The process follows these steps:
   * 1. Checks the thread-safe RAM cache for an existing image.
   * 2. If missing, retrieves the file path from the registry.
   * 3. Loads and downscales (if >1024px) the image from disk.
   * 4. Updates the cache for future requests.
   * * @param id The hash string identifying the image (from s_pathRegistry).
   * @param size Output pointer for the image size (optional).
   * @param requestedSize The requested size (unused).
   * @return The loaded QImage, or a null QImage if loading fails.
   */
  QImage requestImage(const QString &id, QSize *size, const QSize &) override {
    // Check RAM Cache
    {
      QMutexLocker locker(&s_mutex);
      if (s_ramCache.contains(id)) {
        QImage img = s_ramCache.value(id);
        if (size)
          *size = img.size();
        return img;
      }
    }

    // Get Real File Path
    QString filePath;
    {
      QMutexLocker locker(&s_mutex);
      filePath = s_pathRegistry.value(id);
    }

    if (filePath.isEmpty())
      return QImage();

    // Load & Downscale (HEAVY WORK - Runs in Background Thread)
    QImage image;
    if (!image.load(filePath)) {
      return QImage();
    }

    if (image.width() > 1024) {
      image = image.scaledToWidth(1024, Qt::SmoothTransformation);
    }

    // Save to Cache
    {
      QMutexLocker locker(&s_mutex);
      s_ramCache.insert(id, image);
    }

    if (size)
      *size = image.size();
    return image;
  }
};
