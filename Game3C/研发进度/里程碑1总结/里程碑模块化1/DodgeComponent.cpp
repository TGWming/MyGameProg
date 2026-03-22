// Fill out your copyright notice in the Description page of Project Settings.

#include "DodgeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UDodgeComponent::UDodgeComponent()
{
	// Set this component to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;
	
	// 初始化状态变量
	bIsDodging = false;
	bIsInvincible = false;
	CurrentDodgeDirection = EDodgeDirection::None;
	LastDodgeDirection = EDodgeDirection::None;
	DodgeStartLocation = FVector::ZeroVector;
	DodgeTargetLocation = FVector::ZeroVector;
	DodgeStartTime = 0.0f;
	StaminaCost = 25.0f;
	bEnableDebugLogs = false;
	
	// 创建时间轴组件
	DodgeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DodgeTimeline"));
	
	// 初始化精力组件引用为空
	StaminaComponent = nullptr;
}

// Called when the game starts
void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化时间轴
	InitializeTimeline();
	
	// 尝试获取精力组件引用（可选）
	AActor* Owner = GetOwner();
	if (Owner)
	{
		// 查找任何可能的精力组件
		TArray<UActorComponent*> Components = Owner->GetComponents().Array();
		for (UActorComponent* Component : Components)
		{
			if (Component && Component->GetClass()->GetName().Contains(TEXT("Stamina")))
			{
				StaminaComponent = Component;
				break;
			}
		}
		
		if (!StaminaComponent && bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: No StaminaComponent found on owner %s - stamina checks disabled"), *Owner->GetName());
		}
		else if (StaminaComponent && bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Found StaminaComponent: %s"), *StaminaComponent->GetClass()->GetName());
		}
	}
	
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Initialized successfully"));
	}
}

// Called every frame
void UDodgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 如果正在闪避，更新移动
	if (bIsDodging)
	{
		UpdateDodgeMovement(DeltaTime);
	}
}

// ==================== 核心接口函数实现 ====================

bool UDodgeComponent::StartDodge(const FVector& InputDirection)
{
	// 检查是否可以闪避
	if (!CanDodge())
	{
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Cannot dodge - conditions not met"));
		}
		return false;
	}
	
	// 计算闪避方向
	EDodgeDirection Direction = CalculateDodgeDirection(InputDirection);
	
	if (Direction == EDodgeDirection::None)
	{
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Invalid dodge direction calculated"));
		}
		return false;
	}
	
	return StartDodgeInDirection(Direction);
}

bool UDodgeComponent::StartDodgeInDirection(EDodgeDirection Direction)
{
	// 检查是否可以闪避
	if (!CanDodge() || Direction == EDodgeDirection::None)
	{
		return false;
	}
	
	// 消耗精力（如果有StaminaComponent）
	if (StaminaComponent)
	{
		// 使用反射调用方法，避免编译错误
		UFunction* ConsumeStaminaFunc = StaminaComponent->GetClass()->FindFunctionByName(TEXT("ConsumeStamina"));
		if (ConsumeStaminaFunc)
		{
			struct FConsumeStaminaParams
			{
				float Amount;
				bool ReturnValue;
			};
			
			FConsumeStaminaParams Params;
			Params.Amount = StaminaCost;
			Params.ReturnValue = false;
			
			StaminaComponent->ProcessEvent(ConsumeStaminaFunc, &Params);
			
			if (!Params.ReturnValue)
			{
				if (bEnableDebugLogs)
				{
					UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Not enough stamina for dodge"));
				}
				return false;
			}
		}
	}
	
	// 计算闪避目标位置
	FVector TargetLocation = CalculateDodgeTargetLocation(Direction);
	
	// 检查路径安全性
	if (!IsPathSafe(GetOwner()->GetActorLocation(), TargetLocation))
	{
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Dodge path is not safe"));
		}
		
		// 退还精力（如果有StaminaComponent）
		if (StaminaComponent)
		{
			UFunction* RestoreStaminaFunc = StaminaComponent->GetClass()->FindFunctionByName(TEXT("RestoreStamina"));
			if (RestoreStaminaFunc)
			{
				struct FRestoreStaminaParams
				{
					float Amount;
				};
				
				FRestoreStaminaParams Params;
				Params.Amount = StaminaCost;
				
				StaminaComponent->ProcessEvent(RestoreStaminaFunc, &Params);
			}
		}
		return false;
	}
	
	// 设置闪避状态
	bIsDodging = true;
	CurrentDodgeDirection = Direction;
	LastDodgeDirection = Direction;
	DodgeStartLocation = GetOwner()->GetActorLocation();
	DodgeTargetLocation = TargetLocation;
	DodgeStartTime = GetWorld()->GetTimeSeconds();
	
	// 播放闪避动画
	PlayDodgeAnimation(Direction);
	
	// 启动时间轴
	if (DodgeTimeline)
	{
		DodgeTimeline->PlayFromStart();
	}
	
	// 延迟启动无敌帧
	if (DodgeSettings.InvincibilityStartTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimerHandle,
			this,
			&UDodgeComponent::StartInvincibilityFrames,
			DodgeSettings.InvincibilityStartTime,
			false
		);
	}
	else
	{
		StartInvincibilityFrames();
	}
	
	// 触发开始事件
	OnDodgeStarted.Broadcast(Direction);
	
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Started dodge in direction: %d"), (int32)Direction);
	}
	
	return true;
}

void UDodgeComponent::EndDodge()
{
	if (!bIsDodging)
	{
		return;
	}
	
	EDodgeDirection EndedDirection = CurrentDodgeDirection;
	
	// 重置状态
	bIsDodging = false;
	CurrentDodgeDirection = EDodgeDirection::None;
	
	// 停止时间轴
	if (DodgeTimeline)
	{
		DodgeTimeline->Stop();
	}
	
	// 结束无敌帧
	EndInvincibilityFrames();
	
	// 清除定时器
	if (InvincibilityTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	}
	
	// 触发结束事件
	OnDodgeEnded.Broadcast(EndedDirection);
	
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Ended dodge"));
	}
}

bool UDodgeComponent::CanDodge() const
{
	// 检查基本条件
	if (bIsDodging)
	{
		return false;
	}
	
	// 检查角色状态
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}
	
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character)
	{
		return false;
	}
	
	// 检查角色是否在地面上
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent || !MovementComponent->IsMovingOnGround())
	{
		return false;
	}
	
	// 检查精力（如果有StaminaComponent且有相应方法）
	if (StaminaComponent)
	{
		UFunction* HasEnoughStaminaFunc = StaminaComponent->GetClass()->FindFunctionByName(TEXT("HasEnoughStamina"));
		if (HasEnoughStaminaFunc)
		{
			struct FHasEnoughStaminaParams
			{
				float Amount;
				bool ReturnValue;
			};
			
			FHasEnoughStaminaParams Params;
			Params.Amount = StaminaCost;
			Params.ReturnValue = true; // 默认为true，如果没有精力系统就允许闪避
			
			StaminaComponent->ProcessEvent(HasEnoughStaminaFunc, &Params);
			
			if (!Params.ReturnValue)
			{
				return false;
			}
		}
		// 如果没有找到HasEnoughStamina方法，就假设有足够精力
	}
	
	return true;
}

void UDodgeComponent::CancelDodge()
{
	if (bIsDodging)
	{
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Dodge cancelled"));
		}
		
		EndDodge();
	}
}

// ==================== 无敌帧管理实现 ====================

void UDodgeComponent::StartInvincibilityFrames()
{
	if (!bIsInvincible)
	{
		bIsInvincible = true;
		OnInvincibilityChanged.Broadcast(true);
		
		// 设置无敌帧结束定时器
		if (DodgeSettings.InvincibilityDuration > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				InvincibilityTimerHandle,
				this,
				&UDodgeComponent::EndInvincibilityFrames,
				DodgeSettings.InvincibilityDuration,
				false
			);
		}
		
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("DodgeComponent: Invincibility frames started"));
		}
	}
}

void UDodgeComponent::EndInvincibilityFrames()
{
	if (bIsInvincible)
	{
		bIsInvincible = false;
		OnInvincibilityChanged.Broadcast(false);
		
		// 清除定时器
		if (InvincibilityTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
		}
		
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("DodgeComponent: Invincibility frames ended"));
		}
	}
}

bool UDodgeComponent::IsInvincible() const
{
	return bIsInvincible;
}

// ==================== 状态查询函数实现 ====================

bool UDodgeComponent::IsDodging() const
{
	return bIsDodging;
}

EDodgeDirection UDodgeComponent::GetLastDodgeDirection() const
{
	return LastDodgeDirection;
}

float UDodgeComponent::GetDodgeProgress() const
{
	if (!bIsDodging || DodgeSettings.DodgeDuration <= 0.0f)
	{
		return 0.0f;
	}
	
	float ElapsedTime = GetWorld()->GetTimeSeconds() - DodgeStartTime;
	return FMath::Clamp(ElapsedTime / DodgeSettings.DodgeDuration, 0.0f, 1.0f);
}

// ==================== 私有辅助函数实现 ====================

void UDodgeComponent::UpdateDodgeMovement(float DeltaTime)
{
	if (!bIsDodging || !DodgeTimeline)
	{
		return;
	}
	
	// 时间轴会自动处理移动更新
	// 这里可以添加额外的移动逻辑或碰撞检测
}

EDodgeDirection UDodgeComponent::CalculateDodgeDirection(const FVector& InputDirection) const
{
	if (InputDirection.IsNearlyZero())
	{
		return EDodgeDirection::None;
	}
	
	// 获取角色的前向量和右向量
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return EDodgeDirection::None;
	}
	
	FVector Forward = Owner->GetActorForwardVector();
	FVector Right = Owner->GetActorRightVector();
	
	// 将输入方向投影到角色的本地坐标系
	float ForwardDot = FVector::DotProduct(InputDirection.GetSafeNormal(), Forward);
	float RightDot = FVector::DotProduct(InputDirection.GetSafeNormal(), Right);
	
	// 定义阈值
	const float Threshold = 0.5f;
	
	// 判断主要方向
	bool bForward = ForwardDot > Threshold;
	bool bBackward = ForwardDot < -Threshold;
	bool bRight = RightDot > Threshold;
	bool bLeft = RightDot < -Threshold;
	
	// 组合方向判断
	if (bForward && bLeft)
	{
		return EDodgeDirection::ForwardLeft;
	}
	else if (bForward && bRight)
	{
		return EDodgeDirection::ForwardRight;
	}
	else if (bBackward && bLeft)
	{
		return EDodgeDirection::BackwardLeft;
	}
	else if (bBackward && bRight)
	{
		return EDodgeDirection::BackwardRight;
	}
	else if (bForward)
	{
		return EDodgeDirection::Forward;
	}
	else if (bBackward)
	{
		return EDodgeDirection::Backward;
	}
	else if (bLeft)
	{
		return EDodgeDirection::Left;
	}
	else if (bRight)
	{
		return EDodgeDirection::Right;
	}
	
	return EDodgeDirection::None;
}

UAnimMontage* UDodgeComponent::GetDodgeMontageForDirection(EDodgeDirection Direction) const
{
	switch (Direction)
	{
	case EDodgeDirection::Forward:
	case EDodgeDirection::ForwardLeft:
	case EDodgeDirection::ForwardRight:
		return DodgeSettings.ForwardDodgeMontage;
		
	case EDodgeDirection::Backward:
	case EDodgeDirection::BackwardLeft:
	case EDodgeDirection::BackwardRight:
		return DodgeSettings.BackwardDodgeMontage;
		
	case EDodgeDirection::Left:
		return DodgeSettings.LeftDodgeMontage;
		
	case EDodgeDirection::Right:
		return DodgeSettings.RightDodgeMontage;
		
	default:
		return nullptr;
	}
}

void UDodgeComponent::OnDodgeTimelineUpdate(float Value)
{
	if (!bIsDodging)
	{
		return;
	}
	
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	
	// 应用速度曲线调整
	float SpeedMultiplier = 1.0f;
	if (DodgeSettings.DodgeSpeedCurve)
	{
		SpeedMultiplier = DodgeSettings.DodgeSpeedCurve->GetFloatValue(Value);
	}
	
	// 计算当前位置
	FVector CurrentLocation = FMath::Lerp(DodgeStartLocation, DodgeTargetLocation, Value * SpeedMultiplier);
	
	// 设置位置
	Owner->SetActorLocation(CurrentLocation, true);
	
	if (bEnableDebugLogs)
	{
		static float LastLogTime = 0.0f;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastLogTime > 0.1f) // 每0.1秒记录一次
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("DodgeComponent: Timeline progress: %.2f, Speed: %.2f"), Value, SpeedMultiplier);
			LastLogTime = CurrentTime;
		}
	}
}

void UDodgeComponent::OnDodgeTimelineFinished()
{
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("DodgeComponent: Timeline finished"));
	}
	
	EndDodge();
}

void UDodgeComponent::InitializeTimeline()
{
	if (!DodgeTimeline)
	{
		return;
	}
	
	// 创建时间轴曲线（如果没有设置速度曲线，使用默认的线性曲线）
	if (DodgeSettings.DodgeSpeedCurve)
	{
		// 绑定时间轴更新事件
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("OnDodgeTimelineUpdate"));
		DodgeTimeline->AddInterpFloat(DodgeSettings.DodgeSpeedCurve, UpdateDelegate);
	}
	
	// 绑定时间轴完成事件
	FOnTimelineEvent FinishDelegate;
	FinishDelegate.BindUFunction(this, FName("OnDodgeTimelineFinished"));
	DodgeTimeline->SetTimelineFinishedFunc(FinishDelegate);
	
	// 设置时间轴长度
	DodgeTimeline->SetTimelineLength(DodgeSettings.DodgeDuration);
	
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("DodgeComponent: Timeline initialized with duration: %.2f"), DodgeSettings.DodgeDuration);
	}
}

void UDodgeComponent::PlayDodgeAnimation(EDodgeDirection Direction)
{
	UAnimMontage* MontageToPlay = GetDodgeMontageForDirection(Direction);
	
	if (!MontageToPlay)
	{
		if (bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: No animation montage set for direction: %d"), (int32)Direction);
		}
		return;
	}
	
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character || !Character->GetMesh())
	{
		return;
	}
	
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}
	
	// 播放动画蒙太奇
	float PlayRate = 1.0f;
	if (DodgeSettings.DodgeDuration > 0.0f && MontageToPlay->GetPlayLength() > 0.0f)
	{
		PlayRate = MontageToPlay->GetPlayLength() / DodgeSettings.DodgeDuration;
	}
	
	AnimInstance->Montage_Play(MontageToPlay, PlayRate);
	
	if (bEnableDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("DodgeComponent: Playing dodge animation for direction: %d, PlayRate: %.2f"), (int32)Direction, PlayRate);
	}
}

FVector UDodgeComponent::CalculateDodgeTargetLocation(EDodgeDirection Direction) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}
	
	FVector StartLocation = Owner->GetActorLocation();
	FVector DirectionVector = GetDirectionVector(Direction);
	
	return StartLocation + (DirectionVector * DodgeSettings.DodgeDistance);
}

FVector UDodgeComponent::GetDirectionVector(EDodgeDirection Direction) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}
	
	FVector Forward = Owner->GetActorForwardVector();
	FVector Right = Owner->GetActorRightVector();
	
	switch (Direction)
	{
	case EDodgeDirection::Forward:
		return Forward;
		
	case EDodgeDirection::Backward:
		return -Forward;
		
	case EDodgeDirection::Left:
		return -Right;
		
	case EDodgeDirection::Right:
		return Right;
		
	case EDodgeDirection::ForwardLeft:
		return (Forward - Right).GetSafeNormal();
		
	case EDodgeDirection::ForwardRight:
		return (Forward + Right).GetSafeNormal();
		
	case EDodgeDirection::BackwardLeft:
		return (-Forward - Right).GetSafeNormal();
		
	case EDodgeDirection::BackwardRight:
		return (-Forward + Right).GetSafeNormal();
		
	default:
		return FVector::ZeroVector;
	}
}

bool UDodgeComponent::IsPathSafe(const FVector& StartLocation, const FVector& EndLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	
	// 进行射线检测，检查路径上是否有障碍物
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	
	// 使用角色胶囊体的半径进行球形追踪
	float CapsuleRadius = 50.0f; // 默认半径
	
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->GetCapsuleComponent())
	{
		CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
	
	bool bHit = World->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(CapsuleRadius),
		QueryParams
	);
	
	// 如果没有撞击到任何东西，路径是安全的
	bool bIsSafe = !bHit;
	
	if (bEnableDebugLogs && !bIsSafe)
	{
		UE_LOG(LogTemp, Warning, TEXT("DodgeComponent: Dodge path blocked by: %s"), 
			HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("Unknown"));
	}
	
	return bIsSafe;
}