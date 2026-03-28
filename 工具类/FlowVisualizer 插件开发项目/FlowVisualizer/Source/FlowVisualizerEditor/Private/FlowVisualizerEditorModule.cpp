#include "FlowVisualizerEditorModule.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "SFlowVisualizerPanel.h"
#include "FlowPipelineTypes.h"
#include "FlowPipelineRegistry.h"
#include "Misc/ConfigCacheIni.h"
#include "FMODAutoCapture.h"

#define LOCTEXT_NAMESPACE "FlowVisualizerEditor"

const TCHAR* FFlowVisualizerEditorModule::WindowLayoutConfigSection = TEXT("FlowVisualizer.WindowLayout");

void FFlowVisualizerEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FlowVisualizerEditor module loaded"));

	PluginCommands = MakeShareable(new FUICommandList);

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension(
		"WindowLayout",
		EExtensionHook::After,
		PluginCommands,
		FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("FlowVisualizerTitle", "Flow Visualizer"),
				LOCTEXT("FlowVisualizerTooltip", "Open runtime data flow visualization window"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FModuleManager::GetModuleChecked<FFlowVisualizerEditorModule>("FlowVisualizerEditor").OpenVisualizerWindow();
				}))
			);
		})
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// --- Register built-in Demo pipeline (P4.1) ---
	{
		FFlowPipeline Demo;
		Demo.PipelineID = FName(TEXT("Demo"));
		Demo.DisplayName = FText::FromString(TEXT("Demo Pipeline"));

		// Nodes
		FFlowNode InputNode;
		InputNode.NodeID = FName(TEXT("Input"));
		InputNode.DisplayName = FText::FromString(TEXT("Input"));
		InputNode.RelativePosition = FVector2D(0.10f, 0.40f);
		Demo.Nodes.Add(InputNode);

		FFlowNode ProcessNode;
		ProcessNode.NodeID = FName(TEXT("Process"));
		ProcessNode.DisplayName = FText::FromString(TEXT("Process"));
		ProcessNode.RelativePosition = FVector2D(0.40f, 0.40f);
		Demo.Nodes.Add(ProcessNode);

		FFlowNode OutputNode;
		OutputNode.NodeID = FName(TEXT("Output"));
		OutputNode.DisplayName = FText::FromString(TEXT("Output"));
		OutputNode.RelativePosition = FVector2D(0.70f, 0.40f);
		Demo.Nodes.Add(OutputNode);

		// Edges
		FFlowEdge Edge1;
		Edge1.SourceNodeID = FName(TEXT("Input"));
		Edge1.TargetNodeID = FName(TEXT("Process"));
		Demo.Edges.Add(Edge1);

		FFlowEdge Edge2;
		Edge2.SourceNodeID = FName(TEXT("Process"));
		Edge2.TargetNodeID = FName(TEXT("Output"));
		Demo.Edges.Add(Edge2);

		FFlowPipelineRegistry::Get().RegisterPipeline(Demo);
		UE_LOG(LogTemp, Log, TEXT("FlowVisualizerEditor: Registered built-in Demo pipeline"));
	}

	// 注册一条空的蓝图自动管线，运行时动态添加节点
	{
		FFlowPipeline BlueprintAuto;
		BlueprintAuto.PipelineID = TEXT("Blueprint_Auto");
		BlueprintAuto.DisplayName = FText::FromString(TEXT("Blueprint (Auto)"));
		FFlowPipelineRegistry::Get().RegisterPipeline(BlueprintAuto);
	}

	// --- Register built-in FMOD Audio System pipeline (PF.10) ---
	{
		FFlowPipeline FMOD;
		FMOD.PipelineID = FName(TEXT("FMODAudioSystem"));
		FMOD.DisplayName = FText::FromString(TEXT("FMOD Audio System"));

		// Helper lambda to add a node concisely
		auto AddNode = [&FMOD](const TCHAR* ID, const TCHAR* Name, float X, float Y)
		{
			FFlowNode N;
			N.NodeID = FName(ID);
			N.DisplayName = FText::FromString(Name);
			N.RelativePosition = FVector2D(X, Y);
			FMOD.Nodes.Add(N);
		};

		// Helper lambda to add an edge concisely
		auto AddEdge = [&FMOD](const TCHAR* Src, const TCHAR* Dst)
		{
			FFlowEdge E;
			E.SourceNodeID = FName(Src);
			E.TargetNodeID = FName(Dst);
			FMOD.Edges.Add(E);
		};

		// Row 1 (y=0.15): Normal lifecycle — creation to timeline events
		AddNode(TEXT("FMOD_Created"),   TEXT("Created"),   0.10f, 0.15f);
		AddNode(TEXT("FMOD_Started"),   TEXT("Started"),   0.30f, 0.15f);
		AddNode(TEXT("FMOD_SoundPlay"), TEXT("SoundPlay"), 0.50f, 0.15f);
		AddNode(TEXT("FMOD_Marker"),    TEXT("Marker"),    0.70f, 0.15f);
		AddNode(TEXT("FMOD_Beat"),      TEXT("Beat"),      0.90f, 0.15f);

		// Row 2 (y=0.45): Stop / restart / destroy
		AddNode(TEXT("FMOD_Restarted"), TEXT("Restarted"), 0.10f, 0.45f);
		AddNode(TEXT("FMOD_SoundStop"), TEXT("SoundStop"), 0.35f, 0.45f);
		AddNode(TEXT("FMOD_Stopped"),   TEXT("Stopped"),   0.60f, 0.45f);
		AddNode(TEXT("FMOD_Destroyed"), TEXT("Destroyed"), 0.85f, 0.45f);

		// Row 3 (y=0.75): Error / warning / utility
		AddNode(TEXT("FMOD_StartFailed"), TEXT("StartFailed"), 0.10f, 0.75f);
		AddNode(TEXT("FMOD_Virtualized"), TEXT("Virtualized"), 0.30f, 0.75f);
		AddNode(TEXT("FMOD_Restored"),    TEXT("Restored"),    0.50f, 0.75f);
		AddNode(TEXT("FMOD_BankLoad"),    TEXT("BankLoad"),    0.70f, 0.75f);
		AddNode(TEXT("FMOD_GlobalParam"), TEXT("GlobalParam"), 0.90f, 0.75f);

		// Edges — Normal lifecycle flow (Row 1)
		AddEdge(TEXT("FMOD_Created"),   TEXT("FMOD_Started"));
		AddEdge(TEXT("FMOD_Started"),   TEXT("FMOD_SoundPlay"));
		AddEdge(TEXT("FMOD_SoundPlay"), TEXT("FMOD_Marker"));
		AddEdge(TEXT("FMOD_SoundPlay"), TEXT("FMOD_Beat"));

		// Edges — Stop flow (Row 1 → Row 2)
		AddEdge(TEXT("FMOD_SoundPlay"), TEXT("FMOD_SoundStop"));
		AddEdge(TEXT("FMOD_SoundStop"), TEXT("FMOD_Stopped"));
		AddEdge(TEXT("FMOD_Stopped"),   TEXT("FMOD_Destroyed"));

		// Edges — Restart loop (Row 2 → Row 1)
		AddEdge(TEXT("FMOD_Stopped"),   TEXT("FMOD_Restarted"));
		AddEdge(TEXT("FMOD_Restarted"), TEXT("FMOD_Started"));

		// Edges — Error branch (Row 1 → Row 3)
		AddEdge(TEXT("FMOD_Started"),   TEXT("FMOD_StartFailed"));

		// Edges — Warning / recovery branch (Row 2 ↔ Row 3)
		AddEdge(TEXT("FMOD_SoundPlay"), TEXT("FMOD_Virtualized"));
		AddEdge(TEXT("FMOD_Virtualized"), TEXT("FMOD_Restored"));
		AddEdge(TEXT("FMOD_Restored"),  TEXT("FMOD_SoundPlay"));

		FFlowPipelineRegistry::Get().RegisterPipeline(FMOD);
		UE_LOG(LogTemp, Log, TEXT("FlowVisualizerEditor: Registered built-in FMODAudioSystem pipeline (%d nodes, %d edges)"),
			FMOD.Nodes.Num(), FMOD.Edges.Num());
	}

	// --- Initialize FMOD Auto-Capture (PA.8) ---
#if WITH_FMOD_STUDIO
	FFMODAutoCapture::Get().Initialize();
#endif
}

void FFlowVisualizerEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FlowVisualizerEditor module unloaded"));

	// --- Shutdown FMOD Auto-Capture (PA.8) ---
#if WITH_FMOD_STUDIO
	FFMODAutoCapture::Get().Shutdown();
#endif

	if (VisualizerWindow.IsValid())
	{
		SaveWindowLayout();
		VisualizerWindow->RequestDestroyWindow();
		VisualizerWindow.Reset();
	}

	if (MenuExtender.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
		}
	}

	MenuExtender.Reset();
	PluginCommands.Reset();
}

void FFlowVisualizerEditorModule::OpenVisualizerWindow()
{
	// Step 1 — Prevent duplicate window
	if (VisualizerWindow.IsValid())
	{
		if (VisualizerWindow->IsWindowMinimized())
		{
			VisualizerWindow->Restore();
		}
		VisualizerWindow->BringToFront();
		return;
	}

	// Step 2 — Load saved layout
	FVector2D LoadedPosition;
	FVector2D LoadedSize;
	LoadWindowLayout(LoadedPosition, LoadedSize);

	const bool bHasSavedPosition = (LoadedPosition.X >= 0.0f && LoadedPosition.Y >= 0.0f);

	// Step 3 — Create SWindow
	if (bHasSavedPosition)
	{
		VisualizerWindow = SNew(SWindow)
			.Title(FText::FromString(TEXT("Flow Visualizer")))
			.ClientSize(LoadedSize)
			.ScreenPosition(LoadedPosition)
			.AutoCenter(EAutoCenter::None)
			.SupportsMinimize(true)
			.SupportsMaximize(true)
			.IsTopmostWindow(false)
			.SizingRule(ESizingRule::UserSized)
			[
				SNew(SFlowVisualizerPanel)
			];
	}
	else
	{
		VisualizerWindow = SNew(SWindow)
			.Title(FText::FromString(TEXT("Flow Visualizer")))
			.ClientSize(LoadedSize)
			.AutoCenter(EAutoCenter::PreferredWorkArea)
			.SupportsMinimize(true)
			.SupportsMaximize(true)
			.IsTopmostWindow(false)
			.SizingRule(ESizingRule::UserSized)
			[
				SNew(SFlowVisualizerPanel)
			];
	}

	// Step 4 — Set close callback
	VisualizerWindow->SetOnWindowClosed(
		FOnWindowClosed::CreateRaw(this, &FFlowVisualizerEditorModule::OnWindowClosed));

	// Step 5 — Add to Slate Application
	FSlateApplication::Get().AddWindow(VisualizerWindow.ToSharedRef());
}

void FFlowVisualizerEditorModule::OnWindowClosed(const TSharedRef<SWindow>& ClosedWindow)
{
	SaveWindowLayout();
	VisualizerWindow.Reset();
}

void FFlowVisualizerEditorModule::SaveWindowLayout()
{
	if (!VisualizerWindow.IsValid())
	{
		return;
	}

	FVector2D Pos = VisualizerWindow->GetPositionInScreen();
	FVector2D Size = VisualizerWindow->GetSizeInScreen();

	GConfig->SetFloat(WindowLayoutConfigSection, TEXT("PosX"), Pos.X, GEditorPerProjectIni);
	GConfig->SetFloat(WindowLayoutConfigSection, TEXT("PosY"), Pos.Y, GEditorPerProjectIni);
	GConfig->SetFloat(WindowLayoutConfigSection, TEXT("SizeX"), Size.X, GEditorPerProjectIni);
	GConfig->SetFloat(WindowLayoutConfigSection, TEXT("SizeY"), Size.Y, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

void FFlowVisualizerEditorModule::LoadWindowLayout(FVector2D& OutPosition, FVector2D& OutSize)
{
	float PosX, PosY, SizeX, SizeY;
	bool bHasLayout = true;

	bHasLayout &= GConfig->GetFloat(WindowLayoutConfigSection, TEXT("PosX"), PosX, GEditorPerProjectIni);
	bHasLayout &= GConfig->GetFloat(WindowLayoutConfigSection, TEXT("PosY"), PosY, GEditorPerProjectIni);
	bHasLayout &= GConfig->GetFloat(WindowLayoutConfigSection, TEXT("SizeX"), SizeX, GEditorPerProjectIni);
	bHasLayout &= GConfig->GetFloat(WindowLayoutConfigSection, TEXT("SizeY"), SizeY, GEditorPerProjectIni);

	if (bHasLayout)
	{
		OutPosition = FVector2D(PosX, PosY);
		OutSize = FVector2D(SizeX, SizeY);
	}
	else
	{
		OutPosition = FVector2D(-1.0f, -1.0f);
		OutSize = FVector2D(1200.0f, 700.0f);
	}

	// Safety clamp
	OutSize.X = FMath::Clamp(OutSize.X, 400.0f, 3840.0f);
	OutSize.Y = FMath::Clamp(OutSize.Y, 300.0f, 2160.0f);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlowVisualizerEditorModule, FlowVisualizerEditor)
