// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SummonInterface.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI, BlueprintType)
class USummonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 召唤物管理接口
 *
 * 提供召唤物数量的查询和管理功能，与战斗逻辑解耦
 * 实现召唤技能的角色（玩家和部分敌人）实现此接口
 *
 * 职责分离原则：
 * - 此接口只负责"召唤物计数管理"，不涉及伤害、死亡等战斗逻辑
 * - 从 ICombatInterface 中分离，使召唤系统可以独立扩展
 *
 * 实现此接口的类：
 * - AAuraCharacterBase（基类统一实现，玩家和敌人均继承）
 *
 * 使用示例：
 *   // 在召唤技能中增加召唤物计数
 *   if (ISummonInterface* SummonInterface = Cast<ISummonInterface>(OwnerActor))
 *   {
 *       SummonInterface->Execute_IncrementMinionCount(OwnerActor, 1);
 *   }
 *   // 在召唤物死亡时减少计数
 *   ISummonInterface::Execute_IncrementMinionCount(OwnerActor, -1);
 */
class AURACORE_API ISummonInterface
{
	GENERATED_BODY()
public:
	/**
	 * 获取当前召唤物数量（蓝图原生事件，蓝图可调用）
	 * @return 当前存活的召唤物数量
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();

	/**
	 * 增加/减少召唤物计数（蓝图原生事件，蓝图可调用）
	 * @param Amount 正数增加，负数减少
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementMinionCount(int32 Amount);
};
