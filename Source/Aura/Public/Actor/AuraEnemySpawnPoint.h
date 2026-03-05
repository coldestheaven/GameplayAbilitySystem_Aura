// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Engine/TargetPoint.h"
#include "AuraEnemySpawnPoint.generated.h"

class AAuraEnemy;

/**
 * 敌人生成点 Actor
 *
 * 继承自 ATargetPoint，在关卡中标记敌人的生成位置
 * 由 AAuraEnemySpawnVolume 管理，当玩家进入触发区域时调用 SpawnEnemy
 *
 * 功能：
 * - 在自身位置生成指定类型和等级的敌人
 * - 设置敌人的职业类型（影响属性和技能）
 * - 设置敌人的等级（影响属性数值和 XP 奖励）
 *
 * 使用方式：
 *   在关卡中放置此 Actor，配置 EnemyClass、EnemyLevel、CharacterClass
 *   将此 Actor 添加到 AAuraEnemySpawnVolume 的 SpawnPoints 数组中
 */
UCLASS()
class AURA_API AAuraEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
public:

	/**
	 * 在此生成点位置生成敌人（蓝图可调用）
	 * 使用 SpawnActor 在自身位置生成 EnemyClass 类型的敌人
	 * 生成后设置敌人的等级和职业类型
	 */
	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

	/**
	 * 要生成的敌人类
	 * 在 Details 面板中指定（如 BP_Goblin、BP_Skeleton 等）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	TSubclassOf<AAuraEnemy> EnemyClass;

	/**
	 * 敌人等级
	 * 影响敌人的属性数值（通过 ScalableFloat 曲线）和击杀 XP 奖励
	 * 默认 1 级
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	int32 EnemyLevel = 1;

	/**
	 * 敌人职业类型
	 * 决定敌人使用哪套属性曲线和初始技能
	 * 默认为战士（Warrior）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
};
