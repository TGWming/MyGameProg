// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetDetectionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"

UTargetDetectionComponent::UTargetDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// 初始化内部状态
	LockOnDetectionSphere = nullptr;
	LastTargetSearchTime = 0.0f;
	LastSizeUpdateTime = 0.0f;
	
	// 清空容器
	LockOnCandidates.Empty();
	EnemySizeCache.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("TargetDetectionComponent: Initialized"));
}

void UTargetDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent: BeginPlay called"));
}

void UTargetDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!LockOnDetectionSphere || !GetOwnerCharacter())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 定期查找可锁定目标
	if (CurrentTime - LastTargetSearchTime > TARGET_SEARCH_INTERVAL)
	{
		FindLockOnCandidates();
		LastTargetSearchTime = CurrentTime;

		// 高频日志优化 - 添加降频控制
		if (bEnableTargetDetectionDebugLogs)
		{
			static int32 FrameCounter = 0;
			if (++FrameCounter % 300 == 0) // 每5秒只记录一次（60fps * 5）
			{
				UE_LOG(LogTemp, Verbose, TEXT("TargetDetectionComponent: Target search completed, found %d candidates"), LockOnCandidates.Num());
			}
		}
	}

	// 定期更新敌人尺寸缓存
	if (CurrentTime - LastSizeUpdateTime > SIZE_UPDATE_INTERVAL)
	{
		UpdateEnemySizeCache();
		LastSizeUpdateTime = CurrentTime;

		// 高频日志优化 - 添加降频控制
		if (bEnableSizeAnalysisDebugLogs)
		{
			static int32 SizeFrameCounter = 0;
			if (++SizeFrameCounter % 300 == 0) // 每5秒只记录一次
			{
				UE_LOG(LogTemp, Verbose, TEXT("TargetDetectionComponent: Size cache updated, tracking %d enemies"), EnemySizeCache.Num());
			}
		}
	}
}

void UTargetDetectionComponent::SetLockOnDetectionSphere(USphereComponent* DetectionSphere)
{
	LockOnDetectionSphere = DetectionSphere;
	
	if (LockOnDetectionSphere)
	{
		// 更新检测球体半径
		LockOnDetectionSphere->SetSphereRadius(LockOnSettings.LockOnRange);
		UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent: Detection sphere set and configured"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TargetDetectionComponent: Detection sphere is null!"));
	}
}

EEnemySizeCategory UTargetDetectionComponent::GetTargetSizeCategory(AActor* Target)
{
	if (!Target)
	{
		return EEnemySizeCategory::Unknown;
	}

	// 检查缓存
	EEnemySizeCategory* CachedSize = EnemySizeCache.Find(Target);
	if (CachedSize)
	{
		return *CachedSize;
	}

	// 分析并缓存结果
	EEnemySizeCategory SizeCategory = AnalyzeTargetSize(Target);
	EnemySizeCache.Add(Target, SizeCategory);

	// 触发事件
	if (OnValidTargetFound.IsBound())
	{
		OnValidTargetFound.Broadcast(Target, SizeCategory);
	}

	return SizeCategory;
}

void UTargetDetectionComponent::FindLockOnCandidates()
{
	if (!LockOnDetectionSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("TargetDetectionComponent::FindLockOnCandidates: LockOnDetectionSphere is null!"));
		return;
	}

	// 清空上次的候选列表
	LockOnCandidates.Empty();

	// 获取球体范围内的所有重叠Actor
	TArray<AActor*> OverlappingActors;
	LockOnDetectionSphere->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	// 临时存储有效目标
	TArray<AActor*> ValidTargets;
	
	for (AActor* Actor : OverlappingActors)
	{
		if (IsValidLockOnTarget(Actor))
		{
			ValidTargets.Add(Actor);
		}
	}

	// 按角度排序有效目标 - 从左到右
	if (ValidTargets.Num() > 1)
	{
		SortCandidatesByDirection(ValidTargets);
	}

	// 将排序后的目标添加到候选列表
	LockOnCandidates = ValidTargets;

	// 触发目标更新事件
	if (OnTargetsUpdated.IsBound())
	{
		OnTargetsUpdated.Broadcast(LockOnCandidates);
	}

	// 高频日志优化 - 添加降频控制
	if (bEnableTargetDetectionDebugLogs)
	{
		static int32 FindFrameCounter = 0;
		if (++FindFrameCounter % 120 == 0) // 每2秒只记录一次
		{
			UE_LOG(LogTemp, Verbose, TEXT("TargetDetectionComponent: Lock-on candidates updated: %d targets available"), LockOnCandidates.Num());
		}
	}
}

bool UTargetDetectionComponent::IsValidLockOnTarget(AActor* Target)
{
	if (!ValidateBasicTargetConditions(Target))
	{
		return false;
	}

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return false;
	}

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// 检查距离
	float Distance = FVector::Dist(PlayerLocation, TargetLocation);
	if (Distance > LockOnSettings.LockOnRange)
	{
		return false;
	}

	// 执行射线检测检查视线遮挡
	if (!PerformLineOfSightCheck(Target))
	{
		return false;
	}

	return true;
}

bool UTargetDetectionComponent::IsTargetStillLockable(AActor* Target)
{
	if (!ValidateBasicTargetConditions(Target))
	{
		return false;
	}

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return false;
	}

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// 使用更大的距离范围来保持锁定
	float Distance = FVector::Dist(PlayerLocation, TargetLocation);
	float ExtendedLockOnRange = LockOnSettings.LockOnRange * LockOnSettings.ExtendedLockRangeMultiplier;
	
	if (Distance > ExtendedLockOnRange)
	{
		return false;
	}

	return true;
}

AActor* UTargetDetectionComponent::GetBestTargetFromList(const TArray<AActor*>& TargetList)
{
	if (TargetList.Num() == 0)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	AController* OwnerController = GetOwnerController();
	
	if (!OwnerCharacter || !OwnerController)
		return nullptr;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector CameraForward = OwnerController->GetControlRotation().Vector();

	for (AActor* Candidate : TargetList)
	{
		if (!IsValid(Candidate))
			continue;
			
		float Score = CalculateTargetScore(Candidate);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

AActor* UTargetDetectionComponent::GetBestSectorLockTarget()
{
	FindLockOnCandidates();

	if (LockOnCandidates.Num() == 0)
		return nullptr;

	// 首先尝试在扇形锁定区域内找目标
	TArray<AActor*> SectorTargets;
	TArray<AActor*> EdgeTargets;

	for (AActor* Candidate : LockOnCandidates)
	{
		if (!Candidate)
			continue;
			
		if (IsTargetInSectorLockZone(Candidate))
		{
			SectorTargets.Add(Candidate);
		}
		else if (IsTargetInEdgeDetectionZone(Candidate))
		{
			EdgeTargets.Add(Candidate);
		}
	}

	// 优先从扇形区域内选择目标
	if (SectorTargets.Num() > 0)
	{
		return GetBestTargetFromList(SectorTargets);
	}

	// 如果扇形区域内没有目标，检查边缘区域
	if (EdgeTargets.Num() > 0)
	{
		return GetBestTargetFromList(EdgeTargets);
	}

	return nullptr;
}

AActor* UTargetDetectionComponent::TryGetSectorLockTarget()
{
	if (LockOnCandidates.Num() == 0)
		return nullptr;

	// 筛选扇形区域内的目标
	TArray<AActor*> SectorTargets;
	
	for (AActor* Candidate : LockOnCandidates)
	{
		if (Candidate && IsTargetInSectorLockZone(Candidate))
		{
			SectorTargets.Add(Candidate);
		}
	}

	// 如果扇形区域内有目标，返回最佳目标
	if (SectorTargets.Num() > 0)
	{
		if (bEnableTargetDetectionDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent: Found %d targets in sector lock zone"), SectorTargets.Num());
		}
		return GetBestTargetFromList(SectorTargets);
	}

	return nullptr;
}

AActor* UTargetDetectionComponent::TryGetCameraCorrectionTarget()
{
	if (LockOnCandidates.Num() == 0)
		return nullptr;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return nullptr;

	// 寻找最近的目标（不限制角度）
	AActor* ClosestTarget = nullptr;
	float ClosestDistance = FLT_MAX;
	
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	
	for (AActor* Candidate : LockOnCandidates)
	{
		if (!Candidate)
			continue;
			
		float Distance = FVector::Dist(PlayerLocation, Candidate->GetActorLocation());
		
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestTarget = Candidate;
		}
	}

	// 检查最近的目标是否在合理的修正范围内
	if (ClosestTarget)
	{
		float AngleToTarget = CalculateAngleToTarget(ClosestTarget);
		
		// 只对角度在合理范围内的目标进行相机修正（避免修正到背后的敌人）
		if (AngleToTarget <= 160.0f) // 最大160度，即左右160度范围内
		{
			if (bEnableTargetDetectionDebugLogs)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent: Found camera correction target: %s (Distance: %.1f, Angle: %.1f degrees)"), 
					*ClosestTarget->GetName(), ClosestDistance, AngleToTarget);
			}
			return ClosestTarget;
		}
		else
		{
			if (bEnableTargetDetectionDebugLogs)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent: Closest target %s is too far behind (%.1f degrees), no correction"), 
					*ClosestTarget->GetName(), AngleToTarget);
			}
		}
	}

	return nullptr;
}

void UTargetDetectionComponent::SortCandidatesByDirection(TArray<AActor*>& Targets)
{
	if (Targets.Num() <= 1)
		return;

	// 移除无效指针
	Targets.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

	if (Targets.Num() <= 1)
		return;

	// 使用Lambda表达式进行排序，从左到右（角度从小到大）
	Targets.Sort([this](const AActor& A, const AActor& B) {
		if (!IsValid(&A) || !IsValid(&B)) 
			return false;
		
		float AngleA = CalculateDirectionAngle(const_cast<AActor*>(&A));
		float AngleB = CalculateDirectionAngle(const_cast<AActor*>(&B));
		return AngleA < AngleB;
	});
}

bool UTargetDetectionComponent::HasCandidatesInSphere()
{
	FindLockOnCandidates();
	return LockOnCandidates.Num() > 0;
}

// ==================== 新增的敌人尺寸分析功能 ====================

TArray<AActor*> UTargetDetectionComponent::GetTargetsBySize(EEnemySizeCategory SizeCategory)
{
	TArray<AActor*> FilteredTargets;

	for (AActor* Candidate : LockOnCandidates)
	{
		if (Candidate && GetTargetSizeCategory(Candidate) == SizeCategory)
		{
			FilteredTargets.Add(Candidate);
		}
	}

	return FilteredTargets;
}

AActor* UTargetDetectionComponent::GetNearestTargetBySize(EEnemySizeCategory SizeCategory)
{
	TArray<AActor*> TargetsOfSize = GetTargetsBySize(SizeCategory);
	
	if (TargetsOfSize.Num() == 0)
		return nullptr;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return nullptr;

	AActor* NearestTarget = nullptr;
	float NearestDistance = FLT_MAX;
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();

	for (AActor* Target : TargetsOfSize)
	{
		if (!Target)
			continue;

		float Distance = FVector::Dist(PlayerLocation, Target->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestTarget = Target;
		}
	}

	return NearestTarget;
}

TMap<EEnemySizeCategory, int32> UTargetDetectionComponent::GetSizeCategoryStatistics()
{
	TMap<EEnemySizeCategory, int32> Statistics;
	
	// 初始化所有分类为0
	Statistics.Add(EEnemySizeCategory::Small, 0);
	Statistics.Add(EEnemySizeCategory::Medium, 0);
	Statistics.Add(EEnemySizeCategory::Large, 0);
	Statistics.Add(EEnemySizeCategory::Unknown, 0);

	// 统计每个分类的数量
	for (AActor* Candidate : LockOnCandidates)
	{
		if (Candidate)
		{
			EEnemySizeCategory Category = GetTargetSizeCategory(Candidate);
			if (int32* Count = Statistics.Find(Category))
			{
				(*Count)++;
			}
		}
	}

	return Statistics;
}

void UTargetDetectionComponent::UpdateTargetSizeCategory(AActor* Target)
{
	if (!Target)
		return;

	EEnemySizeCategory NewCategory = AnalyzeTargetSize(Target);
	EnemySizeCache.FindOrAdd(Target) = NewCategory;

	if (bEnableSizeAnalysisDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("TargetDetectionComponent: Updated size category for %s: %s"), 
			*Target->GetName(), 
			*UEnum::GetValueAsString(NewCategory));
	}
}

void UTargetDetectionComponent::CleanupSizeCache()
{
	TArray<AActor*> InvalidActors;

	// 查找无效的Actor
	for (auto& Pair : EnemySizeCache)
	{
		if (!IsValid(Pair.Key))
		{
			InvalidActors.Add(Pair.Key);
		}
	}

	// 移除无效的缓存条目
	for (AActor* InvalidActor : InvalidActors)
	{
		EnemySizeCache.Remove(InvalidActor);
	}

	if (bEnableSizeAnalysisDebugLogs && InvalidActors.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("TargetDetectionComponent: Cleaned up %d invalid size cache entries"), InvalidActors.Num());
	}
}

// ==================== 内部辅助函数 ====================

float UTargetDetectionComponent::CalculateTargetBoundingBoxSize(AActor* Target) const
{
	if (!Target)
		return 0.0f;

	// 获取Actor的边界盒
	FVector Origin, BoxExtent;
	Target->GetActorBounds(false, Origin, BoxExtent);

	// 计算最大维度作为尺寸参考
	float MaxDimension = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z) * 2.0f; // BoxExtent是半长度

	return MaxDimension;
}

EEnemySizeCategory UTargetDetectionComponent::AnalyzeTargetSize(AActor* Target)
{
	if (!Target)
		return EEnemySizeCategory::Unknown;

	float BoundingBoxSize = CalculateTargetBoundingBoxSize(Target);

	// 根据高级相机设置中的阈值进行分类
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

bool UTargetDetectionComponent::IsTargetInSectorLockZone(AActor* Target) const
{
	AController* OwnerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (!Target || !OwnerController || !OwnerCharacter)
		return false;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CameraForward = OwnerController->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// 检查是否在扇形锁定区域内
	bool bInSectorZone = AngleDegrees <= (LockOnSettings.SectorLockAngle * 0.5f);

	return bInSectorZone;
}

bool UTargetDetectionComponent::IsTargetInEdgeDetectionZone(AActor* Target) const
{
	AController* OwnerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (!Target || !OwnerController || !OwnerCharacter)
		return false;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CameraForward = OwnerController->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// 检查是否在边缘检测区域内
	bool bInEdgeZone = (AngleDegrees > (LockOnSettings.SectorLockAngle * 0.5f)) && 
					   (AngleDegrees <= (LockOnSettings.EdgeDetectionAngle * 0.5f));

	return bInEdgeZone;
}

float UTargetDetectionComponent::CalculateAngleToTarget(AActor* Target) const
{
	AController* OwnerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (!IsValid(Target) || !OwnerController || !OwnerCharacter)
		return 180.0f;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CurrentCameraForward = OwnerController->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度差异
	float DotProduct = FVector::DotProduct(CurrentCameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

float UTargetDetectionComponent::CalculateDirectionAngle(AActor* Target) const
{
	AController* OwnerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (!IsValid(Target) || !OwnerController || !OwnerCharacter)
		return 0.0f;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FRotator ControlRotation = OwnerController->GetControlRotation();
	FVector CurrentCameraForward = ControlRotation.Vector();
	FVector CurrentCameraRight = ControlRotation.RotateVector(FVector::RightVector);

	// 计算到目标的向量
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算相对于相机前方向的角度
	float ForwardDot = FVector::DotProduct(CurrentCameraForward, ToTarget);
	float RightDot = FVector::DotProduct(CurrentCameraRight, ToTarget);

	// 使用atan2计算角度，范围是-180到180
	float AngleRadians = FMath::Atan2(RightDot, ForwardDot);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

void UTargetDetectionComponent::UpdateEnemySizeCache()
{
	// 首先清理无效缓存
	CleanupSizeCache();

	// 为新发现的目标更新尺寸分类
	for (AActor* Candidate : LockOnCandidates)
	{
		if (Candidate && !EnemySizeCache.Contains(Candidate))
		{
			UpdateTargetSizeCategory(Candidate);
		}
	}
}

ACharacter* UTargetDetectionComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

AController* UTargetDetectionComponent::GetOwnerController() const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	return OwnerCharacter ? OwnerCharacter->GetController() : nullptr;
}

// ==================== 私有辅助函数 ====================

bool UTargetDetectionComponent::PerformLineOfSightCheck(AActor* Target) const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !Target)
		return false;

	FHitResult HitResult;
	FVector StartLocation = OwnerCharacter->GetActorLocation() + FVector(0, 0, LockOnSettings.RaycastHeightOffset);
	FVector EndLocation = Target->GetActorLocation() + FVector(0, 0, LockOnSettings.RaycastHeightOffset);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 如果有遮挡且击中的不是目标本身，则无效
	return !bHit || HitResult.GetActor() == Target;
}

float UTargetDetectionComponent::CalculateTargetScore(AActor* Target) const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	AController* OwnerController = GetOwnerController();
	
	if (!OwnerCharacter || !OwnerController || !Target)
		return -1.0f;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector ToTarget = (Target->GetActorLocation() - PlayerLocation).GetSafeNormal();
	float Distance = FVector::Dist(PlayerLocation, Target->GetActorLocation());
	
	// 防止除零错误
	if (LockOnSettings.LockOnRange <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("TargetDetectionComponent: LockOnRange is zero or negative!"));
		return -1.0f;
	}
	
	// 使用相机前方向量计算点积
	FVector CameraForward = OwnerController->GetControlRotation().Vector();
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);

	// 评分算法：角度因素占70%，距离因素占30%
	float NormalizedDistance = FMath::Sqrt(Distance / LockOnSettings.LockOnRange);
	float AngleFactor = DotProduct;
	float DistanceFactor = 1.0f - NormalizedDistance;
	
	float Score = (AngleFactor * 0.7f) + (DistanceFactor * 0.3f);

	// 排除几乎在相同位置的目标
	if (Distance < 50.0f)
	{
		Score -= 0.5f; // 减少评分，避免锁定到几乎重叠的目标
	}

	return Score;
}

bool UTargetDetectionComponent::ValidateBasicTargetConditions(AActor* Target) const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (!Target || Target == OwnerCharacter)
	{
		return false;
	}

	// 添加敌友识别检查
	if (Target->ActorHasTag(FName("Friendly")) || Target->ActorHasTag(FName("Player")))
	{
		return false;
	}

	// 检查目标是否还活着（如果目标为Pawn）	
	if (APawn* TargetPawn = Cast<APawn>(Target))
	{
		// 检查Pawn是否被销毁或无效
		if (!IsValid(TargetPawn) || TargetPawn->IsPendingKill())
		{
			return false;
		}
	}

	return true;
}