#include <QtCore>
#include <FreeImage.h>

#include "painter.h"
#include "private/imagepaintengine.h"
#include "image.h"

namespace Malachite
{

QImage wrapInQImage(const ImageU8 &image)
{
	return QImage(
		reinterpret_cast<const uint8_t *>((const BgraPremultU8 *)image.cbegin()),
		image.width(), image.height(),
		QImage::Format_ARGB32_Premultiplied);
}

ConstImageU8 wrapQImage(const QImage &image)
{
	return ConstImageU8::wrap(
		reinterpret_cast<const BgraPremultU8 *>(image.constBits()),
		image.size());
}

ImageU8 wrapQImage(QImage &image)
{
	return ImageU8::wrap(
		reinterpret_cast<BgraPremultU8 *>(image.bits()),
		image.size());
}

bool Image::isBlank() const
{
	return std::find_if(begin(), end(), [](const Pixel &p){
		return p.a() != 0;
	}) == end();
}

Image Image::toOpaqueImage() const
{
	Image image = *this;
	
	Painter painter(&image);
	painter.setColor(Color::white());
	painter.setBlendMode(BlendMode::DestinationOver);
	painter.drawRect(image.rect());
	
	painter.end();
	
	return image;
}

PaintEngine *Image::createPaintEngine()
{
	return new ImagePaintEngine;
}

void Image::pasteWithBlendMode(BlendMode mode, float opacity, const Image &image, const QPoint &offset, const QRect &imageRect)
{
	QRect r = rect() & imageRect.translated(offset);
	
	auto blendOp = mode.op();
	
	if (opacity == 1.0)
	{
		for (int y = r.top(); y <= r.bottom(); ++y)
		{
			auto dp = scanline(y);
			dp += r.left();
			
			auto sp = image.constScanline(y - offset.y());
			sp += (r.left() - offset.x());
			
			blendOp->blend(r.width(), dp, sp);
		}
	}
	else
	{
		for (int y = r.top(); y <= r.bottom(); ++y)
		{
			auto dp = scanline(y);
			dp += r.left();
			
			auto sp = image.constScanline(y - offset.y());
			sp += (r.left() - offset.x());
			
			blendOp->blend(r.width(), dp, sp, opacity);
		}
	}
}

// assumes both dst and src are 16bit aligned
static void copyColorFast(int count, BgraPremultU8 *dst, const Pixel *src)
{
	int countPer4 = count / 4;
	int rem = count % 4;
	
	static const PixelVec vec255(0xFF);
	
	while (countPer4--)
	{
		__m128i d0 = _mm_cvtps_epi32(src->v() * vec255);
		src++;
		__m128i d1 = _mm_cvtps_epi32(src->v() * vec255);
		src++;
		
		__m128i w0 = _mm_packs_epi32(d0, d1);
		
		__m128i d2 = _mm_cvtps_epi32(src->v() * vec255);
		src++;
		__m128i d3 = _mm_cvtps_epi32(src->v() * vec255);
		src++;
		
		__m128i w1 = _mm_packs_epi32(d2, d3);
		
		__m128i b = _mm_packus_epi16(w0, w1);
		
		*(reinterpret_cast<__m128i *>(dst)) = b;
		
		dst += 4;
	}
	
	if (rem)
	{
		auto dstDwords = reinterpret_cast<uint32_t *>(dst);
		
		auto convert1 = [](const Pixel &p) -> uint32_t
		{
			union
			{
				uint32_t dwords[4];
				__m128i d;
			} u;
			
			u.d = _mm_cvtps_epi32(p.v() * vec255);
			u.d = _mm_packs_epi32(u.d, u.d);
			u.d = _mm_packus_epi16(u.d, u.d);
			
			return u.dwords[0];
		};
		
		while (rem--)
			*dstDwords++ = convert1(*src++);
	}
}

ImageU8 Image::toImageU8() const
{
	ImageU8 result(this->size());
	copyColorFast(this->area(), (ImageU8::value_type *)result.begin(), (const Image::value_type *)this->begin());
	return result;
}

/*
QByteArray Image::toByteArray() const
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	
	int count = width() * height();
	data.reserve(count * sizeof(PixelType));
	
	Pointer<const Pixel> p = constBits();
	
	for (int i = 0; i < count; ++i)
	{
		stream << p->a();
		stream << p->r();
		stream << p->g();
		stream << p->b();
		p++;
	}
	
	return data;
}

Image Image::fromByteArray(const QByteArray &data, const QSize &size)
{
	int count = size.width() * size.height();
	if (size_t(data.size()) < size_t(count) * sizeof(PixelType))
		return Image();
	
	Image image(size);
	
	Pointer<Pixel> p = image.bits();
	
	QDataStream stream(data);
	
	for (int i = 0; i < count; ++i)
	{
		stream >> p->ra();
		stream >> p->rr();
		stream >> p->rg();
		stream >> p->rb();
		p++;
	}
	
	return image;
}*/

Image &Image::operator*=(float factor)
{
	factor = qBound(0.f, factor, 1.f);
	if (factor == 1.f)
		return *this;
	
	PixelVec vfactor(factor);

	for (auto &p : *this) {
		p.rv() *= vfactor;
	}

	return *this;
}

QDataStream &operator<<(QDataStream &out, const Image &image)
{
	out << int32_t(image.width()) << int32_t(image.height());

	for (auto p : image) {
		out << p.a();
		out << p.r();
		out << p.g();
		out << p.b();
	}
	
	return out;
}

QDataStream &operator>>(QDataStream &in, Image &image)
{
	int32_t w, h;
	in >> w >> h;
	
	Image result(w, h);
	
	for (auto &p : result) {
		in >> p.ra();
		in >> p.rr();
		in >> p.rg();
		in >> p.rb();
	}
	
	image = result;
	
	return in;
}

}
