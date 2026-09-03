// Copyright Druid Mechanics


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraFactionTypes.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * 构造函数：初始化效果 Actor
 * 
 * 实现流程：
 * 1. 禁用 Tick（默认不启用，需要时手动启用）
 * 2. 创建根组件（SceneComponent）
 * 
 * 使用场景：
 * - 效果 Actor（如宝箱、药水等）在关卡中放置时构造
 */
AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

/**
 * 每帧更新：处理物品动画（旋转、正弦运动）
 * 
 * 实现流程：
 * 1. 累加运行时间
 * 2. 计算正弦周期，如果超过周期则重置时间
 * 3. 调用 ItemMovement 更新位置和旋转
 * 
 * @param DeltaTime 帧时间间隔
 * 
 * 使用场景：
 * - 需要物品动画时启用 Tick（如宝箱旋转、药水上下浮动）
 */
void AAuraEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RunningTime += DeltaTime;
	const float SinePeriod = 2 * PI / SinePeriodConstant;
	if (RunningTime > SinePeriod)
	{
		RunningTime = 0.f;
	}
	ItemMovement(DeltaTime);
}

/**
 * 物品动画：旋转和正弦运动
 * 
 * 实现流程：
 * 1. 如果启用旋转：累加旋转角度（Yaw 轴）
 * 2. 如果启用正弦运动：计算正弦值，更新 Z 轴位置
 * 
 * @param DeltaTime 帧时间间隔
 * 
 * 使用场景：
 * - 在 Tick 中调用，实现物品的视觉效果
 * 
 * 注意：
 * - CalculatedRotation 和 CalculatedLocation 需要在蓝图中应用到 Actor
 */
void AAuraEffectActor::ItemMovement(float DeltaTime)
{
	if (bRotates)
	{
		const FRotator DeltaRotation(0.f, DeltaTime * RotationRate, 0.f);
		CalculatedRotation = UKismetMathLibrary::ComposeRotators(CalculatedRotation, DeltaRotation);
	}
	if (bSinusoidalMovement)
	{
		const float Sine = SineAmplitude * FMath::Sin(RunningTime * SinePeriodConstant);
		CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);
	}
}


/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 记录初始位置和旋转（用于动画计算）
 * 
 * 使用场景：
 * - 效果 Actor 生成后自动调用
 */
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation;
	CalculatedRotation = GetActorRotation();
}

/**
 * 开始正弦运动（上下浮动）
 * 
 * 实现流程：
 * 1. 启用正弦运动标志
 * 2. 更新初始位置（从当前位置开始）
 * 
 * 使用场景：
 * - 需要物品上下浮动效果时调用（如药水）
 */
void AAuraEffectActor::StartSinusoidalMovement()
{
	bSinusoidalMovement = true;
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation;
}

/**
 * 开始旋转动画
 * 
 * 实现流程：
 * 1. 启用旋转标志
 * 2. 记录当前旋转（作为起始旋转）
 * 
 * 使用场景：
 * - 需要物品旋转效果时调用（如宝箱）
 */
void AAuraEffectActor::StartRotation()
{
	bRotates = true;
	CalculatedRotation = GetActorRotation();
}

/**
 * 将 GameplayEffect 应用到目标 Actor
 * 
 * 实现流程：
 * 1. 检查是否允许应用到敌人（如果目标是敌人且 bApplyEffectsToEnemies=false，则返回）
 * 2. 获取目标的 ASC
 * 3. 创建 EffectContext，设置 SourceObject 为自身
 * 4. 创建 EffectSpec（使用 ActorLevel）
 * 5. 应用 GE 到目标
 * 6. 如果是 Infinite GE 且移除策略为 RemoveOnEndOverlap，保存 EffectHandle
 * 7. 如果不是 Infinite GE，销毁自身（一次性效果）
 * 
 * @param TargetActor 目标 Actor（要应用效果的 Actor）
 * @param GameplayEffectClass 要应用的 GE 类
 * 
 * 使用场景：
 * - 玩家与效果 Actor 重叠时应用效果（如拾取药水、宝箱）
 * 
 * 注意：
 * - Infinite GE 会在重叠结束时移除（通过保存的 EffectHandle）
 * - 一次性效果（非 Infinite）应用后立即销毁 Actor
 */
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (TargetActor->ActorHasTag(AuraFaction::Enemy()) && !bApplyEffectsToEnemies) return;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// 如果是 Infinite GE 且移除策略为 RemoveOnEndOverlap，保存 EffectHandle（用于重叠结束时移除）
	const bool bIsInfinite =  EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

	// 如果不是 Infinite GE，应用后立即销毁（一次性效果）
	if (!bIsInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(AuraFaction::Enemy()) && !bApplyEffectsToEnemies) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(AuraFaction::Enemy()) && !bApplyEffectsToEnemies) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;

		TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}


