
#include <QtCore>
#include <Malachite/Painter>
#include "testutil.h"
#include "paintfield/core/document.h"
#include "paintfield/core/rasterlayer.h"
#include "paintfield/core/grouplayer.h"

using namespace Malachite;

namespace PaintField
{

namespace TestUtil
{

QDir createTestDir()
{
	auto tempDir = QDir::temp();
	
	QProcess process;
	process.start("rm -rf " + tempDir.filePath("test"));
	process.waitForFinished();
	
	tempDir.mkdir("test");
	tempDir.cd("test");
	
	return tempDir;
}

void createTestFile(const QString &path)
{
	QFile file(path);
	file.open(QIODevice::WriteOnly);
}

Document *createTestDocument(QObject *parent)
{
	LayerRef layer[3];
	
	for (int i = 0; i < 3; ++i)
	{
		auto l = makeSP<RasterLayer>("layer" + QString::number(i));
		l->setSurface(createTestSurface(i));
		layer[i] = l;
	}
	
	auto group = makeSP<GroupLayer>("group");
	group->append(layer[1]);
	group->append(layer[2]);
	
	QList<LayerRef> layers = { layer[0], group };
	
	return new Document("document", QSize(400, 300), layers, parent);
}

void drawTestPattern(Paintable *paintable, int index)
{
	Painter painter(paintable);
	
	switch (index)
	{
		case 0:
			painter.setColor(Color::fromRgbValue(1,0,0));
			painter.drawEllipse(128, 96, 64, 32);
			break;
		case 1:
			painter.setColor(Color::fromRgbValue(0,1,0));
			painter.drawRect(128, 96, 128, 96);
			break;
		case 2:
		{
			Polygon polygon;
			polygon << Vec2D(256, 128) << Vec2D(192, 256) << Vec2D(320, 256);
			painter.drawPolygon(polygon);
			break;
		}
		default:
			break;
	}
}

Surface createTestSurface(int patternIndex)
{
	Surface surface;
	drawTestPattern(&surface, patternIndex);
	return surface;
}

}

}

