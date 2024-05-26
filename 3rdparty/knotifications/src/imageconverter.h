/*
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only
*/

#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

class QVariant;
class QImage;

namespace ImageConverter {
/**
 * Returns a variant representing an image using the format describe in the
 * freedesktop.org spec
 */
QVariant variantForImage(const QImage& image);

}// namespace ImageConverter

#endif /* IMAGECONVERTER_H */
