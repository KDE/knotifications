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
struct SpecImage
{
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

	QImage image = _image.convertToFormat(QImage::Format_RGBA8888);

	QByteArray data((const char*)image.constBits(), image.byteCount());

	SpecImage specImage;
	specImage.width = image.width();
	specImage.height = image.height();
	specImage.rowStride = image.bytesPerLine();
	specImage.hasAlpha = true;
	specImage.bitsPerSample = 8;
	specImage.channels = 4;
	specImage.data = data;

	return QVariant::fromValue(specImage);
}

} // namespace

