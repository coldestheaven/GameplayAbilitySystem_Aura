// Copyright Druid Mechanics


#include "AbilitySystem/Data/LootTiers.h"

/**
 * 获取掉落物品列表（根据概率随机生成）
 * 
 * 实现流程：
 * 1. 遍历配置的掉落物品列表（LootItems）
 * 2. 对每个物品，尝试生成 MaxNumberToSpawn 次：
 *    - 随机生成 1-100 的数值
 *    - 如果随机值 < ChanceToSpawn，生成该物品
 *    - 复制物品类（LootClass）和等级覆盖标志（bLootLevelOverride）
 *    - 添加到返回列表
 * 3. 返回生成的掉落物品列表
 * 
 * @return 生成的掉落物品数组
 * 
 * 使用场景：
 * - 敌人死亡时生成掉落物
 * - 由 AuraEnemy::SpawnLoot 调用
 * 
 * 注意：
 * - 每个物品可以生成多次（最多 MaxNumberToSpawn 次）
 * - 每次生成都独立判定概率（ChanceToSpawn）
 * - 返回的物品数量可能为 0（如果所有概率判定都失败）
 */
TArray<FLootItem> ULootTiers::GetLootItems()
{
	TArray<FLootItem> ReturnItems;

	// 遍历配置的掉落物品列表
	for (FLootItem& Item : LootItems)
	{
		// 尝试生成 MaxNumberToSpawn 次
		for (int32 i = 0; i < Item.MaxNumberToSpawn; ++i)
		{
			// 根据概率判定是否生成
			if (FMath::FRandRange(1.f, 100.f) < Item.ChanceToSpawn)
			{
				FLootItem NewItem;
				NewItem.LootClass = Item.LootClass;
				NewItem.bLootLevelOverride = Item.bLootLevelOverride;
				ReturnItems.Add(NewItem);
			}
		}
	}

	return ReturnItems;
}
