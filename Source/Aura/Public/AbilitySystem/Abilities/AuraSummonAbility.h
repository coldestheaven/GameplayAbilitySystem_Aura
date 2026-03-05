// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 召唤技能基类
 *
 * 用于召唤小兵/随从的技能基类
 * 负责计算召唤位置并随机选择召唤物类型
 *
 * 功能：
 * - 在施法者前方的扇形区域内计算 NumMinions 个均匀分布的生成位置
 * - 生成位置会通过射线检测对齐到地面
 * - 随机从 MinionClasses 数组中选择召唤物类型
 * - 通过 ICombatInterface::IncremenetMinionCount 追踪召唤物数量
 *
 * 使用示例（蓝图中）：
 *   // 获取生成位置
 *   TArray<FVector> Locations = GetSpawnLocations();
 *   // 随机获取召唤物类型
 *   TSubclassOf<APawn> MinionClass = GetRandomMinionClass();
 *   // 在每个位置生成召唤物
 *   for (FVector Loc : Locations) { SpawnActor(MinionClass, Loc); }
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:

	/**
	 * 获取召唤物的生成位置数组（蓝图可调用）
	 * 在施法者前方的扇形区域内计算 NumMinions 个均匀分布的位置
	 * 每个位置通过向下射线检测对齐到地面
	 * @return 世界坐标的生成位置数组（数量等于 NumMinions）
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();
	
	/**
	 * 随机获取一个召唤物类（蓝图纯函数）
	 * 从 MinionClasses 数组中随机选择一个类型
	 * @return 随机选中的召唤物 Pawn 类
	 */
	UFUNCTION(BlueprintPure, Category="Summoning")
	TSubclassOf<APawn> GetRandomMinionClass();
	
	/**
	 * 召唤物数量
	 * 每次激活技能时召唤的小兵数量
	 * 默认 5 个
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	int32 NumMinions = 5;

	/**
	 * 可召唤的小兵类数组
	 * 每次召唤时从此数组中随机选择一个类型
	 * 在 Details 面板中配置（如骷髅战士、骷髅弓箭手等）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	/**
	 * 最小生成距离（单位：cm）
	 * 召唤物生成位置距施法者的最小距离
	 * 默认 50 cm
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MinSpawnDistance = 50.f;

	/**
	 * 最大生成距离（单位：cm）
	 * 召唤物生成位置距施法者的最大距离
	 * 默认 250 cm
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MaxSpawnDistance = 250.f;

	/**
	 * 生成扩散角度（度）
	 * 召唤物在施法者前方此角度范围内均匀分布
	 * 默认 90 度（左右各 45 度）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float SpawnSpread = 90.f;
};
