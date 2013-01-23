#ifndef COLORMODULE_H
#define COLORMODULE_H

#include "paintfield-core/smartpointer.h"
#include "paintfield-core/module.h"

namespace PaintField
{

class ColorSidebar;

class ColorUIModule : public WorkspaceModule
{
	Q_OBJECT
public:
	ColorUIModule(WorkspaceController *workspace, QObject *parent);
	
signals:
	
public slots:
};

class ColorUIModuleFactory : public ModuleFactory
{
	Q_OBJECT
public:
	
	ColorUIModuleFactory(QObject *parent = 0) : ModuleFactory(parent) {}
	
	void initialize(AppController *app) override;
	
	QList<WorkspaceModule *> createWorkspaceModules(WorkspaceController *workspace, QObject *parent) override
	{
		return { new ColorUIModule(workspace, parent) };
	}
};

}

#endif // COLORMODULE_H