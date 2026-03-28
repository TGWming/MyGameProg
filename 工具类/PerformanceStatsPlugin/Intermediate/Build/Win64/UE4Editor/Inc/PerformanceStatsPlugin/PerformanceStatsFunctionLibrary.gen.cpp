// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "PerformanceStatsPlugin/Public/PerformanceStatsFunctionLibrary.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePerformanceStatsFunctionLibrary() {}
// Cross Module References
	PERFORMANCESTATSPLUGIN_API UClass* Z_Construct_UClass_UPerformanceStatsFunctionLibrary_NoRegister();
	PERFORMANCESTATSPLUGIN_API UClass* Z_Construct_UClass_UPerformanceStatsFunctionLibrary();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_PerformanceStatsPlugin();
// End Cross Module References
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execIsPerformanceMonitorPaused)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=UPerformanceStatsFunctionLibrary::IsPerformanceMonitorPaused();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execTogglePausePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::TogglePausePerformanceMonitor();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execResumePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::ResumePerformanceMonitor();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execPausePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::PausePerformanceMonitor();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execIsPerformanceMonitorEnabled)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=UPerformanceStatsFunctionLibrary::IsPerformanceMonitorEnabled();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execTogglePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::TogglePerformanceMonitor();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execDisablePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::DisablePerformanceMonitor();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UPerformanceStatsFunctionLibrary::execEnablePerformanceMonitor)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		UPerformanceStatsFunctionLibrary::EnablePerformanceMonitor();
		P_NATIVE_END;
	}
	void UPerformanceStatsFunctionLibrary::StaticRegisterNativesUPerformanceStatsFunctionLibrary()
	{
		UClass* Class = UPerformanceStatsFunctionLibrary::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "DisablePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execDisablePerformanceMonitor },
			{ "EnablePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execEnablePerformanceMonitor },
			{ "IsPerformanceMonitorEnabled", &UPerformanceStatsFunctionLibrary::execIsPerformanceMonitorEnabled },
			{ "IsPerformanceMonitorPaused", &UPerformanceStatsFunctionLibrary::execIsPerformanceMonitorPaused },
			{ "PausePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execPausePerformanceMonitor },
			{ "ResumePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execResumePerformanceMonitor },
			{ "TogglePausePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execTogglePausePerformanceMonitor },
			{ "TogglePerformanceMonitor", &UPerformanceStatsFunctionLibrary::execTogglePerformanceMonitor },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ?\xd8\xb1????\xdc\xbc??\xd8\xb4??? */" },
		{ "Keywords", "performance monitor disable close" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "?\xd8\xb1????\xdc\xbc??\xd8\xb4???" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "DisablePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ???????\xdc\xbc??\xd8\xb4??? */" },
		{ "Keywords", "performance monitor enable open" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "???????\xdc\xbc??\xd8\xb4???" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "EnablePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics
	{
		struct PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorEnabled_Parms
		{
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorEnabled_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorEnabled_Parms), &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ???????\xdc\xbc????\xc7\xb7????? */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "???????\xdc\xbc????\xc7\xb7?????" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "IsPerformanceMonitorEnabled", nullptr, nullptr, sizeof(PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorEnabled_Parms), Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics
	{
		struct PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorPaused_Parms
		{
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorPaused_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorPaused_Parms), &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ?????\xc7\xb7?????\xcd\xa3 */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "?????\xc7\xb7?????\xcd\xa3" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "IsPerformanceMonitorPaused", nullptr, nullptr, sizeof(PerformanceStatsFunctionLibrary_eventIsPerformanceMonitorPaused_Parms), Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ??\xcd\xa3????\xcb\xa2?? */" },
		{ "Keywords", "performance pause freeze" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "??\xcd\xa3????\xcb\xa2??" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "PausePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ?\xd6\xb8?????\xcb\xa2?? */" },
		{ "Keywords", "performance resume continue" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "?\xd6\xb8?????\xcb\xa2??" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "ResumePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ?\xd0\xbb???\xcd\xa3\xd7\xb4\xcc\xac */" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "?\xd0\xbb???\xcd\xa3\xd7\xb4\xcc\xac" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "TogglePausePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics::Function_MetaDataParams[] = {
		{ "Category", "Performance Stats" },
		{ "Comment", "/** ?\xd0\xbb????\xdc\xbc??\xd8\xb4??\xda\xbf??? */" },
		{ "Keywords", "performance monitor toggle" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
		{ "ToolTip", "?\xd0\xbb????\xdc\xbc??\xd8\xb4??\xda\xbf???" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UPerformanceStatsFunctionLibrary, nullptr, "TogglePerformanceMonitor", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_UPerformanceStatsFunctionLibrary_NoRegister()
	{
		return UPerformanceStatsFunctionLibrary::StaticClass();
	}
	struct Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_PerformanceStatsPlugin,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_DisablePerformanceMonitor, "DisablePerformanceMonitor" }, // 4260390926
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_EnablePerformanceMonitor, "EnablePerformanceMonitor" }, // 4241144134
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorEnabled, "IsPerformanceMonitorEnabled" }, // 1841481593
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_IsPerformanceMonitorPaused, "IsPerformanceMonitorPaused" }, // 1386608894
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_PausePerformanceMonitor, "PausePerformanceMonitor" }, // 1419345451
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_ResumePerformanceMonitor, "ResumePerformanceMonitor" }, // 954661954
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePausePerformanceMonitor, "TogglePausePerformanceMonitor" }, // 1866018096
		{ &Z_Construct_UFunction_UPerformanceStatsFunctionLibrary_TogglePerformanceMonitor, "TogglePerformanceMonitor" }, // 2280591879
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PerformanceStatsFunctionLibrary.h" },
		{ "ModuleRelativePath", "Public/PerformanceStatsFunctionLibrary.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPerformanceStatsFunctionLibrary>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::ClassParams = {
		&UPerformanceStatsFunctionLibrary::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UPerformanceStatsFunctionLibrary()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UPerformanceStatsFunctionLibrary_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UPerformanceStatsFunctionLibrary, 396567659);
	template<> PERFORMANCESTATSPLUGIN_API UClass* StaticClass<UPerformanceStatsFunctionLibrary>()
	{
		return UPerformanceStatsFunctionLibrary::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UPerformanceStatsFunctionLibrary(Z_Construct_UClass_UPerformanceStatsFunctionLibrary, &UPerformanceStatsFunctionLibrary::StaticClass, TEXT("/Script/PerformanceStatsPlugin"), TEXT("UPerformanceStatsFunctionLibrary"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPerformanceStatsFunctionLibrary);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
