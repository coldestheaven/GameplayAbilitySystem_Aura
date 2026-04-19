// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
// FTaggedMontage 定义在 CombatInterface.h 中（与战斗系统强关联）
#include "Interaction/CombatInterface.h"
#include "AnimationInterface.generated.h"

class UAnimMontage;

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI, BlueprintType)
class UAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 动画接口
 *
 * 提供角色动画查询功能，与战斗逻辑解耦
 * 所有需要被查询动画的角色（玩家和敌人）实现此接口
 *
 * 职责分离原则：
 * - 此接口只负责"动画数据查询"，不涉及伤害、死亡等战斗逻辑
 * - 与 ICombatInterface 分离，使 AI 系统、技能系统可以按需 include
 *
 * 实现此接口的类：
 * - AAuraCharacterBase（基类统一实现，玩家和敌人均继承）
 *
 * 使用示例：
 *   // 在技能中获取受击动画
 *   if (IAnimationInterface* AnimInterface = Cast<IAnimationInterface>(TargetActor))
 *   {
 *       UAnimMontage* HitMontage = AnimInterface->Execute_GetHitReactMontage(TargetActor);
 *   }
 */
class AURACORE_API IAnimationInterface
{
	GENERATED_BODY()
public:
	/**
	 * 获取受击动画蒙太奇（蓝图原生事件，蓝图可调用）
	 * 被攻击时播放的受击反应动画
	 * @return 受击动画蒙太奇
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	/**
	 * 获取所有带标签的攻击蒙太奇（蓝图原生事件，蓝图可调用）
	 * 返回角色配置的所有攻击动画及其对应的标签
	 * @return 带标签蒙太奇数组
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();

	/**
	 * 根据标签获取对应的带标签蒙太奇（蓝图原生事件，蓝图可调用）
	 * @param MontageTag 要查找的蒙太奇标签
	 * @return 对应的 FTaggedMontage 结构体
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);
};
