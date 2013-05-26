#pragma once

#include <QObject>
#include <QRect>
#include "global.h"

class QPoint;
class QSize;

namespace Malachite
{
class Affine2D;
class Image;
}

namespace PaintField {

class CanvasViewportController : public QObject
{
	Q_OBJECT
public:
	explicit CanvasViewportController(QObject *parent = 0);
	~CanvasViewportController();
	
	void beginUpdateTile(int tileCount);
	void updateTile(const QPoint &tileKey, const Malachite::Image &image, const QPoint &offset);
	void endUpdateTile();
	
	void placeViewport(QWidget *window);
	void moveViewport(const QRect &rect, bool visible);
	
signals:
	
	void viewSizeChanged(const QSize &size);
	
public slots:
	
	void setTransform(const Malachite::Affine2D &toScene, const Malachite::Affine2D &fromScene);
	void setRetinaMode(bool mode);
	void setDocumentSize(const QSize &size);
	void update();
	
private:
	
	struct Data;
	Data *d;
};

template <typename TFunction>
void drawDivided(const QRect &viewRect, const TFunction &drawFunc)
{
	constexpr auto unit = 128;
	
	if (viewRect.width() * viewRect.height() <= unit * unit)
	{
		drawFunc(viewRect);
	}
	else
	{
		int xCount = viewRect.width() / unit;
		if (viewRect.width() % unit)
			xCount++;
		
		int yCount = viewRect.height() / unit;
		if (viewRect.height() % unit)
			yCount++;
		
		for (int x = 0; x < xCount; ++x)
		{
			for (int y = 0; y < yCount; ++y)
			{
				auto viewRectDivided = viewRect & QRect(viewRect.topLeft() + QPoint(x, y) * unit, QSize(unit, unit));
				drawFunc(viewRectDivided);
			}
		}
	}
}

} // namespace PaintField