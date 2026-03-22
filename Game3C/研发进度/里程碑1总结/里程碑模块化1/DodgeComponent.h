// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "Animation/AnimMontage.h"
#include "Curves/CurveFloat.h"
#include "DodgeComponent.generated.h"

// 前向声明
class UStaminaComponent;

// 闪避方向枚举
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	None			UMETA(DisplayName = "None"),
	Forward			UMETA(DisplayName = "Forward"),
	Backward		UMETA(DisplayName = "Backward"),
	Left			UMETA(DisplayName = "Left"),
	Right			UMETA(DisplayName = "Right"),
	ForwardLeft		UMETA(DisplayName = "Forward Left"),
	ForwardRight	UMETA(DisplayName = "Forward Right"),
	BackwardLeft	UMETA(DisplayName = "Backward Left"),
	BackwardRight	UMETA(DisplayName = "Backward Right")
};

// 闪避设置结构体
USTRUCT(BlueprintType)
struct FDodgeSettings
{
	GENERATED_BODY()

	// 闪避距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge Settings", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float DodgeDistance = 400.0f;

	// 闪避持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge Settings", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float DodgeDuration = 0.6f;

	// 无敌帧开始时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invincibility", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InvincibilityStartTime = 0.1f;

	// 无敌帧持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invincibility", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float InvincibilityDuration = 0.4f;

	// 闪避速度曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge Settings")
	UCurveFloat* DodgeSpeedCurve;

	// 前向闪避动画蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* ForwardDodgeMontage;

	// 后向闪避动画蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* BackwardDodgeMontage;

	// 左向闪避动画蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* LeftDodgeMontage;

	// 右向闪避动画蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* RightDodgeMontage;

	// 默认构造函数
	FDodgeSettings()
	{
		DodgeDistance = 400.0f;
		DodgeDuration = 0.6f;
		InvincibilityStartTime = 0.1f;
		InvincibilityDuration = 0.4f;
		DodgeSpeedCurve = nullptr;
		ForwardDodgeMontage = nullptr;
		BackwardDodgeMontage = nullptr;
		LeftDodgeMontage = nullptr;
		RightDodgeMontage = nullptr;
	}
};

// 事件委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDodgeStarted, EDodgeDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDodgeEnded, EDodgeDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvincibilityChanged, bool, bIsInvincible);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUL_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDodgeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 核心接口函数 ====================
	
	// 根据输入方向开始闪避
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool StartDodge(const FVector& InputDirection);

	// 根据指定方向开始闪避
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool StartDodgeInDirection(EDodgeDirection Direction);

	// 结束闪避
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void EndDodge();

	// 检查是否可以闪避
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dodge")
	bool CanDodge() const;

	// 取消闪避
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void CancelDodge();

	// ==================== 无敌帧管理 ====================
	
	// 开始无敌帧
	UFUNCTION(BlueprintCallable, Category = "Invincibility")
	void StartInvincibilityFrames();

	// 结束无敌帧
	UFUNCTION(BlueprintCallable, Category = "Invincibility")
	void EndInvincibilityFrames();

	// 检查是否处于无敌状态
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Invincibility")
	bool IsInvincible() const;

	// ==================== 状态查询函数 ====================
	
	// 检查是否正在闪避
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dodge")
	bool IsDodging() const;

	// 获取最后一次闪避方向
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dodge")
	EDodgeDirection GetLastDodgeDirection() const;

	// 获取闪避进度 (0.0 - 1.0)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dodge")
	float GetDodgeProgress() const;

	// ==================== 事件委托 ====================
	
	// 闪避开始事件
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDodgeStarted OnDodgeStarted;

	// 闪避结束事件
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDodgeEnded OnDodgeEnded;

	// 无敌状态变化事件
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInvincibilityChanged OnInvincibilityChanged;

	// ==================== 配置属性 ====================
	
	// 闪避设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FDodgeSettings DodgeSettings;

	// 闪避消耗的精力值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StaminaCost = 25.0f;

	// 是否启用调试日志
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugLogs = false;

protected:
	// ==================== 组件引用 ====================
	
	// 闪避时间轴组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTimelineComponent* DodgeTimeline;

	// 精力组件引用（使用UActorComponent*避免编译错误）
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	UActorComponent* StaminaComponent;

	// ==================== 状态变量 ====================
	
	// 是否正在闪避
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDodging;

	// 是否处于无敌状态
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsInvincible;

	// 当前闪避方向
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EDodgeDirection CurrentDodgeDirection;

	// 最后一次闪避方向
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EDodgeDirection LastDodgeDirection;

	// 闪避开始位置
	FVector DodgeStartLocation;

	// 闪避目标位置
	FVector DodgeTargetLocation;

	// 闪避开始时间
	float DodgeStartTime;

	// 无敌帧计时器句柄
	FTimerHandle InvincibilityTimerHandle;

private:
	// ==================== 私有辅助函数 ====================
	
	// 更新闪避移动
	void UpdateDodgeMovement(float DeltaTime);

	// 计算闪避方向
	EDodgeDirection CalculateDodgeDirection(const FVector& InputDirection) const;

	// 根据方向获取对应的动画蒙太奇
	UAnimMontage* GetDodgeMontageForDirection(EDodgeDirection Direction) const;

	// 时间轴更新回调
	UFUNCTION()
	void OnDodgeTimelineUpdate(float Value);

	// 时间轴完成回调
	UFUNCTION()
	void OnDodgeTimelineFinished();

	// 初始化时间轴
	void InitializeTimeline();

	// 播放闪避动画
	void PlayDodgeAnimation(EDodgeDirection Direction);

	// 计算闪避目标位置
	FVector CalculateDodgeTargetLocation(EDodgeDirection Direction) const;

	// 获取方向向量
	FVector GetDirectionVector(EDodgeDirection Direction) const;

	// 检查闪避路径是否安全
	bool IsPathSafe(const FVector& StartLocation, const FVector& EndLocation) const;
};