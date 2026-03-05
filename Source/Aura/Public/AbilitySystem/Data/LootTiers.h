// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTiers.generated.h"

/**
 * 单个战利品物品配置结构体
 * 定义一种可掉落物品的类型、概率和数量
 */
USTRUCT(BlueprintType)
struct FLootItem
{
	GENERATED_BODY()

	/** 战利品 Actor 类（如 BP_HealthPotion、BP_ManaPotion 等） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootTiers|Spawning")
	TSubclassOf<AActor> LootClass;

	/**
	 * 生成概率（百分比，0~100）
	 * 每次掉落时，此物品被选中的概率
	 * 例如：50.f 表示 50% 的概率掉落此物品
	 */
	UPROPERTY(EditAnywhere, Category = "LootTiers|Spawning")
	float ChanceToSpawn = 0.f;

	/**
	 * 最大生成数量
	 * 单次掉落时此物品的最大数量
	 * 实际数量在 1 ~ MaxNumberToSpawn 之间随机
	 */
	UPROPERTY(EditAnywhere, Category = "LootTiers|Spawning")
	int32 MaxNumberToSpawn = 0.f;

	/**
	 * 是否覆盖战利品等级
	 * true：战利品使用敌人等级（而非固定等级）
	 * false：战利品使用默认等级
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootTiers|Spawning")
	bool bLootLevelOverride = true;
};

/**
 * 战利品等级数据资产
 *
 * 定义敌人死亡时可能掉落的物品列表
 * 在 GameMode 的 Details 面板中指定，通过 UAuraAbilitySystemLibrary::GetLootTiers 全局访问
 *
 * 使用方式：
 *   // 在敌人死亡时（蓝图 SpawnLoot 事件中）
 *   TArray<FLootItem> Items = LootTiers->GetLootItems();
 *   for (FLootItem& Item : Items)
 *   {
 *       // 根据 ChanceToSpawn 随机决定是否生成
 *       // 根据 MaxNumberToSpawn 随机决定生成数量
 *   }
 */
UCLASS()
class AURA_API ULootTiers : public UDataAsset
{
	GENERATED_BODY()
public:

	/**
	 * 获取本次掉落的战利品列表（蓝图可调用）
	 * 根据每个物品的 ChanceToSpawn 随机决定是否包含在结果中
	 * @return 本次应该生成的战利品列表（已经过概率筛选）
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FLootItem> GetLootItems();
	
	/** 所有可能掉落的战利品配置列表（在 Details 面板中配置） */
	UPROPERTY(EditDefaultsOnly, Category = "LootTiers|Spawning")
	TArray<FLootItem> LootItems;
};
