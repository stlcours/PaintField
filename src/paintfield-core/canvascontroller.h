#ifndef CANVASCONTROLLER_H
#define CANVASCONTROLLER_H

#include <QObject>
#include <QPointer>

#include "module.h"

namespace PaintField
{

class CanvasModule;
class Document;
class CanvasView;
class WorkspaceController;

class CanvasController : public QObject
{
	Q_OBJECT
public:
	
	/**
	 * Constructs a canvas controller.
	 * @param document
	 * @param parent
	 */
	CanvasController(Document *document, WorkspaceController *parent);
	
	/**
	 * Shows a dialog, creates a new file and creates a controller from it.
	 * @param parent The new controller's parent
	 * @return The created controller
	 */
	static CanvasController *fromNew(WorkspaceController *parent);
	
	/**
	 * Shows a dialog, opens a file and creates a controller from it.
	 * @param parent The new controller's parent
	 * @return The created controller
	 */
	static CanvasController *fromOpen(WorkspaceController *parent);
	
	/**
	 * @return The workspace controller which have the canvas controller
	 */
	WorkspaceController *workspace() { return reinterpret_cast<WorkspaceController *>(parent()); }
	
	/**
	 * @return The document the canvas handles
	 */
	Document *document() { return _document; }
	
	void addActions(const QActionList &actions) { _actions += actions; }
	
	/**
	 * @return The actions which belongs to the controller.
	 */
	QActionList actions() { return _actions; }
	
	void addModules(const CanvasModuleList &modules);
	CanvasModuleList modules() { return _modules; }
	
	CanvasView *createView(QWidget *parent = 0);
	
signals:
	
	/**
	 * Emitted when the canvas should be deleted.
	 */
	void shouldBeDeleted();
	
public slots:
	
	/**
	 * Shows a dialog and saves the canvas into a new file.
	 * @return false if cancelled or failed
	 */
	bool saveAsCanvas();
	
	/**
	 * Shows a dialog and saves the canvas.
	 * @return false if cancelled or failed
	 */
	bool saveCanvas();
	
	/**
	 * Shows a warning dialog and emits shouldBeClosed() if not cancelled.
	 * @return false if cancelled
	 */
	bool closeCanvas();
	
private:
	
	Document *_document = 0;
	QPointer<CanvasView> _view;
	QActionList _actions;
	CanvasModuleList _modules;
};

}

#endif // CANVASCONTROLLER_H