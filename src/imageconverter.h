/*
   Copyright (C) 2009 Canonical
   Author: Aurélien Gâteau <aurelien.gateau@canonical.com>

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 2 or 3 of the License.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

class QVariant;
class QImage;

namespace ImageConverter
{

/**
 * Returns a variant representing an image using the format describe in the
 * galago spec
 */
QVariant variantForImage(const QImage &image);

} // namespace

#endif /* IMAGECONVERTER_H */
