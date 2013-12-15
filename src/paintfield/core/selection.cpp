#include "selection.h"

#include "document.h"
#include "closureundocommand.h"
#include <QUndoCommand>

namespace PaintField {

struct Selection::Data
{
	Document *mDocument = 0;
	SelectionSurface mSurface, mOriginalSurface;
	QPointSet mModifiedKeys;
};

Selection::Selection(Document *document) :
	QObject(document),
	d(new Data)
{
	d->mDocument = document;
}

Selection::~Selection()
{
}

SelectionSurface Selection::surface() const
{
	return d->mSurface;
}

void Selection::updateSurface(const SelectionSurface &surface, const QPointSet &keys)
{
	for (const auto &key : keys) {
		d->mSurface[key] = surface[key];
	}
	d->mModifiedKeys |= keys;
	emit updated(keys);
}

void Selection::commitUpdates()
{
	d->mSurface.squeeze();

	auto before = d->mOriginalSurface;
	auto after = d->mSurface;
	auto keys = d->mModifiedKeys;

	auto command = new ClosureUndoCommand(
		[=](){
			d->mSurface = after;
			emit updated(keys);
		},
		[=](){
			d->mSurface = before;
			emit updated(keys);
		});
	d->mDocument->undoStack()->push(command);
}



} // namespace PaintField
