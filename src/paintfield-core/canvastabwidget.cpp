#include <QUrl>
#include "workspacecontroller.h"
#include "appcontroller.h"
#include "workspacemanager.h"
#include "debug.h"

#include "canvastabwidget.h"

namespace PaintField {

struct CanvasTabWidget::Data
{
};

CanvasTabWidget::CanvasTabWidget(WorkspaceController *workspace, QWidget *parent) :
    WorkspaceTabWidget(workspace, parent),
    d(new Data)
{
	new CanvasTabBar(this);
	
	setTabsClosable(true);
	setAutoDeletionEnabled(true);
	
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(activate()));
	connect(this, SIGNAL(tabClicked()), this, SIGNAL(activated()));
	connect(this, SIGNAL(tabMovedIn(QWidget*)), this, SIGNAL(activated()));
	
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
	
	connect(workspace, SIGNAL(currentCanvasChanged(CanvasController*)), this, SLOT(onCurrentCanvasChanged(CanvasController*)));
	connect(this, SIGNAL(currentCanvasChanged(CanvasController*)), workspace, SLOT(setCurrentCanvas(CanvasController*)));
}

CanvasTabWidget::~CanvasTabWidget()
{
	delete d;
}

bool CanvasTabWidget::tabIsInsertable(DockTabWidget *other, int index)
{
	Q_UNUSED(index)
	
	CanvasTabWidget *tabWidget = qobject_cast<CanvasTabWidget *>(other);
	return tabWidget;
}

void CanvasTabWidget::insertTab(int index, QWidget *widget, const QString &title)
{
	Q_UNUSED(title)
	
	CanvasView *canvasView = qobject_cast<CanvasView *>(widget);
	if (!canvasView)
		return;
	
	insertCanvas(index, canvasView->controller());
}

QObject *CanvasTabWidget::createNew()
{
	return new CanvasTabWidget(workspace(), 0);
}

void CanvasTabWidget::memorizeTransforms()
{
	for (auto view : canvasViews())
		view->memorizeTransform();
}

void CanvasTabWidget::restoreTransforms()
{
	for (auto view : canvasViews())
		view->restoreTransform();
}

void CanvasTabWidget::insertCanvas(int index, CanvasController *canvas)
{
	workspace()->addCanvas(canvas);
	DockTabWidget::insertTab(index, canvas->view(), canvas->document()->fileName());
}

QList<CanvasView *> CanvasTabWidget::canvasViews()
{
	QList<CanvasView *> list;
	
	for (int i = 0; i < count(); ++i)
	{
		auto view = canvasViewAt(i);
		if (view)
			list << view;
	}
	
	return list;
}

void CanvasTabWidget::onCurrentCanvasChanged(CanvasController *canvas)
{
	if (!canvas)
		return;
	
	bool set = false;
	
	for (int i = 0; i < count(); ++i)
	{
		if (canvas->view() == widget(i))
		{
			setCurrentIndex(i);
			set = true;
		}
	}
	
	if (set)
		activate();
}

bool CanvasTabWidget::tryClose()
{
	for (auto canvasView : canvasViews())
	{
		if (!canvasView->controller()->closeCanvas())
			return false;
	}
	
	return true;
}

void CanvasTabWidget::activate()
{
	auto canvasView = canvasViewAt(currentIndex());
	emit currentCanvasChanged(canvasView ? canvasView->controller() : 0);
	emit activated();
}

void CanvasTabWidget::onTabCloseRequested(int index)
{
	auto canvasView = canvasViewAt(index);
	if (canvasView)
		canvasView->controller()->closeCanvas();
}
CanvasView *CanvasTabWidget::canvasViewAt(int index)
{
	return qobject_cast<CanvasView *>(widget(index));
}

CanvasTabBar::CanvasTabBar(CanvasTabWidget *tabWidget, QWidget *parent) :
    DockTabBar(tabWidget, parent),
    _tabWidget(tabWidget)
{
	setAcceptDrops(true);
}

void CanvasTabBar::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void CanvasTabBar::dropEvent(QDropEvent *event)
{
	auto mimeData = event->mimeData();
	
	if (mimeData->hasUrls())
	{
		for (const QUrl &url : mimeData->urls())
		{
			auto canvas = CanvasController::fromFile(url.toLocalFile());
			if (canvas)
				_tabWidget->insertCanvas(insertionIndexAt(event->pos()), canvas);
		}
		event->acceptProposedAction();
	}
}


} // namespace PaintField