using UnrealBuildTool;
using System.IO;

public class FlowVisualizerRuntime : ModuleRules
{
	public FlowVisualizerRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		// PF.1 – FMOD Studio optional dependency
		string PluginsDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
		bool bHasFMOD = Directory.Exists(Path.Combine(PluginsDir, "FMODStudio"));
		if (bHasFMOD)
		{
			PublicDependencyModuleNames.Add("FMODStudio");
			PublicDefinitions.Add("WITH_FMOD_STUDIO=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_FMOD_STUDIO=0");
		}
	}
}
