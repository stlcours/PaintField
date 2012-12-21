#ifndef PAINTFIELD_BRUSH_SOURCEFACTORYMANAGER_H
#define PAINTFIELD_BRUSH_SOURCEFACTORYMANAGER_H

#include <QObject>
#include <QVariant>
#include "paintfield-core/list.h"

namespace Malachite
{
class Surface;
}

namespace PaintField
{

class BrushStrokerFactory;
class BrushStroker;

class BrushStrokerFactoryManager : public QObject
{
	Q_OBJECT
public:
	explicit BrushStrokerFactoryManager(QObject *parent = 0);
	
	void addFactory(BrushStrokerFactory *factory);
	
	bool contains(const QString &name) const;
	QVariantMap defaultSettings(const QString &name) const;
	BrushStrokerFactory *factory(const QString &name);
	
signals:
	
public slots:
	
private:
	
	List<BrushStrokerFactory *> _factories;
};

} // namespace PaintField

#endif // PAINTFIELD_BRUSH_SOURCEFACTORYMANAGER_H
