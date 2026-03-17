// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManagerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"
#include "TargetDetectionComponent.h"
#include "LockOnConfigComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "SubTargetManager.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

// Sets default values for this component's properties
UUIManagerComponent::UUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("======== UIManagerComponent BeginPlay ========"));
	UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass: %s"), LockOnWidgetClass ? *LockOnWidgetClass->GetName() : TEXT("NOT SET!"));
	UE_LOG(LogTemp, Warning, TEXT("Display Mode: %d"), (int32)CurrentUIDisplayMode);
	UE_LOG(LogTemp, Warning, TEXT("Size Adaptive UI: %s"), bEnableSizeAdaptiveUI ? TEXT("Enabled") : TEXT("Disabled"));
	
	if (!LockOnWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("WARNING: LockOnWidgetClass is not set! UI will not display!"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=============================================="));
}

// Called every frame
void UUIManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentLockOnTarget && IsValid(CurrentLockOnTarget) && LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		UpdateProjectionWidget(CurrentLockOnTarget);
	}
	else if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport() && !CurrentLockOnTarget)
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		LockOnWidgetInstance->RemoveFromViewport();
		LockOnWidgetInstance = nullptr;
		CurrentLockOnTarget = nullptr;
	}
}

void UUIManagerComponent::ShowLockOnWidget(AActor* Target)
{
	UE_LOG(LogTemp, Error, TEXT("======== ShowLockOnWidget ========"));
	UE_LOG(LogTemp, Warning, TEXT("Target: %s"), Target ? *Target->GetName() : TEXT("NULL"));
	
	// Check target validity
	if (!IsValidTargetForUI(Target))
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Target is not valid for UI"));
		UE_LOG(LogTemp, Error, TEXT("=================================="));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Target validity: OK"));

	// Hide existing widget
	if (LockOnWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hiding existing widget instance"));
		HideLockOnWidget();
	}

	// Check widget class
	UE_LOG(LogTemp, Warning, TEXT("Display Mode: %d"), (int32)CurrentUIDisplayMode);
	UE_LOG(LogTemp, Warning, TEXT("Widget Class: %s"), LockOnWidgetClass ? *LockOnWidgetClass->GetName() : TEXT("NULL - NOT SET!"));

	if (!LockOnWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
		UE_LOG(LogTemp, Error, TEXT("CRITICAL: LockOnWidgetClass is NOT SET!"));
		UE_LOG(LogTemp, Error, TEXT("Please set it in the Character Blueprint:"));
		UE_LOG(LogTemp, Error, TEXT("  1. Open your Character Blueprint"));
		UE_LOG(LogTemp, Error, TEXT("  2. Select UIManagerComponent"));
		UE_LOG(LogTemp, Error, TEXT("  3. Set 'Lock On Widget Class' to your UMG Widget"));
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
				TEXT("ERROR: LockOnWidgetClass not set! Check Output Log"));
		}
		return;
	}

	// Create and show widget based on display mode
	if (CurrentUIDisplayMode == EUIDisplayMode::ScreenSpace || CurrentUIDisplayMode == EUIDisplayMode::SocketProjection)
	{
		// Get PlayerController
		APlayerController* PC = GetPlayerController();
		if (!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: No PlayerController found!"));
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("PlayerController: OK"));

		// Create widget
		UE_LOG(LogTemp, Warning, TEXT("Creating Widget..."));
		LockOnWidgetInstance = CreateWidget<UUserWidget>(PC, LockOnWidgetClass);
		
		if (!LockOnWidgetInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: CreateWidget returned NULL!"));
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("Widget created: %s"), *LockOnWidgetInstance->GetName());

		// Set current target
		CurrentLockOnTarget = Target;

		// Add to viewport
		LockOnWidgetInstance->AddToViewport(100);
		UE_LOG(LogTemp, Warning, TEXT("Widget added to viewport (Z-Order: 100)"));

		// Check if really in viewport
		if (LockOnWidgetInstance->IsInViewport())
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget IS in viewport: YES"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Widget IS in viewport: NO - Something went wrong!"));
		}

		// Check visibility
		ESlateVisibility Visibility = LockOnWidgetInstance->GetVisibility();
		UE_LOG(LogTemp, Warning, TEXT("Widget Visibility: %d (0=Visible, 1=Collapsed, 2=Hidden, etc.)"), (int32)Visibility);

		// Apply size-based settings if enabled
		if (bEnableSizeAdaptiveUI)
		{
			UTargetDetectionComponent* DetectionComp = GetOwner()->FindComponentByClass<UTargetDetectionComponent>();
			if (DetectionComp)
			{
				EEnemySizeCategory SizeCategory = DetectionComp->GetTargetSizeCategory(Target);
				ApplySizeBasedUISettings(Target, SizeCategory);
			}
		}

		// Set initial position immediately - CRITICAL: Set initial position
		UE_LOG(LogTemp, Warning, TEXT("Setting initial widget position..."));
		
		// Set initial position using the same method as UpdateProjectionWidget
		// to avoid first-frame position mismatch
		UpdateProjectionWidget(Target);

		// Broadcast event
		OnLockOnWidgetShown.Broadcast(Target);
		
		UE_LOG(LogTemp, Error, TEXT("ShowLockOnWidget COMPLETED"));
		UE_LOG(LogTemp, Error, TEXT("=================================="));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				FString::Printf(TEXT("Widget shown for: %s"), *Target->GetName()));
		}
	}
	else if (CurrentUIDisplayMode == EUIDisplayMode::Traditional3D)
	{
		UE_LOG(LogTemp, Warning, TEXT("Using Traditional3D mode"));
		// For 3D widget mode, find or create widget component on target
		UWidgetComponent* WidgetComp = Target->FindComponentByClass<UWidgetComponent>();
		if (WidgetComp && LockOnWidgetClass)
		{
			WidgetComp->SetWidgetClass(LockOnWidgetClass);
			WidgetComp->SetVisibility(true);
			CurrentLockOnTarget = Target;
			TargetsWithActiveWidgets.AddUnique(Target);
			OnLockOnWidgetShown.Broadcast(Target);
			UE_LOG(LogTemp, Log, TEXT("3D widget shown on target"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No WidgetComponent found on target for 3D mode"));
		}
	}
}

void UUIManagerComponent::HideLockOnWidget()
{
	// CRITICAL: Clear CurrentLockOnTarget FIRST to prevent Tick from re-showing
	AActor* TargetToHide = CurrentLockOnTarget;
	CurrentLockOnTarget = nullptr;

	if (LockOnWidgetInstance)
	{
		if (LockOnWidgetInstance->IsInViewport())
		{
			LockOnWidgetInstance->RemoveFromViewport();
		}
		LockOnWidgetInstance = nullptr;
	}

	// Clean up 3D widgets
	for (AActor* Target : TargetsWithActiveWidgets)
	{
		if (Target && IsValid(Target))
		{
			UWidgetComponent* WidgetComp = Target->FindComponentByClass<UWidgetComponent>();
			if (WidgetComp)
			{
				WidgetComp->SetVisibility(false);
			}
		}
	}
	TargetsWithActiveWidgets.Empty();

	OnLockOnWidgetHidden.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("UIManagerComponent: Lock-on widget hidden"));
}

void UUIManagerComponent::UpdateLockOnWidget(AActor* NewTarget, AActor* OldTarget)
{
	// Throttle updates
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastUIUpdateTime < UI_UPDATE_INTERVAL)
	{
		return;
	}
	LastUIUpdateTime = CurrentTime;

	// Clean up old target
	if (OldTarget && OldTarget != NewTarget)
	{
		PreviousLockOnTarget = OldTarget;
	}

	// Show widget for new target
	if (NewTarget && NewTarget != CurrentLockOnTarget)
	{
		ShowLockOnWidget(NewTarget);
	}
}

FVector UUIManagerComponent::GetTargetProjectionLocation(AActor* Target) const
{
	if (!Target)
		return FVector::ZeroVector;

	// ==================== 优先从 SubTargetManager 获取子锁点位置 ====================
	// 如果当前有有效的子锁点，UI 标记跟随子锁点位置
	// 小怪没有 SubTargetContainer，会跳过此段走原有逻辑
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			USubTargetManager* SubTargetMgr = OwnerPawn->FindComponentByClass<USubTargetManager>();
			if (SubTargetMgr && SubTargetMgr->HasValidSubTarget() && SubTargetMgr->GetLockedEntity() == Target)
			{
				FVector SubTargetLocation = SubTargetMgr->GetCurrentLockPosition();
				if (!SubTargetLocation.IsZero())
				{
					return SubTargetLocation;
				}
			}
		}
	}

	// Check for per-actor config override first
	ULockOnConfigComponent* ConfigComp = Target->FindComponentByClass<ULockOnConfigComponent>();
	if (ConfigComp && ConfigComp->IsConfigValid())
	{
		if (ConfigComp->bPreferSocket)
		{
			FVector SocketLocation;
			if (TryGetSocketLocation(Target, ConfigComp->GetEffectiveSocketName(), SocketLocation))
			{
				return SocketLocation;
			}
		}
		return Target->GetActorLocation() + ConfigComp->GetEffectiveOffset();
	}

	// Try to find socket with multiple common socket names
	FVector SocketLocation;
	TArray<FName> SocketNames = {
		FName("LockOnUMG"),       // Your custom socket name
		FName("LockOnSocket"),    // Default fallback name
		FName("Spine"),           // Common bone names
		FName("spine_03"),        // UE Mannequin skeleton
		FName("Spine2"),
		FName("chest"),
		FName("Chest")
	};
	
	for (const FName& SocketName : SocketNames)
	{
		if (TryGetSocketLocation(Target, SocketName, SocketLocation))
		{
			// Socket found - no logging needed in production
			return SocketLocation;
		}
	}

	// Try to get character capsule center with offset (for better positioning on humanoid targets)
	ACharacter* Character = Cast<ACharacter>(Target);
	if (Character && Character->GetCapsuleComponent())
	{
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		FVector CapsuleCenter = Capsule->GetComponentLocation();
		// Return position slightly above center (chest area for humanoids)
		UE_LOG(LogTemp, Log, TEXT("No socket found, using capsule center for '%s'"), *Target->GetName());
		return CapsuleCenter + FVector(0.0f, 0.0f, CapsuleHalfHeight * 0.3f);
	}

	// For Pawns, try to get bounds center
	APawn* Pawn = Cast<APawn>(Target);
	if (Pawn)
	{
		FVector Origin, BoxExtent;
		Pawn->GetActorBounds(false, Origin, BoxExtent);
		// Return bounds center with slight upward offset
		UE_LOG(LogTemp, Log, TEXT("Using bounds center for pawn '%s'"), *Target->GetName());
		return Origin + FVector(0.0f, 0.0f, BoxExtent.Z * 0.3f);
	}

	// Fallback: use projection mode settings
	switch (HybridProjectionSettings.ProjectionMode)
	{
	case EProjectionMode::ActorCenter:
		return Target->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

	case EProjectionMode::BoundsCenter:
		{
			FVector Origin, BoxExtent;
			Target->GetActorBounds(false, Origin, BoxExtent);
			return Origin + FVector(0.0f, 0.0f, BoxExtent.Z * HybridProjectionSettings.BoundsOffsetRatio);
		}

	case EProjectionMode::CustomOffset:
		return Target->GetActorLocation() + HybridProjectionSettings.CustomOffset;

	case EProjectionMode::Hybrid:
	default:
		{
			FVector Origin, BoxExtent;
			Target->GetActorBounds(false, Origin, BoxExtent);
			return Origin + FVector(0.0f, 0.0f, BoxExtent.Z * 0.5f);
		}
	}
}

FVector2D UUIManagerComponent::ProjectToScreen(FVector WorldLocation) const
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectToScreen: No PlayerController"));
		return FVector2D::ZeroVector;
	}

	FVector2D ScreenPosition;
	bool bSuccess = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition, true);
	
	if (bSuccess)
	{
		// Debug log (throttled to reduce spam)
		static float LastLogTime = 0.0f;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastLogTime > 0.5f)
		{
			LastLogTime = CurrentTime;
			UE_LOG(LogTemp, Log, TEXT("Projection: World(%s) -> Screen(%.1f, %.1f)"), 
				*WorldLocation.ToString(), ScreenPosition.X, ScreenPosition.Y);
		}
		return ScreenPosition;
	}
	else
	{
		// Target is behind camera or off-screen
		return FVector2D::ZeroVector;
	}
}

void UUIManagerComponent::UpdateProjectionWidget(AActor* Target)
{
	if (!Target || !LockOnWidgetInstance || !LockOnWidgetInstance->IsInViewport())
	{
		return;
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	FVector WorldLocation = GetTargetProjectionLocation(Target);

	// Project world location to absolute screen pixels
	FVector2D ScreenPosition;
	bool bOnScreen = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition, false);

	if (!bOnScreen)
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	// Convert absolute screen pixels to DPI-scaled viewport coordinates
	float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	if (ViewportScale > KINDA_SMALL_NUMBER)
	{
		ScreenPosition /= ViewportScale;
	}

	LockOnWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	// Directly set position via Canvas Panel Slot for precise DPI-safe positioning
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(LockOnWidgetInstance);
	if (CanvasSlot)
	{
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(ScreenPosition);
	}
	else
	{
		LockOnWidgetInstance->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		LockOnWidgetInstance->SetPositionInViewport(ScreenPosition, false);
	}

	OnSocketProjectionUpdated.Broadcast(Target, ScreenPosition);
}

bool UUIManagerComponent::IsValidTargetForUI(AActor* Target) const
{
	return Target && IsValid(Target) && !Target->IsPendingKill();
}

APlayerController* UUIManagerComponent::GetPlayerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		return Cast<APlayerController>(OwnerPawn->GetController());
	}
	return UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

bool UUIManagerComponent::TryGetSocketLocation(AActor* Target, FName SocketName, FVector& OutLocation) const
{
	if (!Target || SocketName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetSocketLocation: Target or SocketName is invalid"));
		return false;
	}

	if (bDebugSocketLookup)
	{
		UE_LOG(LogTemp, Log, TEXT("TryGetSocketLocation: Looking for socket '%s' on '%s'"), 
			*SocketName.ToString(), *Target->GetName());
	}

	// Try character mesh first
	ACharacter* Character = Cast<ACharacter>(Target);
	if (Character)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (Mesh)
		{
			if (bDebugSocketLookup)
			{
				UE_LOG(LogTemp, Log, TEXT("  Character has mesh: %s"), *Mesh->GetName());
			}
			
			if (Mesh->DoesSocketExist(SocketName))
			{
				OutLocation = Mesh->GetSocketLocation(SocketName);
				if (bDebugSocketLookup)
				{
					UE_LOG(LogTemp, Warning, TEXT("  SUCCESS: Found socket '%s' at %s"), 
						*SocketName.ToString(), *OutLocation.ToString());
				}
				return true;
			}
			else
			{
				if (bDebugSocketLookup)
				{
					UE_LOG(LogTemp, Log, TEXT("  Socket '%s' does NOT exist on this mesh"), *SocketName.ToString());
				}
			}
		}
		else
		{
			if (bDebugSocketLookup)
			{
				UE_LOG(LogTemp, Warning, TEXT("  Character has NO mesh component!"));
			}
		}
	}
	else
	{
		if (bDebugSocketLookup)
		{
			UE_LOG(LogTemp, Log, TEXT("  Target is not a Character, trying to find SkeletalMeshComponent..."));
		}
	}

	// Try all skeletal mesh components on the actor
	TArray<UActorComponent*> MeshComponents = Target->GetComponentsByClass(USkeletalMeshComponent::StaticClass());
	if (bDebugSocketLookup)
	{
		UE_LOG(LogTemp, Log, TEXT("  Found %d SkeletalMeshComponent(s) on actor"), MeshComponents.Num());
	}
	
	for (UActorComponent* Comp : MeshComponents)
	{
		USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Comp);
		if (SkelMesh)
		{
			if (bDebugSocketLookup)
			{
				UE_LOG(LogTemp, Log, TEXT("    Checking mesh: %s"), *SkelMesh->GetName());
			}
			
			if (SkelMesh->DoesSocketExist(SocketName))
			{
				OutLocation = SkelMesh->GetSocketLocation(SocketName);
				if (bDebugSocketLookup)
				{
					UE_LOG(LogTemp, Warning, TEXT("    SUCCESS: Found socket '%s' at %s"), 
						*SocketName.ToString(), *OutLocation.ToString());
				}
				return true;
			}
		}
	}

	if (bDebugSocketLookup)
	{
		UE_LOG(LogTemp, Log, TEXT("  Socket '%s' not found on any mesh"), *SocketName.ToString());
	}
	return false;
}

void UUIManagerComponent::ApplySizeBasedUISettings(AActor* Target, EEnemySizeCategory SizeCategory)
{
	if (!LockOnWidgetInstance)
		return;

	// Get scale and color based on size
	float Scale = SizeUIConfig.MediumEnemyUIScale;
	FLinearColor Color = SizeUIConfig.MediumEnemyColor;

	switch (SizeCategory)
	{
	case EEnemySizeCategory::Small:
		Scale = SizeUIConfig.SmallEnemyUIScale;
		Color = SizeUIConfig.SmallEnemyColor;
		break;
	case EEnemySizeCategory::Large:
		Scale = SizeUIConfig.LargeEnemyUIScale;
		Color = SizeUIConfig.LargeEnemyColor;
		break;
	default:
		break;
	}

	// Try to call Blueprint functions to set scale and color
	UFunction* SetScaleFunc = LockOnWidgetInstance->FindFunction(FName("SetUIScale"));
	if (SetScaleFunc)
	{
		struct FSetScaleParams
		{
			float Scale;
		};
		FSetScaleParams Params;
		Params.Scale = Scale;
		LockOnWidgetInstance->ProcessEvent(SetScaleFunc, &Params);
	}

	UFunction* SetColorFunc = LockOnWidgetInstance->FindFunction(FName("SetUIColor"));
	if (SetColorFunc)
	{
		struct FSetColorParams
		{
			FLinearColor Color;
		};
		FSetColorParams Params;
		Params.Color = Color;
		LockOnWidgetInstance->ProcessEvent(SetColorFunc, &Params);
	}
}

void UUIManagerComponent::DebugListTargetSockets(AActor* Target) const
{
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("DebugListTargetSockets: Target is NULL"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("======== SOCKETS ON %s ========"), *Target->GetName());

	// Check if it's a Character
	ACharacter* Character = Cast<ACharacter>(Target);
	if (Character)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (Mesh && Mesh->SkeletalMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("Character Mesh: %s"), *Mesh->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh Asset: %s"), *Mesh->SkeletalMesh->GetName());
			
			// Get sockets from skeleton
			if (Mesh->SkeletalMesh->GetSkeleton())
			{
				const TArray<class USkeletalMeshSocket*>& Sockets = Mesh->SkeletalMesh->GetSkeleton()->Sockets;
				UE_LOG(LogTemp, Warning, TEXT("Skeleton Sockets (%d):"), Sockets.Num());
				for (const USkeletalMeshSocket* Socket : Sockets)
				{
					if (Socket)
					{
						UE_LOG(LogTemp, Warning, TEXT("  - %s (Bone: %s)"), 
							*Socket->SocketName.ToString(), *Socket->BoneName.ToString());
					}
				}
			}
			
			// Get sockets from mesh asset
			const TArray<class USkeletalMeshSocket*>& MeshSockets = Mesh->SkeletalMesh->GetMeshOnlySocketList();
			UE_LOG(LogTemp, Warning, TEXT("Mesh Sockets (%d):"), MeshSockets.Num());
			for (const USkeletalMeshSocket* Socket : MeshSockets)
			{
				if (Socket)
				{
					UE_LOG(LogTemp, Warning, TEXT("  - %s (Bone: %s)"), 
						*Socket->SocketName.ToString(), *Socket->BoneName.ToString());
				}
			}
			
			// Also list all bone names for reference
			UE_LOG(LogTemp, Warning, TEXT("Bone Names:"));
			const FReferenceSkeleton& RefSkel = Mesh->SkeletalMesh->GetRefSkeleton();
			for (int32 i = 0; i < FMath::Min(20, RefSkel.GetNum()); ++i)
			{
				UE_LOG(LogTemp, Log, TEXT("  Bone[%d]: %s"), i, *RefSkel.GetBoneName(i).ToString());
			}
			if (RefSkel.GetNum() > 20)
			{
				UE_LOG(LogTemp, Log, TEXT("  ... and %d more bones"), RefSkel.GetNum() - 20);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Character has no valid mesh!"));
		}
	}
	else
	{
		// Not a character, check for any skeletal mesh
		TArray<UActorComponent*> MeshComponents = Target->GetComponentsByClass(USkeletalMeshComponent::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("Found %d SkeletalMeshComponent(s)"), MeshComponents.Num());
		
		for (UActorComponent* Comp : MeshComponents)
		{
			USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Comp);
			if (SkelMesh && SkelMesh->SkeletalMesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mesh: %s"), *SkelMesh->GetName());
				
				// Get sockets from skeleton
				if (SkelMesh->SkeletalMesh->GetSkeleton())
				{
					const TArray<class USkeletalMeshSocket*>& Sockets = SkelMesh->SkeletalMesh->GetSkeleton()->Sockets;
					UE_LOG(LogTemp, Warning, TEXT("  Skeleton Sockets (%d):"), Sockets.Num());
					for (const USkeletalMeshSocket* Socket : Sockets)
					{
						if (Socket)
						{
							UE_LOG(LogTemp, Warning, TEXT("    - %s (Bone: %s)"), 
								*Socket->SocketName.ToString(), *Socket->BoneName.ToString());
						}
					}
				}
				
				// Get sockets from mesh asset
				const TArray<class USkeletalMeshSocket*>& MeshSockets = SkelMesh->SkeletalMesh->GetMeshOnlySocketList();
				UE_LOG(LogTemp, Warning, TEXT("  Mesh Sockets (%d):"), MeshSockets.Num());
				for (const USkeletalMeshSocket* Socket : MeshSockets)
				{
					if (Socket)
					{
						UE_LOG(LogTemp, Warning, TEXT("    - %s (Bone: %s)"), 
							*Socket->SocketName.ToString(), *Socket->BoneName.ToString());
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("====================================="));
}
