#ifndef BRUSHTOOLMODULE_H
#define BRUSHTOOLMODULE_H

#include "paintfield-core/smartpointer.h"
#include "paintfield-core/module.h"

namespace PaintField {

class BrushSettingSidebar;
class BrushPresetManager;
class BrushStrokerFactory;
class BrushStrokerFactoryManager;

class BrushToolModule : public WorkspaceModule
{
	Q_OBJECT
public:
	BrushToolModule(WorkspaceController *workspace, QObject *parent);
	
	BrushPresetManager *presetManager() { return _presetManager; }
	BrushStrokerFactoryManager *sourceFactoryManager() { return _strokerFactoryManager; }
	
	Tool *createTool(const QString &name, CanvasView *parent) override;
	
signals:
	
	void strokerFactoryChanged(BrushStrokerFactory *factory);
	
private slots:
	
	void onStrokerChanged(const QString &name);
	
private:
	
	BrushPresetManager *_presetManager = 0;
	BrushStrokerFactoryManager *_strokerFactoryManager = 0;
};

class BrushToolModuleFactory : public ModuleFactory
{
public:
	
	explicit BrushToolModuleFactory(QObject *parent = 0) : ModuleFactory(parent) {}
	
	void initialize(AppController *app) override;
	
	WorkspaceModuleList createWorkspaceModules(WorkspaceController *workspace, QObject *parent) override
	{
		return { new BrushToolModule(workspace, parent) };
	}
};

}

#endif // BRUSHTOOLMODULE_H
