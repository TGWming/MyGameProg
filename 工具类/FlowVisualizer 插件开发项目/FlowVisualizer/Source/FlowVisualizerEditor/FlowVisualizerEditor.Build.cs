using UnrealBuildTool;
using System.IO;

public class FlowVisualizerEditor : ModuleRules
{
	public FlowVisualizerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FlowVisualizerRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd",
			"EditorStyle",
			"LevelEditor",
			"InputCore"
		});

		// PA.1 – FMOD Studio optional dependency (mirrors PF.1 in Runtime)
		string PluginsDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
		bool bHasFMOD = Directory.Exists(Path.Combine(PluginsDir, "FMODStudio"));
		if (bHasFMOD)
		{
			PrivateDependencyModuleNames.Add("FMODStudio");
			PrivateDefinitions.Add("WITH_FMOD_STUDIO=1");
		}
		else
		{
			PrivateDefinitions.Add("WITH_FMOD_STUDIO=0");
		}
	}
}
