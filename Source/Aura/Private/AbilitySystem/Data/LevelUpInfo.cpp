// Copyright Druid Mechanics


#include "AbilitySystem/Data/LevelUpInfo.h"

/**
 * 根据 XP 值查找对应等级
 * 
 * 实现流程：
 * 1. 初始化等级为 1
 * 2. 循环查找：
 *    - 检查是否超出最大等级（LevelUpInformation.Num() - 1）
 *    - 如果 XP >= 当前等级的升级需求，等级 +1，继续查找
 *    - 否则停止查找
 * 3. 返回找到的等级
 * 
 * @param XP 当前 XP 值
 * @return 对应的等级（1 到最大等级）
 * 
 * 使用场景：
 * - 根据玩家 XP 计算当前等级
 * - 在 OverlayWidgetController::OnXPChanged 中调用
 * 
 * 注意：
 * - LevelUpInformation 数组索引从 1 开始（索引 1 = 等级 1 的信息）
 * - 等级计算基于升级需求（LevelUpRequirement），XP 达到需求即升级
 */
int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// 检查是否超出最大等级（LevelUpInformation 数组大小 - 1）
		// LevelUpInformation[1] = Level 1 Information
		// LevelUpInformation[2] = Level 2 Information
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		// 如果 XP 达到当前等级的升级需求，等级 +1
		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}
	return Level;
}
