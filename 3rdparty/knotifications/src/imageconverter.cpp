/*
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only
*/

#include "imageconverter.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QImage>

namespace ImageConverter
{
/**
 * A structure representing an image which can be marshalled to fit the
 * notification spec.
 */
struct SpecImage {
    int width, height, rowStride;
    bool hasAlpha;
    int bitsPerSample, channels;
    QByteArray data;
};

QDBusArgument &operator<<(QDBusArgument &argument, const SpecImage &image)
{
    argument.beginStructure();
    argument << image.width << image.height << image.rowStride << image.hasAlpha;
    argument << image.bitsPerSample << image.channels << image.data;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SpecImage &image)
{
    argument.beginStructure();
    argument >> image.width >> image.height >> image.rowStride >> image.hasAlpha;
    argument >> image.bitsPerSample >> image.channels >> image.data;
    argument.endStructure();
    return argument;
}

} // namespace

// This must be before the QVariant::fromValue below (#211726)
Q_DECLARE_METATYPE(ImageConverter::SpecImage)

namespace ImageConverter
{
QVariant variantForImage(const QImage &_image)
{
    qDBusRegisterMetaType<SpecImage>();

    const bool hasAlpha = _image.hasAlphaChannel();
    QImage image;
    if (hasAlpha) {
        image = _image.convertToFormat(QImage::Format_RGBA8888);
    } else {
        image = _image.convertToFormat(QImage::Format_RGB888);
    }

    QByteArray data((const char *)image.constBits(), image.sizeInBytes());

    SpecImage specImage;
    specImage.width = image.width();
    specImage.height = image.height();
    specImage.rowStride = image.bytesPerLine();
    specImage.hasAlpha = hasAlpha;
    specImage.bitsPerSample = 8;
    specImage.channels = hasAlpha ? 4 : 3;
    specImage.data = data;

    return QVariant::fromValue(specImage);
}

} // namespace
