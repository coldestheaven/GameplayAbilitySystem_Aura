// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

/**
 * 单个等级的升级信息结构体
 * 定义升级到此等级所需的 XP 和升级奖励
 */
USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	/**
	 * 升级到此等级所需的累计 XP 总量
	 * 例如：Level 2 需要 300 XP，Level 3 需要 900 XP
	 * 注意：这是累计总量，不是增量
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	/**
	 * 升级到此等级时奖励的属性点数量
	 * 玩家可用属性点提升主属性（力量、智力、韧性、活力）
	 * 默认每级奖励 1 点
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;

	/**
	 * 升级到此等级时奖励的技能点数量
	 * 玩家可用技能点解锁/升级技能
	 * 默认每级奖励 1 点
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};

/**
 * 等级升级信息数据资产
 *
 * 定义游戏中所有等级的升级要求和奖励
 * 在 PlayerState 的 Details 面板中指定
 *
 * 数组索引说明：
 * - 索引 0：占位（不使用，等级从 1 开始）
 * - 索引 1：Level 1 的信息（LevelUpRequirement=0，表示初始等级）
 * - 索引 2：Level 2 的信息（LevelUpRequirement=300，表示升到 2 级需要 300 XP）
 * - 以此类推...
 *
 * 使用方式：
 *   // 根据 XP 查找对应等级
 *   int32 Level = LevelUpInfo->FindLevelForXP(CurrentXP);
 *   // 获取升级奖励
 *   int32 AttrPoints = LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
 */
UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()
public:

	/** 所有等级的升级信息数组（索引对应等级，从 1 开始） */
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraLevelUpInfo> LevelUpInformation;

	/**
	 * 根据 XP 总量查找对应的等级
	 * 遍历 LevelUpInformation，找到 XP 达到的最高等级
	 * @param XP 当前 XP 总量
	 * @return 对应的角色等级（最小为 1）
	 */
	int32 FindLevelForXP(int32 XP) const;
};
