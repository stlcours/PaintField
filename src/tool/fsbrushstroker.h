#ifndef FSBRUSHSTROKER_H
#define FSBRUSHSTROKER_H

#include "mlsurfacepainter.h"
#include "fstabletinputdata.h"
#include "fsbrushsetting.h"

class FSStroker
{
public:
	
	FSStroker(MLSurface *surface, const FSBrushSetting *setting);
	virtual ~FSStroker() {}
	
	void moveTo(const FSTabletInputData &data);
	void lineTo(const FSTabletInputData &data);
	void end();
	
	QPointSet lastEditedKeys() const { return _lastEditedKeys; }
	QPointSet totalEditedKeys() const { return _totalEditedKeys; }
	
	void clearLastEditedKeys() { _lastEditedKeys.clear(); }
	
	const FSBrushSetting *setting() const { return _setting; }
	MLVec4F argb() const { return _argb; }
	MLSurface *surface() { return _surface; }
	
protected:
	
	virtual void drawFirst(const FSTabletInputData &data) = 0;
	virtual void drawInterval(const MLPolygon &polygon, const FSTabletInputData &dataStart, const FSTabletInputData &dataEnd) = 0;
	
	void addEditedKeys(const QPointSet &keys);
	
private:
	
	MLSurface *_surface;
	QPointSet _lastEditedKeys, _totalEditedKeys;
	
	int _count;
	FSTabletInputData  _dataPrev, _dataStart, _dataEnd, _currentData;
	//MLVec2D _v1, v2;
	
	MLVec4F _argb;
	const FSBrushSetting *_setting;
};

class FSPenStroker : public FSStroker
{
public:
	
	//using FSStroker::FSStroker;
	FSPenStroker(MLSurface *surface, const FSBrushSetting *setting) : FSStroker(surface, setting) {}
	
protected:
	
	void drawFirst(const FSTabletInputData &data);
	void drawInterval(const MLPolygon &polygon, const FSTabletInputData &dataStart, const FSTabletInputData &dataEnd);
	
private:
	
	void drawOne(const MLVec2D &pos, double pressure, bool drawQuad);
	
	static MLPolygon calcTangentQuadrangle(double radius1, const MLVec2D &center1, double radius2, const MLVec2D &center2);
	
	MLFixedMultiPolygon _drawnShape;
	double _radiusPrev;
	MLVec2D _posPrev;
};

class FSBrushStroker : public FSStroker
{
public:
	
	//using FSStroker::FSStroker;
	FSBrushStroker(MLSurface *surface, const FSBrushSetting *setting) : FSStroker(surface, setting) {}
	
protected:
	
	void drawFirst(const FSTabletInputData &data);
	void drawInterval(const MLPolygon &polygon, const FSTabletInputData &dataStart, const FSTabletInputData &dataEnd);
	
private:
	
	void drawSegment(const MLVec2D &p1, const MLVec2D &p2, double length, FSTabletInputData &data, double pressureNormalized, double rotationNormalized, double tangentialPressureNormalized, const MLVec2D &tiltNormalized);
	
	void drawDab(const FSTabletInputData &data);
	MLImage drawDabImage(const FSTabletInputData &data, QRect *resultRect);
	
	double _carryOver;
	double _lastMinorRadius;
};

#endif // FSBRUSHSTROKER_H
