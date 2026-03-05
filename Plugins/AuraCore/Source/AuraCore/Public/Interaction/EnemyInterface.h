// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 敌人接口
 *
 * 敌人角色（AAuraEnemy）实现此接口，提供 AI 系统所需的战斗目标管理功能
 * 主要用于 AI 行为树和 BT 服务中设置/获取当前追击目标
 *
 * 使用方式（在 BT 服务中）：
 *   // 设置追击目标
 *   if (IEnemyInterface* EnemyInterface = Cast<IEnemyInterface>(ControlledPawn))
 *   {
 *       EnemyInterface->Execute_SetCombatTarget(ControlledPawn, NearestPlayer);
 *   }
 *   // 获取当前目标
 *   AActor* Target = IEnemyInterface::Execute_GetCombatTarget(ControlledPawn);
 */
class AURACORE_API IEnemyInterface
{
	GENERATED_BODY()
public:
	/**
	 * 设置当前战斗目标（蓝图可调用，蓝图原生事件）
	 * 由 AI 系统（BT 服务）在找到最近玩家时调用
	 * 设置后，行为树可以通过黑板键访问此目标
	 * @param InCombatTarget 要追击/攻击的目标 Actor（通常为玩家）
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);

	/**
	 * 获取当前战斗目标（蓝图可调用，蓝图原生事件）
	 * 由行为树任务节点调用，获取当前应该攻击的目标
	 * @return 当前战斗目标 Actor，未设置则返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;
};
