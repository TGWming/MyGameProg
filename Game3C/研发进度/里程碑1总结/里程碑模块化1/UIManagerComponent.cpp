#include "UIManagerComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/StructOnScope.h"
#include "UObject/UObjectIterator.h"

// Sets default values for this component's properties
UUIManagerComponent::UUIManagerComponent()
{
	// Set this component to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// ==================== Initialize Default Values ====================
	
	// UI Configuration
	LockOnWidgetClass = nullptr;
	CurrentUIDisplayMode = EUIDisplayMode::SocketProjection;
	bEnableUIDebugLogs = false;
	bEnableSizeAnalysisDebugLogs = false;

	// State Management
	LockOnWidgetInstance = nullptr;
	PreviousLockOnTarget = nullptr;
	CurrentLockOnTarget = nullptr;
	CurrentUIScale = 1.0f;
	CurrentUIColor = FLinearColor::White;

	// Initialize socket projection settings with defaults from LockOnConfig
	SocketProjectionSettings = FSocketProjectionSettings();
	AdvancedCameraSettings = FAdvancedCameraSettings();

	// Initialize arrays
	TargetsWithActiveWidgets.Empty();
	WidgetComponentCache.Empty();
	EnemySizeCache.Empty();

	// Component references
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

// Called when the game starts
void UUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the UI manager
	InitializeUIManager();
}

// Called every frame
void UUIManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update owner references if needed
	UpdateOwnerReferences();

	// Update socket projection widget if in socket projection mode
	if (CurrentUIDisplayMode == EUIDisplayMode::SocketProjection && CurrentLockOnTarget)
	{
		UpdateSocketProjectionWidget(CurrentLockOnTarget);
	}
}

// ==================== Core Public Interface ====================

void UUIManagerComponent::ShowLockOnWidget(AActor* Target)
{
	if (!IsValidTargetForUI(Target))
	{
		// 高频日志优化 - 添加降频控制
		if (bEnableUIDebugLogs)
		{
			static int32 FrameCounter = 0;
			if (++FrameCounter % 300 == 0) // 每5秒只记录一次
			{
				UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ShowLockOnWidget - Invalid target: %s"), 
					Target ? *Target->GetName() : TEXT("None"));
			}
		}
		return;
	}

	// Hide all other widgets first
	HideAllLockOnWidgets();

	// Set current target
	CurrentLockOnTarget = Target;

	// Choose display method based on current mode
	switch (CurrentUIDisplayMode)
	{
		case EUIDisplayMode::SocketProjection:
			ShowSocketProjectionWidget(Target);
			break;
			
		case EUIDisplayMode::Traditional3D:
			// Traditional 3D space UI display
			{
				UActorComponent* WidgetComp = Target->GetComponentByClass(UWidgetComponent::StaticClass());
				if (WidgetComp)
				{
					UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
					if (WidgetComponent)
					{
						WidgetComponent->SetVisibility(true);
						TargetsWithActiveWidgets.AddUnique(Target);
					}
				}
			}
			break;

		case EUIDisplayMode::ScreenSpace:
			// Screen space overlay UI (复制MyCharacter的逻辑)
			{
				// Try to find widget class at runtime if not set
				if (!LockOnWidgetClass)
				{
					TryFindWidgetClassAtRuntime();
				}

				if (!LockOnWidgetClass)
				{
					if (bEnableUIDebugLogs)
					{
						UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::ShowLockOnWidget - No LockOnWidgetClass available for ScreenSpace mode"));
					}
					return;
				}

				UpdateOwnerReferences();
				if (!OwnerController)
				{
					if (bEnableUIDebugLogs)
					{
						UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::ShowLockOnWidget - No PlayerController available"));
					}
					return;
				}

				if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
				{
					LockOnWidgetInstance->RemoveFromViewport();
				}

				LockOnWidgetInstance = CreateWidget<UUserWidget>(OwnerController, LockOnWidgetClass);
				if (LockOnWidgetInstance)
				{
					LockOnWidgetInstance->AddToViewport();
					PreviousLockOnTarget = CurrentLockOnTarget;
					
					if (bEnableUIDebugLogs)
					{
						UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Created and added ScreenSpace widget to viewport"));
					}
				}
			}
			break;

		case EUIDisplayMode::SizeAdaptive:
			// Size adaptive UI mode - analyze target size and adapt UI accordingly
			{
				// Get or calculate target size category
				EEnemySizeCategory SizeCategory = EEnemySizeCategory::Unknown;
				
				if (EEnemySizeCategory* CachedSize = EnemySizeCache.Find(Target))
				{
					SizeCategory = *CachedSize;
				}
				else
				{
					SizeCategory = AnalyzeTargetSize(Target);
					EnemySizeCache.Add(Target, SizeCategory);
				}

				// Show size-adaptive widget
				ShowSizeAdaptiveWidget(Target, SizeCategory);
				return; // ShowSizeAdaptiveWidget handles the rest
			}
			break;
			
		default:
			if (bEnableUIDebugLogs)
			{
				UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ShowLockOnWidget - Unsupported display mode: %d"), 
					(int32)CurrentUIDisplayMode);
			}
			break;
	}

	// Broadcast event
	if (OnLockOnWidgetShown.IsBound())
	{
		OnLockOnWidgetShown.Broadcast(Target);
	}

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::ShowLockOnWidget - Target: %s, Mode: %d"), 
			*Target->GetName(), (int32)CurrentUIDisplayMode);
	}
}

void UUIManagerComponent::HideLockOnWidget()
{
	HideAllLockOnWidgets();
	
	// Hide socket projection widget
	if (CurrentUIDisplayMode == EUIDisplayMode::SocketProjection)
	{
		HideSocketProjectionWidget();
	}

	// Hide screen space widget (复制MyCharacter的逻辑)
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
	}

	LockOnWidgetInstance = nullptr;
	CurrentLockOnTarget = nullptr;
	PreviousLockOnTarget = nullptr;

	// Broadcast event
	if (OnLockOnWidgetHidden.IsBound())
	{
		OnLockOnWidgetHidden.Broadcast();
	}

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::HideLockOnWidget called"));
	}
}

void UUIManagerComponent::HideAllLockOnWidgets()
{
	// Hide all traditional 3D widgets
	for (AActor* Target : TargetsWithActiveWidgets)
	{
		if (IsValid(Target))
		{
			UActorComponent* WidgetComp = Target->GetComponentByClass(UWidgetComponent::StaticClass());
			if (WidgetComp)
			{
				UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
				if (WidgetComponent && WidgetComponent->IsVisible())
				{
					WidgetComponent->SetVisibility(false);
				}
			}
		}
	}

	// Clear the active widgets list
	TargetsWithActiveWidgets.Empty();

	// Hide socket projection widget
	if (CurrentUIDisplayMode == EUIDisplayMode::SocketProjection)
	{
		HideSocketProjectionWidget();
	}

	// Hide screen space widget
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
		LockOnWidgetInstance = nullptr;
	}

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::HideAllLockOnWidgets called"));
	}
}

void UUIManagerComponent::UpdateLockOnWidget(AActor* CurrentTarget, AActor* PreviousTarget)
{
	// Update internal state
	PreviousLockOnTarget = PreviousTarget;
	CurrentLockOnTarget = CurrentTarget;

	// 复制MyCharacter.cpp的完整逻辑
	// Check if not locked on - ensure all UI is hidden
	if (!CurrentTarget)
	{
		HideAllLockOnWidgets();
		
		if (CurrentUIDisplayMode == EUIDisplayMode::SocketProjection)
		{
			HideSocketProjectionWidget();
		}
		
		if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
		{
			LockOnWidgetInstance->RemoveFromViewport();
			LockOnWidgetInstance = nullptr;
		}
		return;
	}

	// Check if target changed
	bool bTargetChanged = (CurrentTarget != PreviousTarget);
	
	if (bTargetChanged && IsValid(PreviousTarget))
	{
		// Hide previous target's UI
		if (CurrentUIDisplayMode == EUIDisplayMode::SocketProjection)
		{
			HideSocketProjectionWidget();
		}
		else
		{
			// Hide traditional 3D widget of previous target
			UActorComponent* PrevWidgetComp = PreviousTarget->GetComponentByClass(UWidgetComponent::StaticClass());
			if (PrevWidgetComp)
			{
				UWidgetComponent* PrevWidgetComponent = Cast<UWidgetComponent>(PrevWidgetComp);
				if (PrevWidgetComponent && PrevWidgetComponent->IsVisible())
				{
					PrevWidgetComponent->SetVisibility(false);
				}
			}
		}
	}

	// Show current target's widget
	if (IsValid(CurrentTarget))
	{
		// Handle size adaptive mode specifically
		if (CurrentUIDisplayMode == EUIDisplayMode::SizeAdaptive)
		{
			EEnemySizeCategory SizeCategory = EEnemySizeCategory::Unknown;
			
			if (EEnemySizeCategory* CachedSize = EnemySizeCache.Find(CurrentTarget))
			{
				SizeCategory = *CachedSize;
			}
			else
			{
				SizeCategory = AnalyzeTargetSize(CurrentTarget);
				EnemySizeCache.Add(CurrentTarget, SizeCategory);
			}
			
			ShowSizeAdaptiveWidget(CurrentTarget, SizeCategory);
		}
		else
		{
			ShowLockOnWidget(CurrentTarget);
		}
	}

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::UpdateLockOnWidget - Current: %s, Previous: %s"), 
			CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"),
			PreviousTarget ? *PreviousTarget->GetName() : TEXT("None"));
	}
}

// ==================== Socket Projection Implementation ====================

FVector UUIManagerComponent::GetTargetSocketWorldLocation(AActor* Target) const
{
	if (!Target)
	{
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::GetTargetSocketWorldLocation: Target is null"));
		// }
		return FVector::ZeroVector;
	}

	// Try to get target's skeletal mesh component
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh && SkeletalMesh->DoesSocketExist(SocketProjectionSettings.TargetSocketName))
	{
		// If socket exists, return socket world location plus offset
		FVector SocketLocation = SkeletalMesh->GetSocketLocation(SocketProjectionSettings.TargetSocketName);
		FVector FinalLocation = SocketLocation + SocketProjectionSettings.SocketOffset;
		
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Socket '%s' found on target '%s' at location (%.1f, %.1f, %.1f)"), 
		//		*SocketProjectionSettings.TargetSocketName.ToString(), *Target->GetName(), 
		//		FinalLocation.X, FinalLocation.Y, FinalLocation.Z);
		// }
		
		return FinalLocation;
	}
	else
	{
		// If no socket, return Actor location plus offset
		FVector FallbackLocation = Target->GetActorLocation() + SocketProjectionSettings.SocketOffset;
		
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Socket '%s' not found on target '%s', using fallback location (%.1f, %.1f, %.1f)"), 
		//		*SocketProjectionSettings.TargetSocketName.ToString(), *Target->GetName(), 
		//		FallbackLocation.X, FallbackLocation.Y, FallbackLocation.Z);
		// }
		
		return FallbackLocation;
	}
}

bool UUIManagerComponent::HasValidSocket(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// Check if target has the specified socket
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh)
	{
		bool bSocketExists = SkeletalMesh->DoesSocketExist(SocketProjectionSettings.TargetSocketName);
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Socket check for '%s': Socket '%s' exists = %s"), 
		//		*Target->GetName(), *SocketProjectionSettings.TargetSocketName.ToString(), 
		//		bSocketExists ? TEXT("YES") : TEXT("NO"));
		// }
		return bSocketExists;
	}

	// 高频日志优化 - 原始代码保留但注释
	// if (bEnableUIDebugLogs)
	// {
	//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Target '%s' has no SkeletalMeshComponent"), *Target->GetName());
	// }
	return false;
}

FVector2D UUIManagerComponent::ProjectSocketToScreen(const FVector& SocketWorldLocation) const
{
	// Update owner references
	const_cast<UUIManagerComponent*>(this)->UpdateOwnerReferences();

	// Get player controller
	if (!OwnerController)
	{
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ProjectSocketToScreen: No PlayerController"));
		// }
		return FVector2D::ZeroVector;
	}

	// Project world location to screen coordinates
	FVector2D ScreenLocation;
	bool bProjected = OwnerController->ProjectWorldLocationToScreen(SocketWorldLocation, ScreenLocation);
	
	if (bProjected)
	{
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Projection successful: World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
		//		SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
		//		ScreenLocation.X, ScreenLocation.Y);
		// }
		return ScreenLocation;
	}
	else
	{
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableUIDebugLogs)
		// {
		//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Projection failed for world location (%.1f, %.1f, %.1f)"), 
		//		SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z);
		// }
		return FVector2D::ZeroVector;
	}
}

void UUIManagerComponent::ShowSocketProjectionWidget(AActor* Target)
{
	if (!Target)
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::ShowSocketProjectionWidget: No target specified"));
		}
		return;
	}

	// Update owner references
	UpdateOwnerReferences();

	// If LockOnWidgetClass is not set, try to find it at runtime (复制MyCharacter的逻辑)
	if (!LockOnWidgetClass)
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ShowSocketProjectionWidget: LockOnWidgetClass is not set! Attempting runtime search..."));
		}
		
		if (!TryFindWidgetClassAtRuntime())
		{
			if (bEnableUIDebugLogs)
			{
				UE_LOG(LogTemp, Error, TEXT("UIManagerComponent: Could not find any Widget class at runtime!"));
				UE_LOG(LogTemp, Error, TEXT("Please ensure:"));
				UE_LOG(LogTemp, Error, TEXT("1. One of these Widgets is compiled and valid"));
				UE_LOG(LogTemp, Error, TEXT("2. LockOnWidgetClass is set in Blueprint to one of these Widgets"));
				UE_LOG(LogTemp, Error, TEXT("3. Widget contains UpdateLockOnPostition event"));
			}
			return;
		}
	}

	if (!OwnerController)
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::ShowSocketProjectionWidget: No valid PlayerController"));
		}
		return;
	}

	// If already have instance and in viewport, remove it first (复制MyCharacter的逻辑)
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Removing existing UMG widget instance"));
		}
		LockOnWidgetInstance->RemoveFromViewport();
	}

	// Create new Widget instance (复制MyCharacter的逻辑)
	LockOnWidgetInstance = CreateWidget<UUserWidget>(OwnerController, LockOnWidgetClass);
	if (LockOnWidgetInstance)
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Successfully created UMG widget instance: %s"), 
				*LockOnWidgetInstance->GetClass()->GetName());
		}
		
		LockOnWidgetInstance->AddToViewport();
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: UMG widget added to viewport"));
		}
		
		// Immediately update position
		UpdateSocketProjectionWidget(Target);
		
		PreviousLockOnTarget = CurrentLockOnTarget;
		CurrentLockOnTarget = Target;
		
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Socket projection widget created and shown for target: %s"), 
				*Target->GetName());
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Using Socket: %s"), 
				*SocketProjectionSettings.TargetSocketName.ToString());
		}
	}
	else
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManagerComponent: Failed to create UMG widget instance! LockOnWidgetClass: %s"), 
				LockOnWidgetClass ? *LockOnWidgetClass->GetName() : TEXT("NULL"));
		}
	}
}

void UUIManagerComponent::UpdateSocketProjectionWidget(AActor* Target)
{
	if (!Target || !LockOnWidgetInstance || !LockOnWidgetInstance->IsInViewport())
		return;

	// Get target Socket world location
	FVector SocketWorldLocation = GetTargetSocketWorldLocation(Target);
	
	// Project to screen coordinates
	FVector2D ScreenPosition = ProjectSocketToScreen(SocketWorldLocation);
	
	// Check if projection is successful (screen coordinates are valid)
	if (ScreenPosition != FVector2D::ZeroVector)
	{
		// Try to call your UMG event function UpdateLockOnPostition (keeping original spelling)
		UFunction* UpdateFunction = LockOnWidgetInstance->GetClass()->FindFunctionByName(FName(TEXT("UpdateLockOnPostition")));
		
		if (UpdateFunction)
		{
			// Check function parameter count
			if (UpdateFunction->NumParms >= 1)
			{
				// Create parameter structure
				uint8* Params = static_cast<uint8*>(FMemory_Alloca(UpdateFunction->ParmsSize));
				FMemory::Memzero(Params, UpdateFunction->ParmsSize);
				
				// Find FVector2D parameter
				bool bFoundParam = false;
				for (TFieldIterator<FProperty> It(UpdateFunction); It; ++It)
				{
					FProperty* Property = *It;
					if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm))
					{
						// Check if it's FVector2D type
						if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
						{
							if (StructProp->Struct && StructProp->Struct->GetFName() == FName("Vector2D"))
							{
								// Set parameter value
								FVector2D* ParamPtr = reinterpret_cast<FVector2D*>(Params + Property->GetOffset_ForInternal());
								*ParamPtr = ScreenPosition;
								bFoundParam = true;
								// 高频日志优化 - 原始代码保留但注释
								// if (bEnableUIDebugLogs)
								// {
								//	UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Setting UMG position parameter: (%.1f, %.1f) using function UpdateLockOnPostition"), 
								//		ScreenPosition.X, ScreenPosition.Y);
								// }
								break;
							}
						}
					}
				}
				
				if (bFoundParam)
				{
					// Call function
					LockOnWidgetInstance->ProcessEvent(UpdateFunction, Params);
					// 高频日志优化 - 原始代码保留但注释
					// if (bEnableUIDebugLogs)
					// {
					//	UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Successfully called UMG update function UpdateLockOnPostition with position: (%.1f, %.1f)"), 
					//		ScreenPosition.X, ScreenPosition.Y);
					// }
				}
				else
				{
					if (bEnableUIDebugLogs)
					{
						UE_LOG(LogTemp, Error, TEXT("UIManagerComponent: Could not find Vector2D parameter in UMG function UpdateLockOnPostition"));
					}
				}
			}
			else
			{
				if (bEnableUIDebugLogs)
				{
					UE_LOG(LogTemp, Error, TEXT("UIManagerComponent: UMG function UpdateLockOnPostition has no parameters"));
				}
			}
		}
		else
		{
			if (bEnableUIDebugLogs)
			{
				UE_LOG(LogTemp, Error, TEXT("UIManagerComponent: Could not find UMG update function UpdateLockOnPostition in widget class"));
			}
		}
		
		// Ensure Widget is visible
		if (!LockOnWidgetInstance->IsVisible())
		{
			LockOnWidgetInstance->SetVisibility(ESlateVisibility::Visible);
			// 高频日志优化 - 原始代码保留但注释
			// if (bEnableUIDebugLogs)
			// {
			//	UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Set UMG widget visibility to Visible"));
			// }
		}
		
		// Broadcast event
		if (OnSocketProjectionUpdated.IsBound())
		{
			OnSocketProjectionUpdated.Broadcast(Target, ScreenPosition);
		}
		
		// Debug log - including Socket information - 高频日志优化
		if (bEnableUIDebugLogs)
		{
			static float LastDetailedLogTime = 0.0f;
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - LastDetailedLogTime > 3.0f) // 每3秒记录一次详细信息
			{
				// 高频日志优化 - 原始代码保留但注释
				// UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Socket projection updated: Socket(%s) World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
				//	*SocketProjectionSettings.TargetSocketName.ToString(),
				//	SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
				//	ScreenPosition.X, ScreenPosition.Y);
				LastDetailedLogTime = CurrentTime;
			}
		}
	}
	else
	{
		// Target is off-screen, hide Widget
		if (LockOnWidgetInstance->IsVisible())
		{
			LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			// 高频日志优化 - 添加降频控制
			if (bEnableUIDebugLogs)
			{
				static int32 OffScreenFrameCounter = 0;
				if (++OffScreenFrameCounter % 120 == 0) // 每2秒只记录一次
				{
					UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Target off-screen, hiding UMG widget"));
				}
			}
		}
	}
}

void UUIManagerComponent::HideSocketProjectionWidget()
{
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Socket projection widget hidden"));
		}
	}
	
	LockOnWidgetInstance = nullptr;
}

// ==================== Configuration Accessors ====================

void UUIManagerComponent::SetUIDisplayMode(EUIDisplayMode NewMode)
{
	if (CurrentUIDisplayMode != NewMode)
	{
		// Hide current widgets before switching
		HideAllLockOnWidgets();
		
		CurrentUIDisplayMode = NewMode;
		
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::SetUIDisplayMode - New mode: %d"), (int32)NewMode);
		}
	}
}

void UUIManagerComponent::SetSocketProjectionSettings(const FSocketProjectionSettings& NewSettings)
{
	SocketProjectionSettings = NewSettings;
	
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::SetSocketProjectionSettings - Socket: %s"), 
			*NewSettings.TargetSocketName.ToString());
	}
}

// ==================== Internal Helper Functions ====================

void UUIManagerComponent::InitializeUIManager()
{
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::InitializeUIManager - Starting initialization"));
	}

	// Clear any existing state
	CleanupUIResources();

	// Update owner references
	UpdateOwnerReferences();

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::InitializeUIManager - Initialization complete"));
	}
}

void UUIManagerComponent::CleanupUIResources()
{
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::CleanupUIResources - Cleaning up resources"));
	}

	// Hide all widgets
	HideAllLockOnWidgets();

	// Clear state
	LockOnWidgetInstance = nullptr;
	PreviousLockOnTarget = nullptr;
	CurrentLockOnTarget = nullptr;
	TargetsWithActiveWidgets.Empty();
	ClearWidgetComponentCache();
	
	// Clean up size cache
	CleanupSizeCache();

	// Reset UI properties
	CurrentUIScale = 1.0f;
	CurrentUIColor = FLinearColor::White;
}

APlayerController* UUIManagerComponent::GetPlayerController() const
{
	if (OwnerController)
	{
		return OwnerController;
	}

	if (AActor* Owner = GetOwner())
	{
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			return Cast<APlayerController>(OwnerPawn->GetController());
		}
	}
	return nullptr;
}

bool UUIManagerComponent::IsValidTargetForUI(AActor* Target) const
{
	// Basic validation - will be expanded if needed
	return IsValid(Target);
}

void UUIManagerComponent::ClearWidgetComponentCache()
{
	WidgetComponentCache.Empty();
	
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::ClearWidgetComponentCache - Cache cleared"));
	}
}

void UUIManagerComponent::UpdateOwnerReferences()
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}

	if (!OwnerController && OwnerCharacter)
	{
		OwnerController = Cast<APlayerController>(OwnerCharacter->GetController());
	}
}

// ==================== Size Adaptive UI Implementation ====================

void UUIManagerComponent::ShowSizeAdaptiveWidget(AActor* Target, EEnemySizeCategory SizeCategory)
{
	if (!IsValidTargetForUI(Target))
	{
		if (bEnableSizeAnalysisDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ShowSizeAdaptiveWidget - Invalid target: %s"), 
				Target ? *Target->GetName() : TEXT("None"));
		}
		return;
	}

	// Cache the size category
	EnemySizeCache.FindOrAdd(Target) = SizeCategory;

	// Update UI scale and color based on size
	CurrentUIScale = GetUIScaleForEnemySize(SizeCategory);
	CurrentUIColor = GetUIColorForEnemySize(SizeCategory);

	if (bEnableSizeAdaptiveUI)
	{
		if (bEnableSizeAnalysisDebugLogs)
		{
			FString SizeCategoryString = UEnum::GetValueAsString(SizeCategory);
			UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::ShowSizeAdaptiveWidget - Target: %s, Size: %s, Scale: %.2f"), 
				*Target->GetName(), *SizeCategoryString, CurrentUIScale);
		}
	}

	// Show the widget with size adaptations
	ShowLockOnWidget(Target);

	// Apply size-specific adaptations if widget was created successfully
	if (LockOnWidgetInstance)
	{
		UpdateWidgetForEnemySize(Target, SizeCategory);
	}
}

void UUIManagerComponent::UpdateWidgetForEnemySize(AActor* Target, EEnemySizeCategory SizeCategory)
{
	if (!Target || !LockOnWidgetInstance)
	{
		return;
	}

	if (!bEnableSizeAdaptiveUI)
	{
		if (bEnableSizeAnalysisDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::UpdateWidgetForEnemySize - Size adaptive UI is disabled"));
		}
		return;
	}

	// Update UI properties based on size
	float TargetScale = GetUIScaleForEnemySize(SizeCategory);
	FLinearColor TargetColor = GetUIColorForEnemySize(SizeCategory);

	// Try to find and call size adaptation function on the widget
	UFunction* SetScaleFunction = LockOnWidgetInstance->GetClass()->FindFunctionByName(FName(TEXT("SetWidgetScale")));
	if (SetScaleFunction)
	{
		// Prepare parameters for SetWidgetScale function
		uint8* Params = static_cast<uint8*>(FMemory_Alloca(SetScaleFunction->ParmsSize));
		FMemory::Memzero(Params, SetScaleFunction->ParmsSize);

		// Find float parameter for scale
		for (TFieldIterator<FProperty> It(SetScaleFunction); It; ++It)
		{
			FProperty* Property = *It;
			if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
				{
					float* ParamPtr = reinterpret_cast<float*>(Params + Property->GetOffset_ForInternal());
					*ParamPtr = TargetScale;
					break;
				}
			}
		}

		LockOnWidgetInstance->ProcessEvent(SetScaleFunction, Params);
		if (bEnableSizeAnalysisDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Applied scale %.2f to widget"), TargetScale);
		}
	}

	// Try to find and call color adaptation function on the widget
	UFunction* SetColorFunction = LockOnWidgetInstance->GetClass()->FindFunctionByName(FName(TEXT("SetWidgetColor")));
	if (SetColorFunction)
	{
		// Prepare parameters for SetWidgetColor function
		uint8* Params = static_cast<uint8*>(FMemory_Alloca(SetColorFunction->ParmsSize));
		FMemory::Memzero(Params, SetColorFunction->ParmsSize);

		// Find FLinearColor parameter
		for (TFieldIterator<FProperty> It(SetColorFunction); It; ++It)
		{
			FProperty* Property = *It;
			if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
				{
					if (StructProp->Struct && StructProp->Struct->GetFName() == FName("LinearColor"))
					{
						FLinearColor* ParamPtr = reinterpret_cast<FLinearColor*>(Params + Property->GetOffset_ForInternal());
						*ParamPtr = TargetColor;
						break;
					}
				}
			}
		}

		LockOnWidgetInstance->ProcessEvent(SetColorFunction, Params);
		if (bEnableSizeAnalysisDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Applied color to widget"));
		}
	}

	CurrentUIScale = TargetScale;
	CurrentUIColor = TargetColor;

	if (bEnableSizeAnalysisDebugLogs)
	{
		FString SizeCategoryString = UEnum::GetValueAsString(SizeCategory);
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::UpdateWidgetForEnemySize - Target: %s, Size: %s, Applied Scale: %.2f"), 
			*Target->GetName(), *SizeCategoryString, TargetScale);
	}
}

float UUIManagerComponent::GetUIScaleForEnemySize(EEnemySizeCategory SizeCategory) const
{
	switch (SizeCategory)
	{
		case EEnemySizeCategory::Small:
			return SmallEnemyUIScale;
		case EEnemySizeCategory::Medium:
			return MediumEnemyUIScale;
		case EEnemySizeCategory::Large:
			return LargeEnemyUIScale;
		case EEnemySizeCategory::Unknown:
		default:
			return MediumEnemyUIScale; // Default to medium size
	}
}

FLinearColor UUIManagerComponent::GetUIColorForEnemySize(EEnemySizeCategory SizeCategory) const
{
	switch (SizeCategory)
	{
		case EEnemySizeCategory::Small:
			return SmallEnemyUIColor;
		case EEnemySizeCategory::Medium:
			return MediumEnemyUIColor;
		case EEnemySizeCategory::Large:
			return LargeEnemyUIColor;
		case EEnemySizeCategory::Unknown:
		default:
			return MediumEnemyUIColor; // Default to medium color
	}
}

// ==================== Debug Implementation ====================

void UUIManagerComponent::DebugWidgetSetup()
{
	UE_LOG(LogTemp, Warning, TEXT("=== UIManagerComponent WIDGET SETUP DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass is set: %s"), LockOnWidgetClass ? TEXT("YES") : TEXT("NO"));
	
	if (LockOnWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass name: %s"), *LockOnWidgetClass->GetName());
		UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass path: %s"), *LockOnWidgetClass->GetPathName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LockOnWidgetClass is NULL! Attempting to find available Widgets..."));
		
		// Try to find Widget class at runtime
		if (TryFindWidgetClassAtRuntime())
		{
			UE_LOG(LogTemp, Warning, TEXT("Successfully found Widget class at runtime: %s"), 
				LockOnWidgetClass ? *LockOnWidgetClass->GetName() : TEXT("Still NULL"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find any Widget class at runtime!"));
			UE_LOG(LogTemp, Error, TEXT("Steps to fix:"));
			UE_LOG(LogTemp, Error, TEXT("1. Open your character Blueprint"));
			UE_LOG(LogTemp, Error, TEXT("2. Find 'Lock On Widget Class' in the UI category"));
			UE_LOG(LogTemp, Error, TEXT("3. Set it to Widget_LockOnIcon or UI_LockOnWidget"));
			UE_LOG(LogTemp, Error, TEXT("4. Ensure the Widget has 'UpdateLockOnPostition' Custom Event"));
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Current UI Display Mode: %d"), (int32)CurrentUIDisplayMode);
	UE_LOG(LogTemp, Warning, TEXT("Socket projection enabled: %s"), SocketProjectionSettings.bUseSocketProjection ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Size adaptive UI enabled: %s"), bEnableSizeAdaptiveUI ? TEXT("YES") : TEXT("NO"));
	
	// Debug size adaptation settings
	UE_LOG(LogTemp, Warning, TEXT("Size UI Scales - Small: %.2f, Medium: %.2f, Large: %.2f"), 
		SmallEnemyUIScale, MediumEnemyUIScale, LargeEnemyUIScale);
	
	// Get PlayerController
	UpdateOwnerReferences();
	UE_LOG(LogTemp, Warning, TEXT("Owner Character available: %s"), OwnerCharacter ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Player Controller available: %s"), OwnerController ? TEXT("YES") : TEXT("NO"));
	
	UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UUIManagerComponent::LogWidgetState(const FString& Context)
{
	if (!bEnableUIDebugLogs)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== UIManagerComponent Widget State [%s] ==="), *Context);
	UE_LOG(LogTemp, Log, TEXT("Current Target: %s"), 
		CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("Previous Target: %s"), 
		PreviousLockOnTarget ? *PreviousLockOnTarget->GetName() : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("Widget Instance: %s"), 
		LockOnWidgetInstance ? TEXT("Valid") : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("Widget In Viewport: %s"), 
		(LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport()) ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Log, TEXT("Active Widgets Count: %d"), TargetsWithActiveWidgets.Num());
	UE_LOG(LogTemp, Log, TEXT("Current UI Scale: %.2f"), CurrentUIScale);
	UE_LOG(LogTemp, Log, TEXT("Current UI Color: R=%.2f G=%.2f B=%.2f A=%.2f"), 
		CurrentUIColor.R, CurrentUIColor.G, CurrentUIColor.B, CurrentUIColor.A);
	UE_LOG(LogTemp, Log, TEXT("Size Cache Entries: %d"), EnemySizeCache.Num());
	UE_LOG(LogTemp, Log, TEXT("================================================"));
}

bool UUIManagerComponent::ValidateWidgetClass() const
{
	if (!LockOnWidgetClass)
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::ValidateWidgetClass - LockOnWidgetClass is not set"));
		}
		return false;
	}

	if (!LockOnWidgetClass->IsChildOf(UUserWidget::StaticClass()))
	{
		if (bEnableUIDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::ValidateWidgetClass - LockOnWidgetClass is not a UUserWidget subclass"));
		}
		return false;
	}

	return true;
}

// ==================== Configuration Accessors Implementation ====================

void UUIManagerComponent::SetAdvancedCameraSettings(const FAdvancedCameraSettings& NewSettings)
{
	AdvancedCameraSettings = NewSettings;
	
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent::SetAdvancedCameraSettings - Size adaptation enabled: %s"), 
			NewSettings.bEnableEnemySizeAdaptation ? TEXT("YES") : TEXT("NO"));
	}
}

// ==================== Size Analysis Helper Functions ====================

bool UUIManagerComponent::TryFindWidgetClassAtRuntime()
{
	if (LockOnWidgetClass)
	{
		return true; // Already have a widget class
	}

	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent::TryFindWidgetClassAtRuntime - Searching for widget classes..."));
	}
	
	// Try multiple possible widget class names and paths
	TArray<FString> PossibleWidgetPaths = {
		TEXT("/Game/Levels/Widget_LockOnIcon.Widget_LockOnIcon_C"),
		TEXT("/Game/LockOnTS/Widgets/UI_LockOnWidget.UI_LockOnWidget_C"),
		TEXT("Widget_LockOnIcon_C"),
		TEXT("UI_LockOnWidget_C"),
		TEXT("/Game/Levels/Widget_LockOnIcon"),
		TEXT("/Game/LockOnTS/Widgets/UI_LockOnWidget"),
		TEXT("/Game/UI/Widget_LockOnIcon.Widget_LockOnIcon_C"),
		TEXT("/Game/Widgets/UI_LockOnWidget.UI_LockOnWidget_C")
	};
	
	for (const FString& WidgetPath : PossibleWidgetPaths)
	{
		UClass* FoundWidgetClass = LoadObject<UClass>(nullptr, *WidgetPath);
		
		if (!FoundWidgetClass)
		{
			// Try to find by name in any package
			FoundWidgetClass = FindObject<UClass>(ANY_PACKAGE, *WidgetPath);
		}
		
		if (FoundWidgetClass && FoundWidgetClass->IsChildOf(UUserWidget::StaticClass()))
		{
			LockOnWidgetClass = FoundWidgetClass;
			if (bEnableUIDebugLogs)
			{
				UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent: Found Widget class at runtime: %s (Path: %s)"), 
					*FoundWidgetClass->GetName(), *WidgetPath);
			}
			return true;
		}
		else
		{
			// 高频日志优化 - 原始代码保留但注释
			// if (bEnableUIDebugLogs)
			// {
			//	UE_LOG(LogTemp, Verbose, TEXT("UIManagerComponent: Widget not found at path: %s"), *WidgetPath);
			// }
		}
	}
	
	if (bEnableUIDebugLogs)
	{
		UE_LOG(LogTemp, Error, TEXT("UIManagerComponent::TryFindWidgetClassAtRuntime - Could not find any Widget class!"));
	}
	return false;
}

float UUIManagerComponent::CalculateTargetBoundingBoxSize(AActor* Target) const
{
	if (!Target)
		return 0.0f;

	// Get Actor's bounding box
	FVector Origin, BoxExtent;
	Target->GetActorBounds(false, Origin, BoxExtent);

	// Calculate maximum dimension as size reference
	float MaxDimension = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z) * 2.0f; // BoxExtent is half-length

	return MaxDimension;
}

EEnemySizeCategory UUIManagerComponent::AnalyzeTargetSize(AActor* Target)
{
	if (!Target)
		return EEnemySizeCategory::Unknown;

	float BoundingBoxSize = CalculateTargetBoundingBoxSize(Target);

	// Classify based on advanced camera settings thresholds
	if (BoundingBoxSize <= AdvancedCameraSettings.SmallEnemySizeThreshold)
	{
		return EEnemySizeCategory::Small;
	}
	else if (BoundingBoxSize <= AdvancedCameraSettings.LargeEnemySizeThreshold)
	{
		return EEnemySizeCategory::Medium;
	}
	else
	{
		return EEnemySizeCategory::Large;
	}
}

void UUIManagerComponent::UpdateTargetSizeCategory(AActor* Target)
{
	if (!Target)
		return;

	EEnemySizeCategory NewCategory = AnalyzeTargetSize(Target);
	EnemySizeCache.FindOrAdd(Target) = NewCategory;

	if (bEnableSizeAnalysisDebugLogs)
	{
		FString SizeCategoryString = UEnum::GetValueAsString(NewCategory);
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Updated size category for %s: %s"), 
			*Target->GetName(), *SizeCategoryString);
	}
}

void UUIManagerComponent::CleanupSizeCache()
{
	TArray<AActor*> InvalidActors;

	// Find invalid actors
	for (auto& Pair : EnemySizeCache)
	{
		if (!IsValid(Pair.Key))
		{
			InvalidActors.Add(Pair.Key);
		}
	}

	// Remove invalid cache entries
	for (AActor* InvalidActor : InvalidActors)
	{
		EnemySizeCache.Remove(InvalidActor);
	}

	if (bEnableSizeAnalysisDebugLogs && InvalidActors.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Cleaned up %d invalid size cache entries"), InvalidActors.Num());
	}
}