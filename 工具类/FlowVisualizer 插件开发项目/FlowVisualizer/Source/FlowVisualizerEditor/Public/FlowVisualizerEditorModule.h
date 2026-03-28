#pragma once

#include "Modules/ModuleManager.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

class SWindow;

class FFlowVisualizerEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OpenVisualizerWindow();
	void OnWindowClosed(const TSharedRef<SWindow>& ClosedWindow);

	/** Save current window position and size to GConfig (EditorPerProjectUserSettings). */
	void SaveWindowLayout();

	/** Load saved window position and size from GConfig. Returns defaults if no saved data exists. */
	void LoadWindowLayout(FVector2D& OutPosition, FVector2D& OutSize);

	static const TCHAR* WindowLayoutConfigSection;

	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr<SWindow> VisualizerWindow;
};
