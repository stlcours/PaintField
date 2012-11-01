#include <QtGui>

#include "util.h"
#include "application.h"
#include "toolmanager.h"
#include "palettemanager.h"
#include "workspacemdiareacontroller.h"
#include "module.h"
#include "modulemanager.h"

#include "workspacecontroller.h"

namespace PaintField
{

namespace MenuArranger
{

QMenu *createMenu(const QVariantMap &order);
QMenuBar *createMenuBar(const QVariant &order);
	
void associateMenuWithActions(QMenu *menu, const QActionList &actions);
void associateMenuBarWithActions(QMenuBar *menuBar, const QActionList &actions);

QMenu *createMenu(const QVariantMap &order)
{
	QString menuId = order["menu"].toString();
	if (menuId.isEmpty())
		return 0;
	
	QString menuTitle = app()->menuHash()[menuId];
	
	if (menuTitle.isEmpty())
		menuTitle = order["menu"].toString();
	
	QMenu *menu = new QMenu(menuTitle);
	
	QVariantList children = order["children"].toList();
	
	ActionInfoHash actionInfoHash = app()->actionInfoHash();
	
	for (const QVariant &child : children)
	{
		switch (child.type())
		{
			case QVariant::String:
			{
				QString id = child.toString();
				
				if (id.isEmpty())
					menu->addSeparator();
				else
				{
					ActionInfo actionInfo = actionInfoHash.value(id);
					
					if (actionInfo.text.isEmpty())
						actionInfo.text = id;
					
					WorkspaceMenuAction *action = new WorkspaceMenuAction(menu);
					action->setObjectName(id);
					action->setText(actionInfo.text);
					action->setShortcut(actionInfo.shortcut);
					menu->addAction(action);
				}
				break;
			}
			case QVariant::Map:
			{
				QMenu *childMenu = createMenu(child.toMap());
				if (childMenu)
					menu->addMenu(childMenu);
				break;
			}
			default:
				break;
		}
	}
	
	return menu;
}

QMenuBar *createMenuBar(const QVariant &order)
{
	QMenuBar *menuBar = new QMenuBar();
	
	QVariantList orders = order.toList();
	for (const QVariant &menuOrder : orders)
		menuBar->addMenu(createMenu(menuOrder.toMap()));
	
	return menuBar;
}

void associateMenuWithActions(QMenu *menu, const QActionList &actions)
{
	for (QAction *action : menu->actions())
	{
		WorkspaceMenuAction *menuAction = qobject_cast<WorkspaceMenuAction *>(action);
		if (menuAction)
		{
			QAction *foundAction = findQObjectReverse(actions, menuAction->objectName());
			if (foundAction)
				menuAction->setBackendAction(foundAction);
			else
				menuAction->setEnabled(false);
		}
		else
		{
			if (action->menu())
				associateMenuWithActions(action->menu(), actions);
		}
	}
}

void associateMenuBarWithActions(QMenuBar *menuBar, const QActionList &actions)
{
	for (QAction *action : menuBar->actions())
	{
		QMenu *menu = action->menu();
		if (menu)
			associateMenuWithActions(menu, actions);
	}
}

}




WorkspaceController::WorkspaceController(QObject *parent) :
    QObject(parent)
{
	// create tool / palette manager
	
	_toolManager = new ToolManager(this);
	_paletteManager = new PaletteManager(this);
	
	_mdiAreaController = new WorkspaceMdiAreaController(this);
	
	connect(this, SIGNAL(canvasAdded(CanvasController*)), _mdiAreaController, SLOT(addCanvas(CanvasController*)));
	connect(this, SIGNAL(canvasRemoved(CanvasController*)), _mdiAreaController, SLOT(removeCanvas(CanvasController*)));
	connect(this, SIGNAL(currentCanvasChanged(CanvasController*)), _mdiAreaController, SLOT(setCurrentCanvas(CanvasController*)));
	
	connect(_mdiAreaController, SIGNAL(currentCanvasChanged(CanvasController*)), this, SLOT(setCurrentCanvas(CanvasController*)));
	
	_actions << createAction("paintfield.file.new", this, SLOT(newCanvas()));
	_actions << createAction("paintfield.file.open", this, SLOT(openCanvas()));
}

WorkspaceView *WorkspaceController::createView(QWidget *parent)
{
	WorkspaceView *view = new WorkspaceView(parent);
	_view = view;
	
	QMdiArea *mdiArea = _mdiAreaController->createView();
	view->setCentralWidget(mdiArea);
	
	createWorkspaceItems();
	updateWorkspaceItems();
	updateWorkspaceItemsForCanvas(_currentCanvas);
	
	createMenuBar();
	updateMenuBar();
	
	return view;
}

void WorkspaceController::addModules(const QList<WorkspaceModule *> &modules)
{
	for (auto module : modules)
		addActions(module->actions());
	
	_modules += modules;
}

void WorkspaceController::addNullCanvasModules(const CanvasModuleList &modules)
{
	for (auto module : modules)
		addNullCanvasActions(module->actions());
	
	_nullCanvasModules += modules;
}

void WorkspaceController::newCanvas()
{
	CanvasController *controller = CanvasController::fromNew(this);
	
	if (controller)
	{
		addCanvas(controller);
		setCurrentCanvas(controller);
	}
}

void WorkspaceController::openCanvas()
{
	CanvasController *controller = CanvasController::fromOpen(this);
	
	if (controller)
	{
		addCanvas(controller);
		setCurrentCanvas(controller);
	}
}

bool WorkspaceController::tryClose()
{
	for (CanvasController *controller : _canvasControllers)
	{
		if (!controller->closeCanvas())
			return false;
	}
	deleteLater();
	return true;
}

void WorkspaceController::setFocus()
{
	_view->setFocus();
}

void WorkspaceController::setCurrentCanvas(CanvasController *canvas)
{
	if (_currentCanvas != canvas)
	{
		_currentCanvas = canvas;
		emit currentCanvasChanged(canvas);
		
		if (_view)
		{
			updateWorkspaceItemsForCanvas(canvas);
			updateMenuBar();
		}
	}
}

bool WorkspaceController::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == _view)
	{
		if (event->type() == QEvent::FocusIn)
		{
			emit focused();
			return true;
		}
	}
	
	return false;
}

void WorkspaceController::addCanvas(CanvasController *canvas)
{
	_canvasControllers << canvas;
	connect(canvas, SIGNAL(shouldBeDeleted()), this, SLOT(onCanvasSholudBeDeleted()));
	canvas->addModules(app()->moduleManager()->createCanvasModules(canvas, canvas));
	
	emit canvasAdded(canvas);
}

void WorkspaceController::onCanvasSholudBeDeleted()
{
	CanvasController *canvas = qobject_cast<CanvasController *>(sender());
	if (canvas && _canvasControllers.contains(canvas))
		removeCanvas(canvas);
}

void WorkspaceController::removeCanvas(CanvasController *canvas)
{
	if (_canvasControllers.contains(canvas))
	{
		if (_currentCanvas == canvas)
			setCurrentCanvas(0);
		
		_canvasControllers.removeAll(canvas);
		emit canvasRemoved(canvas);
		canvas->deleteLater();
	}
}

void WorkspaceController::createWorkspaceItems()
{
	QVariantMap itemOrderMap = app()->workspaceItemOrder().toMap();
	
	QVariantMap sidebarOrderMap = itemOrderMap["sidebars"].toMap();
	QVariantMap toolbarOrderMap = itemOrderMap["toolbars"].toMap();
	
	createSidebarsInArea(sidebarOrderMap["left"].toList(), Qt::LeftDockWidgetArea);
	createSidebarsInArea(sidebarOrderMap["right"].toList(), Qt::RightDockWidgetArea);
	
	createToolbarsInArea(toolbarOrderMap["left"].toList(), Qt::LeftToolBarArea);
	createToolbarsInArea(toolbarOrderMap["right"].toList(), Qt::RightToolBarArea);
	createToolbarsInArea(toolbarOrderMap["top"].toList(), Qt::TopToolBarArea);
	createToolbarsInArea(toolbarOrderMap["bottom"].toList(), Qt::BottomToolBarArea);
}

void WorkspaceController::createSidebarsInArea(const QVariantList &ids, Qt::DockWidgetArea area)
{
	for (const QVariant &id : ids)
	{
		auto infos = app()->sidebarInfoHash();
		
		for (auto iter = infos.begin(); iter != infos.end(); ++iter)
		{
			if (iter.key() == id.toString())
			{
				_view->addSidebarFrame(iter.key(), iter->text, area);
				break;
			}
		}
	}
}

void WorkspaceController::createToolbarsInArea(const QVariantList &ids, Qt::ToolBarArea area)
{
	for (const QVariant &id :ids)
	{
		ToolbarInfoHash infos = app()->toolBarInfoHash();
		
		for (auto iter = infos.begin(); iter != infos.end(); ++iter)
		{
			if (iter.key() == id.toString())
			{
				_view->addToolBar(id.toString(), iter->text, area);
				break;
			}
		}
	}
}

void WorkspaceController::updateWorkspaceItems()
{
	for (const QString &name : app()->sidebarNames())
	{
		QWidget *sidebar = createSidebarForWorkspace(app()->modules(), modules(), name);
		if (sidebar)
			_view->setSidebar(name, sidebar);
	}
	
	for (const QString &name : app()->toolbarNames())
	{
		QToolBar *toolBar = _view->toolBar(name);
		if (toolBar)
			updateToolBar(app()->modules(), modules(), currentCanvasModules(), toolBar, name);
	}
}

void WorkspaceController::updateWorkspaceItemsForCanvas(CanvasController *canvas)
{
	Q_UNUSED(canvas)
	
	for (const QString &name : app()->sidebarNames())
	{
		QWidget *sidebar = createSidebarForCanvas(currentCanvasModules(), name);
		if (sidebar)
			_view->setSidebar(name, sidebar);
	}
	
	for (const QString &name : app()->toolbarNames())
	{
		QToolBar *toolBar = _view->toolBar(name);
		if (toolBar)
			updateToolBar(AppModuleList(), WorkspaceModuleList(), currentCanvasModules(), toolBar, name);
	}
}

void WorkspaceController::createMenuBar()
{
	_view->setMenuBar(MenuArranger::createMenuBar(app()->menuBarOrder()));
}

void WorkspaceController::updateMenuBar()
{
	QActionList actions = app()->actions() + this->actions() + currentCanvasActions();
	
	MenuArranger::associateMenuBarWithActions(_view->menuBar(), actions);
}

}