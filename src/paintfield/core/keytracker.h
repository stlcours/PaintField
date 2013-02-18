#pragma once

#include <QObject>
#include <QSet>

class QKeySequence;

namespace PaintField {

class KeyTracker : public QObject
{
	Q_OBJECT
	
public:
	
	explicit KeyTracker(QObject *parent);
	~KeyTracker();
	
	QSet<int> unmodifiedPressedKeys() const;
	QSet<int> pressedKeys() const;
	Qt::KeyboardModifiers modifiers() const;
	
	bool match(const QKeySequence &sequence) const;
	
public slots:
	
	void pressKey(int key);
	void releaseKey(int key);
	
signals:
	
	void pressedKeysChanged(const QSet<int> &pressedKeys);
	
private:
	
	struct Data;
	Data *d;
};

} // namespace PaintField
