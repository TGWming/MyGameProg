// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Core/SoulsCameraStateMachine.h"
#include "Engine/DataTable.h"
#include "Camera/Config/CameraConfigService.h"
#include "Camera/Core/SoulsCameraManager.h"

USoulsCameraStateMachine::USoulsCameraStateMachine()
	: StatesDataTable(nullptr)
	, ConfigService(nullptr)
	, bUseConfigService(false)
	, CurrentStateName(NAME_None)
	, PreviousStateName(NAME_None)
	, BlendAlpha(1.0f)
	, TransitionTimeRemaining(0.0f)
	, TransitionTotalTime(0.0f)
	, CurrentBlendType(ECameraBlendType::SmoothStep)
	, bIsInitialized(false)
{
}

//========================================
// Initialization
//========================================

void USoulsCameraStateMachine::Initialize(UDataTable* InStatesDataTable)
{
	if (!InStatesDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraStateMachine::Initialize - Invalid DataTable provided"));
		return;
	}

	// Verify the DataTable row structure
	if (!InStatesDataTable->GetRowStruct()->IsChildOf(FCameraStateConfig::StaticStruct()))
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraStateMachine::Initialize - DataTable row struct is not FCameraStateConfig"));
		return;
	}

	StatesDataTable = InStatesDataTable;
	
	// Cache all state configs for quick lookup
	CacheStateConfigs();

	// Set default state if available
	if (CachedConfigs.Num() > 0)
	{
		// Try to find a default exploration state first
		static const FName DefaultStateName = TEXT("Explore_Default");
		if (CachedConfigs.Contains(DefaultStateName))
		{
			CurrentStateName = DefaultStateName;
		}
		else
		{
			// Fall back to first available state
			for (const auto& Pair : CachedConfigs)
		{
			CurrentStateName = Pair.Key;
			break;
		}
	}
}

	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("USoulsCameraStateMachine::Initialize - Initialized with %d states, current state: %s"),
		CachedConfigs.Num(), *CurrentStateName.ToString());
}

void USoulsCameraStateMachine::InitializeWithConfigService(UCameraConfigService* InConfigService)
{
    UE_LOG(LogTemp, Warning, TEXT("=== InitializeWithConfigService START ==="));
    
    if (!InConfigService)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeWithConfigService - ConfigService is NULL"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("InitializeWithConfigService - ConfigService is valid"));

    if (!InConfigService->IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeWithConfigService - ConfigService is NOT initialized"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("InitializeWithConfigService - ConfigService is initialized"));

    // Store reference and set flag
    ConfigService = InConfigService;
    bUseConfigService = true;

    // Clear legacy DataTable reference to avoid confusion
    StatesDataTable = nullptr;

    // Clear legacy cached data (will use ConfigService instead)
    CachedConfigs.Empty();
    CategoryToStates.Empty();

    // ★★★ 关键：必须在这里设置 bIsInitialized = true ★★★
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("InitializeWithConfigService - bIsInitialized set to TRUE"));

    // Set initial state directly (不通过 RequestStateChange，避免循环检查)
    FName DefaultStateName = FName(TEXT("Explore_Default"));
    
    if (DoesStateExistInService(DefaultStateName))
    {
        CurrentStateName = DefaultStateName;
        PreviousStateName = NAME_None;
        BlendAlpha = 1.0f;
        TransitionTimeRemaining = 0.0f;
        TransitionTotalTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("InitializeWithConfigService - Initial state set to: %s"), *CurrentStateName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeWithConfigService - 'Explore_Default' not found, trying fallback..."));
        
        // Fallback: get first available state
        TArray<FName> AllStates = ConfigService->GetAllStateNames();
        UE_LOG(LogTemp, Log, TEXT("InitializeWithConfigService - Available states count: %d"), AllStates.Num());
        
        if (AllStates.Num() > 0)
        {
            CurrentStateName = AllStates[0];
            PreviousStateName = NAME_None;
            BlendAlpha = 1.0f;
            TransitionTimeRemaining = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("InitializeWithConfigService - Fallback state: %s"), *CurrentStateName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InitializeWithConfigService - No states available!"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("=== InitializeWithConfigService END - IsInitialized: %s ==="), 
        IsInitialized() ? TEXT("TRUE") : TEXT("FALSE"));
}

bool USoulsCameraStateMachine::IsInitialized() const
{
    // 首先检查基本标志
    if (!bIsInitialized)
    {
        return false;
    }
    
    // 新架构：只需要 ConfigService 存在
    if (bUseConfigService)
    {
        return ConfigService != nullptr;
    }

    // 旧架构：需要 DataTable
    return StatesDataTable != nullptr;
}

//========================================
// State Management
//========================================

bool USoulsCameraStateMachine::RequestStateChange(FName NewStateName, bool bForce)
{
	//========================================
	// 状态名映射（兼容旧代码和输入错误）
	// State name mapping for legacy compatibility
	//========================================
	static const TMap<FName, FName> StateNameMapping = {
		// 探索状态映射
		{ FName("Exploration"), FName("Explore_Default") },
		{ FName("FreeExploration"), FName("Explore_Default") },
		{ FName("FreeExploration_Default"), FName("Explore_Default") },
		{ FName("Default"), FName("Explore_Default") },
		// 战斗状态映射
		{ FName("Combat"), FName("Combat_Default") },
		{ FName("Combat_LockOn"), FName("LockOn_Hard") },
		{ FName("Combat_LockOn_Default"), FName("LockOn_Hard") },
		// 锁定状态映射
		{ FName("LockOn"), FName("LockOn_Hard") },
		{ FName("LockOn_Default"), FName("LockOn_Hard") },
		// 空闲状态映射
		{ FName("Idle"), FName("Idle_Default") },
	};

	// 检查是否需要映射状态名
	if (const FName* MappedName = StateNameMapping.Find(NewStateName))
	{
#if WITH_EDITOR
		// 只在启用状态机调试时输出 / Only log when state machine debug is enabled
		USoulsCameraManager* CameraManager = Cast<USoulsCameraManager>(GetOuter());
		if (CameraManager && CameraManager->IsStateMachineDebugEnabled())
		{
			UE_LOG(LogTemp, Log, TEXT("RequestStateChange - Mapping '%s' -> '%s'"),
				*NewStateName.ToString(), *MappedName->ToString());
		}
#endif
		NewStateName = *MappedName;
	}

	// === 原有代码从这里开始 ===
	if (!IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraStateMachine::RequestStateChange - Not initialized"));
		return false;
	}

	// Check if state exists
	if (!DoesStateExist(NewStateName))
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraStateMachine::RequestStateChange - State '%s' does not exist"),
			*NewStateName.ToString());
		return false;
	}

	// Skip if already in this state and not in transition
	if (CurrentStateName == NewStateName && !IsInTransition())
	{
		return true;
	}

	// Validate transition unless forced
	if (!bForce)
	{
		FCameraInputContext DummyContext;
		if (!ValidateTransition(CurrentStateName, NewStateName, DummyContext))
		{
			UE_LOG(LogTemp, Verbose, TEXT("USoulsCameraStateMachine::RequestStateChange - Transition from '%s' to '%s' not allowed"),
				*CurrentStateName.ToString(), *NewStateName.ToString());
			return false;
		}
	}

	// Begin the transition
	BeginTransition(NewStateName);
	return true;
}

FName USoulsCameraStateMachine::GetCurrentStateName() const
{
	return CurrentStateName;
}

FName USoulsCameraStateMachine::GetPreviousStateName() const
{
	return PreviousStateName;
}

bool USoulsCameraStateMachine::GetCurrentStateConfig(FCameraStateConfig& OutConfig) const
{
	return GetStateConfig(CurrentStateName, OutConfig);
}

bool USoulsCameraStateMachine::GetStateConfig(FName StateName, FCameraStateConfig& OutConfig) const
{
	// 新架构：使用ConfigService / New architecture: use ConfigService
	if (bUseConfigService && ConfigService)
	{
		return GetStateConfigFromService(StateName, OutConfig);
	}

	// 旧架构：使用缓存配置 / Legacy architecture: use cached configs
	if (const FCameraStateConfig* Config = CachedConfigs.Find(StateName))
	{
		OutConfig = *Config;
		return true;
	}
	return false;
}

bool USoulsCameraStateMachine::DoesStateExist(FName StateName) const
{
	// 新架构：使用ConfigService / New architecture: use ConfigService
	if (bUseConfigService && ConfigService)
	{
		return DoesStateExistInService(StateName);
	}

	// 旧架构：使用缓存配置 / Legacy architecture: use cached configs
	return CachedConfigs.Contains(StateName);
}

//========================================
// State Query
//========================================

TArray<FName> USoulsCameraStateMachine::GetStatesInCategory(ECameraStateCategory Category) const
{
	if (const TArray<FName>* States = CategoryToStates.Find(Category))
	{
		return *States;
	}
	return TArray<FName>();
}

ECameraStateCategory USoulsCameraStateMachine::GetCurrentCategory() const
{
	FCameraStateConfig Config;
	if (GetCurrentStateConfig(Config))
	{
		return Config.GetCategory();
	}
	return ECameraStateCategory::None;
}

bool USoulsCameraStateMachine::IsInCategory(ECameraStateCategory Category) const
{
	return GetCurrentCategory() == Category;
}

//========================================
// Transition
//========================================

void USoulsCameraStateMachine::UpdateTransition(float DeltaTime)
{
	if (!IsInTransition())
	{
		return;
	}

	TransitionTimeRemaining -= DeltaTime;

	if (TransitionTimeRemaining <= 0.0f)
	{
		CompleteTransition();
	}
	else
	{
		// Calculate linear alpha (0 at start, 1 at end)
		float LinearAlpha = 1.0f - (TransitionTimeRemaining / TransitionTotalTime);
		LinearAlpha = FMath::Clamp(LinearAlpha, 0.0f, 1.0f);

		// Apply blend curve
		BlendAlpha = CalculateBlendAlpha(LinearAlpha, CurrentBlendType);
	}
}

float USoulsCameraStateMachine::GetBlendAlpha() const
{
	return BlendAlpha;
}

bool USoulsCameraStateMachine::IsInTransition() const
{
	return TransitionTimeRemaining > 0.0f;
}

bool USoulsCameraStateMachine::GetBlendedConfig(FCameraStateConfig& OutConfig) const
{
	if (!IsInitialized())
	{
		return false;
	}

	// If not in transition or no previous state, return current config
	if (!IsInTransition() || PreviousStateName.IsNone())
	{
		return GetCurrentStateConfig(OutConfig);
	}

	// Get both configs
	FCameraStateConfig PrevConfig, CurrConfig;
	if (!GetStateConfig(PreviousStateName, PrevConfig) || !GetStateConfig(CurrentStateName, CurrConfig))
	{
		return GetCurrentStateConfig(OutConfig);
	}

	// Blend the configs
	const float Alpha = BlendAlpha;

	// Create blended config
	OutConfig = CurrConfig; // Start with current as base

	// Blend distance parameters
	OutConfig.StateBase.Distance.BaseDistance = FMath::Lerp(
		PrevConfig.StateBase.Distance.BaseDistance,
		CurrConfig.StateBase.Distance.BaseDistance,
		Alpha);
	OutConfig.StateBase.Distance.MinDistance = FMath::Lerp(
		PrevConfig.StateBase.Distance.MinDistance,
		CurrConfig.StateBase.Distance.MinDistance,
		Alpha);
	OutConfig.StateBase.Distance.MaxDistance = FMath::Lerp(
		PrevConfig.StateBase.Distance.MaxDistance,
		CurrConfig.StateBase.Distance.MaxDistance,
		Alpha);

	// Blend FOV parameters
	OutConfig.StateBase.FOV.BaseFOV = FMath::Lerp(
		PrevConfig.StateBase.FOV.BaseFOV,
		CurrConfig.StateBase.FOV.BaseFOV,
		Alpha);
	OutConfig.StateBase.FOV.MinFOV = FMath::Lerp(
		PrevConfig.StateBase.FOV.MinFOV,
		CurrConfig.StateBase.FOV.MinFOV,
		Alpha);
	OutConfig.StateBase.FOV.MaxFOV = FMath::Lerp(
		PrevConfig.StateBase.FOV.MaxFOV,
		CurrConfig.StateBase.FOV.MaxFOV,
		Alpha);

	// Blend rotation parameters
	OutConfig.StateBase.Rotation.MinPitch = FMath::Lerp(
		PrevConfig.StateBase.Rotation.MinPitch,
		CurrConfig.StateBase.Rotation.MinPitch,
		Alpha);
	OutConfig.StateBase.Rotation.MaxPitch = FMath::Lerp(
		PrevConfig.StateBase.Rotation.MaxPitch,
		CurrConfig.StateBase.Rotation.MaxPitch,
		Alpha);

	// Blend offset parameters
	OutConfig.StateBase.Offset.FocusOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.FocusOffset,
		CurrConfig.StateBase.Offset.FocusOffset,
		Alpha);
	OutConfig.StateBase.Offset.SocketOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.SocketOffset,
		CurrConfig.StateBase.Offset.SocketOffset,
		Alpha);
	OutConfig.StateBase.Offset.TargetOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.TargetOffset,
		CurrConfig.StateBase.Offset.TargetOffset,
		Alpha);
	OutConfig.StateBase.Offset.VerticalOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.VerticalOffset,
		CurrConfig.StateBase.Offset.VerticalOffset,
		Alpha);
	OutConfig.StateBase.Offset.ShoulderOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.ShoulderOffset,
		CurrConfig.StateBase.Offset.ShoulderOffset,
		Alpha);
	OutConfig.StateBase.Offset.ForwardOffset = FMath::Lerp(
		PrevConfig.StateBase.Offset.ForwardOffset,
		CurrConfig.StateBase.Offset.ForwardOffset,
		Alpha);

	// Blend lag parameters
	OutConfig.StateBase.Lag.PositionLagSpeed = FMath::Lerp(
		PrevConfig.StateBase.Lag.PositionLagSpeed,
		CurrConfig.StateBase.Lag.PositionLagSpeed,
		Alpha);
	OutConfig.StateBase.Lag.RotationLagSpeed = FMath::Lerp(
		PrevConfig.StateBase.Lag.RotationLagSpeed,
		CurrConfig.StateBase.Lag.RotationLagSpeed,
		Alpha);
	OutConfig.StateBase.Lag.DistanceLagSpeed = FMath::Lerp(
		PrevConfig.StateBase.Lag.DistanceLagSpeed,
		CurrConfig.StateBase.Lag.DistanceLagSpeed,
		Alpha);
	OutConfig.StateBase.Lag.FOVLagSpeed = FMath::Lerp(
		PrevConfig.StateBase.Lag.FOVLagSpeed,
		CurrConfig.StateBase.Lag.FOVLagSpeed,
		Alpha);

	// Blend collision parameters
	OutConfig.StateBase.Collision.CollisionRadius = FMath::Lerp(
		PrevConfig.StateBase.Collision.CollisionRadius,
		CurrConfig.StateBase.Collision.CollisionRadius,
		Alpha);
	OutConfig.StateBase.Collision.RecoveryDelay = FMath::Lerp(
		PrevConfig.StateBase.Collision.RecoveryDelay,
		CurrConfig.StateBase.Collision.RecoveryDelay,
		Alpha);

	// Blend auto-correct parameters
	OutConfig.StateBase.AutoCorrect.AutoCenterDelay = FMath::Lerp(
		PrevConfig.StateBase.AutoCorrect.AutoCenterDelay,
		CurrConfig.StateBase.AutoCorrect.AutoCenterDelay,
		Alpha);

	return true;
}

//========================================
// Evaluation
//========================================

void USoulsCameraStateMachine::EvaluateState(const FCameraInputContext& Context)
{
	if (!IsInitialized())
	{
		return;
	}

	// Get current config for evaluation
	FCameraStateConfig CurrentConfig;
	if (!GetCurrentStateConfig(CurrentConfig))
	{
		return;
	}

	// Basic state evaluation logic
	// This is a simplified implementation - full evaluation would check:
	// - Combat state changes (lock-on, no target, etc.)
	// - Environment changes (tight spaces, cliffs, etc.)
	// - Character state changes (sprinting, crouching, etc.)

	FName ProposedState = CurrentStateName;

	// Combat state evaluation
	if (Context.bIsInCombat && Context.bHasTarget)
	{
		// Check if we should switch to lock-on state
		if (!CurrentConfig.RequiresTarget())
		{
			// Try to find an appropriate combat lock-on state
			static const FName LockOnStateName = TEXT("LockOn_Hard");
			if (DoesStateExist(LockOnStateName))
			{
				ProposedState = LockOnStateName;
			}
		}
	}
	else if (!Context.bIsInCombat && CurrentConfig.RequiresTarget())
	{
		// Lost target or exited combat, return to exploration
		static const FName ExplorationStateName = TEXT("Explore_Default");
		if (DoesStateExist(ExplorationStateName))
		{
			ProposedState = ExplorationStateName;
		}
	}

	// Request state change if different
	if (ProposedState != CurrentStateName)
	{
		RequestStateChange(ProposedState, false);
	}
}

//========================================
// Internal Methods
//========================================

void USoulsCameraStateMachine::CacheStateConfigs()
{
	CachedConfigs.Empty();
	CategoryToStates.Empty();

	if (!StatesDataTable)
	{
		return;
	}

	// Get all row names
	TArray<FName> RowNames = StatesDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FCameraStateConfig* Config = StatesDataTable->FindRow<FCameraStateConfig>(RowName, TEXT("CacheStateConfigs"));
		if (Config)
		{
			// Use the row name as the state name if StateName is not set
			FName StateName = Config->GetStateName();
			if (StateName.IsNone())
			{
				StateName = RowName;
				Config->StateBase.Identity.StateName = StateName;
			}

			// Cache the config
			CachedConfigs.Add(StateName, *Config);

			// Add to category mapping
			ECameraStateCategory Category = Config->GetCategory();
			if (!CategoryToStates.Contains(Category))
			{
				CategoryToStates.Add(Category, TArray<FName>());
			}
			CategoryToStates[Category].Add(StateName);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("USoulsCameraStateMachine::CacheStateConfigs - Cached %d states in %d categories"),
		CachedConfigs.Num(), CategoryToStates.Num());
}

float USoulsCameraStateMachine::CalculateBlendAlpha(float LinearAlpha, ECameraBlendType BlendType) const
{
	return ApplyBlendCurve(LinearAlpha, BlendType);
}

float USoulsCameraStateMachine::ApplyBlendCurve(float Alpha, ECameraBlendType BlendType)
{
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	switch (BlendType)
	{
	case ECameraBlendType::Linear:
		return Alpha;

	case ECameraBlendType::SmoothStep:
		// Hermite interpolation: 3t^2 - 2t^3
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);

	case ECameraBlendType::EaseIn:
		// Quadratic ease in: t^2
		return Alpha * Alpha;

	case ECameraBlendType::EaseOut:
		// Quadratic ease out: 1 - (1-t)^2
		return 1.0f - FMath::Square(1.0f - Alpha);

	case ECameraBlendType::EaseInOut:
		// Smoother step: 6t^5 - 15t^4 + 10t^3
		return Alpha * Alpha * Alpha * (Alpha * (Alpha * 6.0f - 15.0f) + 10.0f);

	case ECameraBlendType::Exponential:
		// Exponential ease out
		return Alpha == 1.0f ? 1.0f : 1.0f - FMath::Pow(2.0f, -10.0f * Alpha);

	case ECameraBlendType::Circular:
		// Circular ease out: sqrt(1 - (1-t)^2)
		return FMath::Sqrt(1.0f - FMath::Square(1.0f - Alpha));

	case ECameraBlendType::Cubic:
		// Cubic ease in-out
		return Alpha < 0.5f
			? 4.0f * Alpha * Alpha * Alpha
			: 1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 3.0f) / 2.0f;

	case ECameraBlendType::Instant:
		// Instant transition
		return Alpha > 0.0f ? 1.0f : 0.0f;

	default:
		return Alpha;
	}
}

bool USoulsCameraStateMachine::ValidateTransition(FName FromState, FName ToState, const FCameraInputContext& Context) const
{
	// Get both state configs
	FCameraStateConfig FromConfig, ToConfig;
	if (!GetStateConfig(FromState, FromConfig) || !GetStateConfig(ToState, ToConfig))
	{
		return false;
	}

	// Check if target state requires a target but context has none
	if (ToConfig.RequiresTarget() && !Context.HasValidTarget())
	{
		return false;
	}

	// Check if target state is cinematic and current state doesn't allow interruption
	if (ToConfig.IsCinematic() && FromConfig.IsCinematic())
	{
		// Don't interrupt cinematics with other cinematics
		return false;
	}

	// Priority-based validation: allow if target has higher or equal priority
	if (ToConfig.GetPriority() < FromConfig.GetPriority())
	{
		// Lower priority states can't interrupt higher priority states
		// (Remember: higher number = higher priority in this system)
		return false;
	}

	return true;
}

void USoulsCameraStateMachine::BeginTransition(FName NewStateName)
{
	// Get the target state config for transition timing
	FCameraStateConfig NewConfig;
	if (!GetStateConfig(NewStateName, NewConfig))
	{
		return;
	}

	// Store previous state
	PreviousStateName = CurrentStateName;
	FName OldStateName = CurrentStateName;

	// Set new current state
	CurrentStateName = NewStateName;

	// Set up transition timing
	TransitionTotalTime = NewConfig.StateBase.Transition.BlendInTime;
	TransitionTimeRemaining = TransitionTotalTime;
	CurrentBlendType = NewConfig.StateBase.Transition.BlendType;

	// Initialize blend alpha
	if (TransitionTotalTime <= 0.0f)
	{
		// Instant transition
		BlendAlpha = 1.0f;
		TransitionTimeRemaining = 0.0f;
		CompleteTransition();
	}
	else
	{
		BlendAlpha = 0.0f;
	}

	// Broadcast state change event
	OnStateChanged.Broadcast(OldStateName, NewStateName);

	// ★ 从 Manager 读取 Debug 开关
	USoulsCameraManager* CameraManager = Cast<USoulsCameraManager>(GetOuter());
	if (CameraManager && CameraManager->IsStateMachineDebugEnabled())
	{
		UE_LOG(LogTemp, Log, TEXT("USoulsCameraStateMachine::BeginTransition - Transitioning from '%s' to '%s' over %.2f seconds"),
			*OldStateName.ToString(), *NewStateName.ToString(), TransitionTotalTime);
	}
}

void USoulsCameraStateMachine::CompleteTransition()
{
	BlendAlpha = 1.0f;
	TransitionTimeRemaining = 0.0f;
	TransitionTotalTime = 0.0f;

	// Broadcast transition complete event
	OnTransitionComplete.Broadcast(CurrentStateName);

	UE_LOG(LogTemp, Verbose, TEXT("USoulsCameraStateMachine::CompleteTransition - Transition to '%s' complete"),
		*CurrentStateName.ToString());
}

//========================================
// ConfigService Support Methods
//========================================

bool USoulsCameraStateMachine::GetStateConfigFromService(FName StateName, FCameraStateConfig& OutConfig) const
{
	if (!bUseConfigService || !ConfigService)
	{
		return false;
	}

	return ConfigService->GetStateConfig(StateName, OutConfig);
}

bool USoulsCameraStateMachine::DoesStateExistInService(FName StateName) const
{
	if (!bUseConfigService || !ConfigService)
	{
		return false;
	}

	return ConfigService->DoesStateExist(StateName);
}
