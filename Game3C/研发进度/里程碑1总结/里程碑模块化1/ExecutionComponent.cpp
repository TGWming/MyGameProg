// Fill out your copyright notice in the Description page of Project Settings.

#include "ExecutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"

// Sets default values for this component's properties
UExecutionComponent::UExecutionComponent()
{
	// Set this component to be ticked every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

	// 初始化处决设置
	ExecutionSettings = FExecutionSettings();
	
	// 初始化状态变量
	CurrentExecutionType = EExecutionType::None;
	CurrentExecutionTarget = nullptr;
	bRiposteWindowActive = false;
	bIsPositioningForExecution = false;
	ExecutionStartTime = 0.0f;
	bExecutionAnimationPlaying = false;
	ExecutionStartPosition = FVector::ZeroVector;
	TargetExecutionPosition = FVector::ZeroVector;
}

// Called when the game starts
void UExecutionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化处决系统
	ResetExecutionState();
	
	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Component initialized successfully"));
	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: BackstabRange=%.1f, BackstabAngle=%.1f"), 
		ExecutionSettings.BackstabRange, ExecutionSettings.BackstabAngle);
}

// Called every frame
void UExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新处决状态
	if (IsExecuting())
	{
		UpdateExecutionState(DeltaTime);
	}

	// 更新位置调整
	if (bIsPositioningForExecution)
	{
		UpdatePositioning(DeltaTime);
	}

	// 定期检查背刺机会（降低频率以优化性能）
	static float LastBackstabCheckTime = 0.0f;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastBackstabCheckTime > 0.2f) // 每0.2秒检查一次
	{
		// 检查附近是否有可背刺的目标
		TArray<AActor*> NearbyActors;
		// 这里可以添加更复杂的目标搜索逻辑
		// 为了保持组件独立性，暂时跳过具体实现
		
		LastBackstabCheckTime = CurrentTime;
	}
}

// ==================== 核心接口函数实现 ====================

bool UExecutionComponent::CanExecute(AActor* Target, EExecutionType ExecutionType) const
{
	// 基础验证
	if (!ValidateExecutionTarget(Target))
	{
		return false;
	}

	// 检查是否已经在执行处决
	if (IsExecuting())
	{
		return false;
	}

	// 根据处决类型进行具体检查
	switch (ExecutionType)
	{
		case EExecutionType::Backstab:
			return CheckBackstabOpportunity(Target);
			
		case EExecutionType::Riposte:
			return CanRiposte(Target);
			
		case EExecutionType::Plunge:
			return CanPlungeAttack();
			
		case EExecutionType::Critical:
			return IsTargetVulnerableToBackstab(Target); // 暴击使用类似背刺的条件
			
		default:
			return false;
	}
}

bool UExecutionComponent::StartExecution(AActor* Target, EExecutionType ExecutionType)
{
	// 检查是否可以执行
	if (!CanExecute(Target, ExecutionType))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Cannot execute %s on target %s"), 
			*UEnum::GetValueAsString(ExecutionType), 
			Target ? *Target->GetName() : TEXT("NULL"));
		return false;
	}

	// 设置处决状态
	CurrentExecutionType = ExecutionType;
	CurrentExecutionTarget = Target;
	ExecutionStartTime = GetWorld()->GetTimeSeconds();
	bExecutionAnimationPlaying = false;

	// 触发处决开始事件
	OnExecutionStarted.Broadcast(Target, ExecutionType);

	// 开始位置调整
	PositionCharacterForExecution(Target, ExecutionType);

	// 播放处决动画
	PlayExecutionAnimation(ExecutionType);

	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Started execution %s on target %s"), 
		*UEnum::GetValueAsString(ExecutionType), *Target->GetName());

	return true;
}

void UExecutionComponent::CompleteExecution()
{
	if (!IsExecuting())
	{
		return;
	}

	AActor* Target = CurrentExecutionTarget;
	EExecutionType ExecutionType = CurrentExecutionType;

	// 应用处决伤害
	if (Target)
	{
		ApplyExecutionDamage(Target, ExecutionType);
	}

	// 触发处决完成事件
	OnExecutionCompleted.Broadcast(Target, ExecutionType);

	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Completed execution %s"), 
		*UEnum::GetValueAsString(ExecutionType));

	// 重置状态
	ResetExecutionState();
}

void UExecutionComponent::CancelExecution()
{
	if (!IsExecuting())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Cancelled execution %s"), 
		*UEnum::GetValueAsString(CurrentExecutionType));

	// 停止动画
	StopExecutionAnimation();

	// 重置状态
	ResetExecutionState();
}

// ==================== 背刺系统函数实现 ====================

bool UExecutionComponent::CheckBackstabOpportunity(AActor* Target) const
{
	if (!ValidateExecutionTarget(Target))
	{
		return false;
	}

	// 检查距离
	if (!IsBackstabDistanceValid(Target))
	{
		return false;
	}

	// 检查角度
	if (!IsBackstabAngleValid(Target))
	{
		return false;
	}

	// 检查目标是否容易受到背刺
	if (!IsTargetVulnerableToBackstab(Target))
	{
		return false;
	}

	return true;
}

bool UExecutionComponent::IsTargetVulnerableToBackstab(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// 这里可以添加更复杂的逻辑，比如检查目标状态
	// 例如：目标是否在攻击、是否被眩晕等
	
	// 基础检查：目标必须是Character类型
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter)
	{
		return false;
	}

	// 检查目标是否存活
	// 这里可以根据项目需要添加健康检查逻辑
	
	return true;
}

FVector UExecutionComponent::GetOptimalBackstabPosition(AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	return GetBackstabPositionBehindTarget(Target);
}

// ==================== 弹反系统函数实现 ====================

bool UExecutionComponent::CanRiposte(AActor* Target) const
{
	if (!ValidateExecutionTarget(Target))
	{
		return false;
	}

	// 检查弹反窗口是否激活
	if (!bRiposteWindowActive)
	{
		return false;
	}

	// 检查距离（弹反距离比背刺稍远）
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	if (Distance > ExecutionSettings.BackstabRange * 1.2f)
	{
		return false;
	}

	return true;
}

void UExecutionComponent::StartRiposteWindow(float Duration)
{
	float WindowDuration = (Duration > 0.0f) ? Duration : ExecutionSettings.RiposteWindow;
	
	bRiposteWindowActive = true;
	
	// 设置定时器来结束弹反窗口
	GetWorld()->GetTimerManager().SetTimer(
		RiposteWindowTimerHandle,
		this,
		&UExecutionComponent::OnRiposteWindowExpired,
		WindowDuration,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Riposte window started (%.2f seconds)"), WindowDuration);
}

void UExecutionComponent::EndRiposteWindow()
{
	bRiposteWindowActive = false;
	
	// 清除定时器
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(RiposteWindowTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(RiposteWindowTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Riposte window ended"));
}

// ==================== 坠落攻击函数实现 ====================

bool UExecutionComponent::CanPlungeAttack() const
{
	// 检查角色是否在空中
	if (!IsCharacterInAir())
	{
		return false;
	}

	// 检查高度是否足够
	float Height = GetCharacterHeight();
	if (Height < ExecutionSettings.PlungeHeightRequirement)
	{
		return false;
	}

	// 检查下方是否有可攻击的目标
	TArray<AActor*> Targets = GetPlungeTargets();
	return Targets.Num() > 0;
}

TArray<AActor*> UExecutionComponent::GetPlungeTargets() const
{
	TArray<AActor*> PlungeTargets;

	if (!GetOwner())
	{
		return PlungeTargets;
	}

	// 从角色位置向下射线检测
	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, ExecutionSettings.PlungeHeightRequirement * 2.0f);

	// 设置射线检测参数
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	// 执行射线检测
	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->LineTraceMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		ECC_Pawn,
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor() && ValidateExecutionTarget(Hit.GetActor()))
			{
				PlungeTargets.Add(Hit.GetActor());
			}
		}
	}

	return PlungeTargets;
}

// ==================== 私有辅助函数实现 ====================

bool UExecutionComponent::ValidateExecutionTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// 检查目标是否有效
	if (!IsValid(Target))
	{
		return false;
	}

	// 检查目标不是自己
	if (Target == GetOwner())
	{
		return false;
	}

	// 检查目标是否是Character类型
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter)
	{
		return false;
	}

	return true;
}

void UExecutionComponent::PositionCharacterForExecution(AActor* Target, EExecutionType ExecutionType)
{
	if (!Target || !GetOwner())
	{
		return;
	}

	bIsPositioningForExecution = true;
	ExecutionStartPosition = GetOwner()->GetActorLocation();

	// 根据处决类型计算目标位置
	switch (ExecutionType)
	{
		case EExecutionType::Backstab:
		case EExecutionType::Critical:
			TargetExecutionPosition = GetBackstabPositionBehindTarget(Target);
			break;
			
		case EExecutionType::Riposte:
			// 弹反位置在目标前方
			TargetExecutionPosition = Target->GetActorLocation() + 
				Target->GetActorForwardVector() * ExecutionSettings.BackstabRange * 0.8f;
			break;
			
		case EExecutionType::Plunge:
			// 坠落攻击保持当前位置
			TargetExecutionPosition = GetOwner()->GetActorLocation();
			bIsPositioningForExecution = false;
			break;
			
		default:
			TargetExecutionPosition = GetOwner()->GetActorLocation();
			bIsPositioningForExecution = false;
			break;
	}

	UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Positioning for execution from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)"),
		ExecutionStartPosition.X, ExecutionStartPosition.Y, ExecutionStartPosition.Z,
		TargetExecutionPosition.X, TargetExecutionPosition.Y, TargetExecutionPosition.Z);
}

float UExecutionComponent::CalculateExecutionDamage(EExecutionType ExecutionType) const
{
	// 基础伤害（这里使用固定值，实际项目中应该从角色属性获取）
	float BaseDamage = 100.0f;

	// 根据处决类型计算伤害倍数
	float DamageMultiplier = 1.0f;
	
	switch (ExecutionType)
	{
		case EExecutionType::Backstab:
			DamageMultiplier = 2.5f;
			break;
			
		case EExecutionType::Riposte:
			DamageMultiplier = 2.0f;
			break;
			
		case EExecutionType::Plunge:
			DamageMultiplier = 3.0f;
			break;
			
		case EExecutionType::Critical:
			DamageMultiplier = ExecutionSettings.CriticalDamageMultiplier;
			break;
			
		default:
			DamageMultiplier = 1.0f;
			break;
	}

	return BaseDamage * DamageMultiplier;
}

bool UExecutionComponent::IsCharacterInAir() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	return MovementComp->IsFalling();
}

float UExecutionComponent::GetCharacterHeight() const
{
	return GetDistanceToGround();
}

float UExecutionComponent::GetDistanceToGround() const
{
	if (!GetOwner())
	{
		return 0.0f;
	}

	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 10000.0f); // 向下100米

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_WorldStatic,
		QueryParams
	);

	if (bHit)
	{
		return FVector::Dist(StartLocation, HitResult.Location);
	}

	return 0.0f;
}

float UExecutionComponent::CalculateAngleBetweenActors(AActor* Actor1, AActor* Actor2) const
{
	if (!Actor1 || !Actor2)
	{
		return 180.0f;
	}

	FVector Actor1Forward = Actor1->GetActorForwardVector();
	FVector ToActor2 = (Actor2->GetActorLocation() - Actor1->GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(Actor1Forward, ToActor2);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

	float AngleRadians = FMath::Acos(DotProduct);
	return FMath::RadiansToDegrees(AngleRadians);
}

USkeletalMeshComponent* UExecutionComponent::GetCharacterMesh() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return nullptr;
	}

	return Character->GetMesh();
}

bool UExecutionComponent::PlayExecutionAnimation(EExecutionType ExecutionType)
{
	USkeletalMeshComponent* Mesh = GetCharacterMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: No mesh component found for animation"));
		return false;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: No anim instance found"));
		return false;
	}

	// 根据处决类型选择动画
	UAnimMontage* MontageToPlay = nullptr;
	
	switch (ExecutionType)
	{
		case EExecutionType::Backstab:
			MontageToPlay = ExecutionSettings.BackstabMontage;
			break;
			
		case EExecutionType::Riposte:
			MontageToPlay = ExecutionSettings.RiposteMontage;
			break;
			
		case EExecutionType::Plunge:
			MontageToPlay = ExecutionSettings.PlungeMontage;
			break;
			
		case EExecutionType::Critical:
			MontageToPlay = ExecutionSettings.BackstabMontage; // 使用背刺动画
			break;
			
		default:
			break;
	}

	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: No animation montage set for execution type %s"), 
			*UEnum::GetValueAsString(ExecutionType));
		return false;
	}

	// 播放动画
	float PlayLength = AnimInstance->Montage_Play(MontageToPlay);
	if (PlayLength > 0.0f)
	{
		bExecutionAnimationPlaying = true;
		
		// 设置动画完成回调 - 修复绑定方法
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUFunction(this, FName("OnExecutionAnimationCompleted"));
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
		
		UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Playing execution animation (%.2f seconds)"), PlayLength);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Failed to play execution animation"));
	return false;
}

void UExecutionComponent::StopExecutionAnimation()
{
	USkeletalMeshComponent* Mesh = GetCharacterMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// 停止当前播放的蒙太奇
	AnimInstance->Montage_Stop(0.2f);
	bExecutionAnimationPlaying = false;
	
	UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Stopped execution animation"));
}

void UExecutionComponent::OnExecutionAnimationCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	bExecutionAnimationPlaying = false;
	
	UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Execution animation completed (Interrupted: %s)"), 
		bInterrupted ? TEXT("Yes") : TEXT("No"));
	
	// 只有在未被打断的情况下才自动完成处决
	if (!bInterrupted)
	{
		CompleteExecution();
	}
}

void UExecutionComponent::OnRiposteWindowExpired()
{
	EndRiposteWindow();
}

void UExecutionComponent::UpdateExecutionState(float DeltaTime)
{
	if (!IsExecuting())
	{
		return;
	}

	// 检查目标是否仍然有效
	if (!ValidateExecutionTarget(CurrentExecutionTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Execution target became invalid, cancelling"));
		CancelExecution();
		return;
	}

	// 检查执行时间是否过长（安全机制）
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - ExecutionStartTime > 10.0f) // 10秒超时
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Execution timeout, auto-completing"));
		CompleteExecution();
		return;
	}
}

void UExecutionComponent::UpdatePositioning(float DeltaTime)
{
	if (!bIsPositioningForExecution || !GetOwner())
	{
		return;
	}

	FVector CurrentPosition = GetOwner()->GetActorLocation();
	float DistanceToTarget = FVector::Dist(CurrentPosition, TargetExecutionPosition);

	// 检查是否到达目标位置
	if (DistanceToTarget < EXECUTION_POSITION_THRESHOLD)
	{
		bIsPositioningForExecution = false;
		UE_LOG(LogTemp, Log, TEXT("ExecutionComponent: Reached execution position"));
		return;
	}

	// 平滑移动到目标位置
	FVector NewPosition = FMath::VInterpTo(
		CurrentPosition,
		TargetExecutionPosition,
		DeltaTime,
		BACKSTAB_POSITION_INTERP_SPEED
	);

	GetOwner()->SetActorLocation(NewPosition);
}

void UExecutionComponent::ApplyExecutionDamage(AActor* Target, EExecutionType ExecutionType)
{
	if (!Target)
	{
		return;
	}

	float Damage = CalculateExecutionDamage(ExecutionType);
	
	// 这里应该调用目标的受伤函数
	// 由于不知道具体的伤害系统，这里只做日志输出
	UE_LOG(LogTemp, Warning, TEXT("ExecutionComponent: Applied %.1f damage to %s (Type: %s)"), 
		Damage, *Target->GetName(), *UEnum::GetValueAsString(ExecutionType));
	
	// 如果目标有健康组件，可以在这里调用
	// 例如：Target->TakeDamage(Damage, ...);
}

void UExecutionComponent::ResetExecutionState()
{
	CurrentExecutionType = EExecutionType::None;
	CurrentExecutionTarget = nullptr;
	bIsPositioningForExecution = false;
	ExecutionStartTime = 0.0f;
	bExecutionAnimationPlaying = false;
	ExecutionStartPosition = FVector::ZeroVector;
	TargetExecutionPosition = FVector::ZeroVector;
	
	// 结束弹反窗口
	EndRiposteWindow();
}

FVector UExecutionComponent::GetBackstabPositionBehindTarget(AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	// 获取目标背后的位置
	FVector TargetLocation = Target->GetActorLocation();
	FVector TargetForward = Target->GetActorForwardVector();
	
	// 在目标背后计算位置
	FVector BackstabPosition = TargetLocation - (TargetForward * ExecutionSettings.BackstabRange * 0.8f);
	
	return BackstabPosition;
}

bool UExecutionComponent::IsBackstabAngleValid(AActor* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	// 计算玩家相对于目标的角度
	FVector TargetForward = Target->GetActorForwardVector();
	FVector ToPlayer = (GetOwner()->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	
	// 计算角度
	float DotProduct = FVector::DotProduct(TargetForward, ToPlayer);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
	
	// 检查玩家是否在目标背后（角度大于90度表示在背后）
	bool bIsBehindTarget = Angle > (180.0f - ExecutionSettings.BackstabAngle);
	
	return bIsBehindTarget;
}

bool UExecutionComponent::IsBackstabDistanceValid(AActor* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	return Distance <= ExecutionSettings.BackstabRange;
}