// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "PerformanceStatsPlugin/Public/PerformanceStatsSettings.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePerformanceStatsSettings() {}
// Cross Module References
	PERFORMANCESTATSPLUGIN_API UClass* Z_Construct_UClass_UPerformanceStatsSettings_NoRegister();
	PERFORMANCESTATSPLUGIN_API UClass* Z_Construct_UClass_UPerformanceStatsSettings();
	DEVELOPERSETTINGS_API UClass* Z_Construct_UClass_UDeveloperSettings();
	UPackage* Z_Construct_UPackage__Script_PerformanceStatsPlugin();
// End Cross Module References
	void UPerformanceStatsSettings::StaticRegisterNativesUPerformanceStatsSettings()
	{
	}
	UClass* Z_Construct_UClass_UPerformanceStatsSettings_NoRegister()
	{
		return UPerformanceStatsSettings::StaticClass();
	}
	struct Z_Construct_UClass_UPerformanceStatsSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnablePerformanceStats_MetaData[];
#endif
		static void NewProp_bEnablePerformanceStats_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnablePerformanceStats;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ConsoleTitle_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ConsoleTitle;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RefreshInterval_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_RefreshInterval;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_TopItemsCount_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_TopItemsCount;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bWaitForKeyOnExit_MetaData[];
#endif
		static void NewProp_bWaitForKeyOnExit_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bWaitForKeyOnExit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_FrameTimeGreenMax_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_FrameTimeGreenMax;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_FrameTimeYellowMax_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_FrameTimeYellowMax;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_AssetCostGreenMax_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_AssetCostGreenMax;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_AssetCostYellowMax_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_AssetCostYellowMax;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackParticles_MetaData[];
#endif
		static void NewProp_bTrackParticles_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackParticles;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackMaterials_MetaData[];
#endif
		static void NewProp_bTrackMaterials_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackMaterials;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackSkeletalMeshes_MetaData[];
#endif
		static void NewProp_bTrackSkeletalMeshes_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackSkeletalMeshes;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackStaticMeshes_MetaData[];
#endif
		static void NewProp_bTrackStaticMeshes_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackStaticMeshes;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackBlueprints_MetaData[];
#endif
		static void NewProp_bTrackBlueprints_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackBlueprints;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackAudio_MetaData[];
#endif
		static void NewProp_bTrackAudio_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackAudio;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bTrackLights_MetaData[];
#endif
		static void NewProp_bTrackLights_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bTrackLights;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPerformanceStatsSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UDeveloperSettings,
		(UObject* (*)())Z_Construct_UPackage__Script_PerformanceStatsPlugin,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::Class_MetaDataParams[] = {
		{ "DisplayName", "Performance Stats Settings" },
		{ "IncludePath", "PerformanceStatsSettings.h" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats_MetaData[] = {
		{ "Category", "General" },
		{ "Comment", "/** ?\xc7\xb7????????\xdc\xbc??? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "?\xc7\xb7????????\xdc\xbc???" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bEnablePerformanceStats = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats = { "bEnablePerformanceStats", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_ConsoleTitle_MetaData[] = {
		{ "Category", "General" },
		{ "Comment", "/** ????\xcc\xa8???\xda\xb1??? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "????\xcc\xa8???\xda\xb1???" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_ConsoleTitle = { "ConsoleTitle", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, ConsoleTitle), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_ConsoleTitle_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_ConsoleTitle_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_RefreshInterval_MetaData[] = {
		{ "Category", "General" },
		{ "ClampMax", "5.0" },
		{ "ClampMin", "0.1" },
		{ "Comment", "/** ????\xcb\xa2?\xc2\xbc??????\xeb\xa3\xa9 */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "????\xcb\xa2?\xc2\xbc??????\xeb\xa3\xa9" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_RefreshInterval = { "RefreshInterval", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, RefreshInterval), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_RefreshInterval_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_RefreshInterval_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_TopItemsCount_MetaData[] = {
		{ "Category", "General" },
		{ "ClampMax", "50" },
		{ "ClampMin", "5" },
		{ "Comment", "/** ??\xca\xbe\xc7\xb0 N ???????????\xdc\xb5???\xc4\xbf */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "??\xca\xbe\xc7\xb0 N ???????????\xdc\xb5???\xc4\xbf" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_TopItemsCount = { "TopItemsCount", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, TopItemsCount), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_TopItemsCount_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_TopItemsCount_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit_MetaData[] = {
		{ "Category", "General" },
		{ "Comment", "/** ???\xda\xb9\xd8\xb1?\xca\xb1?\xc7\xb7??\xc8\xb4????? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "???\xda\xb9\xd8\xb1?\xca\xb1?\xc7\xb7??\xc8\xb4?????" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bWaitForKeyOnExit = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit = { "bWaitForKeyOnExit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeGreenMax_MetaData[] = {
		{ "Category", "Thresholds" },
		{ "ClampMax", "100.0" },
		{ "ClampMin", "1.0" },
		{ "Comment", "/** \xd6\xa1\xca\xb1?? - ??\xc9\xab????\xd6\xb5 (ms) */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd6\xa1\xca\xb1?? - ??\xc9\xab????\xd6\xb5 (ms)" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeGreenMax = { "FrameTimeGreenMax", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, FrameTimeGreenMax), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeGreenMax_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeGreenMax_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeYellowMax_MetaData[] = {
		{ "Category", "Thresholds" },
		{ "ClampMax", "100.0" },
		{ "ClampMin", "1.0" },
		{ "Comment", "/** \xd6\xa1\xca\xb1?? - ??\xc9\xab????\xd6\xb5 (ms) */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd6\xa1\xca\xb1?? - ??\xc9\xab????\xd6\xb5 (ms)" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeYellowMax = { "FrameTimeYellowMax", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, FrameTimeYellowMax), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeYellowMax_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeYellowMax_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostGreenMax_MetaData[] = {
		{ "Category", "Thresholds" },
		{ "ClampMax", "50.0" },
		{ "ClampMin", "0.1" },
		{ "Comment", "/** ?????\xca\xb2????? - ??\xc9\xab????\xd6\xb5 (ms) */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "?????\xca\xb2????? - ??\xc9\xab????\xd6\xb5 (ms)" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostGreenMax = { "AssetCostGreenMax", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, AssetCostGreenMax), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostGreenMax_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostGreenMax_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostYellowMax_MetaData[] = {
		{ "Category", "Thresholds" },
		{ "ClampMax", "50.0" },
		{ "ClampMin", "0.1" },
		{ "Comment", "/** ?????\xca\xb2????? - ??\xc9\xab????\xd6\xb5 (ms) */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "?????\xca\xb2????? - ??\xc9\xab????\xd6\xb5 (ms)" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostYellowMax = { "AssetCostYellowMax", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UPerformanceStatsSettings, AssetCostYellowMax), METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostYellowMax_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostYellowMax_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7??????\xcf\xb5\xcd\xb3 */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7??????\xcf\xb5\xcd\xb3" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackParticles = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles = { "bTrackParticles", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7?\xd9\xb2??? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7?\xd9\xb2???" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackMaterials = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials = { "bTrackMaterials", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7?\xd9\xb9??????? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7?\xd9\xb9???????" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackSkeletalMeshes = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes = { "bTrackSkeletalMeshes", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7?\xd9\xbe?\xcc\xac???? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7?\xd9\xbe?\xcc\xac????" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackStaticMeshes = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes = { "bTrackStaticMeshes", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7?? Blueprint */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7?? Blueprint" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackBlueprints = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints = { "bTrackBlueprints", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7????\xc6\xb5 */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7????\xc6\xb5" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackAudio = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio = { "bTrackAudio", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights_MetaData[] = {
		{ "Category", "Categories" },
		{ "Comment", "/** \xd7\xb7?\xd9\xb9??? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsSettings.h" },
		{ "ToolTip", "\xd7\xb7?\xd9\xb9???" },
	};
#endif
	void Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights_SetBit(void* Obj)
	{
		((UPerformanceStatsSettings*)Obj)->bTrackLights = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights = { "bTrackLights", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UPerformanceStatsSettings), &Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights_SetBit, METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UPerformanceStatsSettings_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bEnablePerformanceStats,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_ConsoleTitle,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_RefreshInterval,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_TopItemsCount,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bWaitForKeyOnExit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeGreenMax,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_FrameTimeYellowMax,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostGreenMax,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_AssetCostYellowMax,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackParticles,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackMaterials,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackSkeletalMeshes,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackStaticMeshes,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackBlueprints,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackAudio,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UPerformanceStatsSettings_Statics::NewProp_bTrackLights,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPerformanceStatsSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPerformanceStatsSettings>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UPerformanceStatsSettings_Statics::ClassParams = {
		&UPerformanceStatsSettings::StaticClass,
		"PerformanceStats",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UPerformanceStatsSettings_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::PropPointers),
		0,
		0x001000A6u,
		METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsSettings_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsSettings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UPerformanceStatsSettings()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UPerformanceStatsSettings_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UPerformanceStatsSettings, 3730648347);
	template<> PERFORMANCESTATSPLUGIN_API UClass* StaticClass<UPerformanceStatsSettings>()
	{
		return UPerformanceStatsSettings::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UPerformanceStatsSettings(Z_Construct_UClass_UPerformanceStatsSettings, &UPerformanceStatsSettings::StaticClass, TEXT("/Script/PerformanceStatsPlugin"), TEXT("UPerformanceStatsSettings"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPerformanceStatsSettings);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
