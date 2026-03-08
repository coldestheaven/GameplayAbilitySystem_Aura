// Copyright Druid Mechanics


#include "AbilitySystem/Data/AbilityInfo.h"

#include "Aura/AuraLogChannels.h"

/**
 * 根据技能标签查找技能信息
 * 
 * 实现流程：
 * 1. 遍历 AbilityInformation 数组
 * 2. 查找匹配 AbilityTag 的技能信息
 * 3. 如果找到，返回对应的技能信息
 * 4. 如果未找到且 bLogNotFound 为 true，记录错误日志
 * 5. 返回空的技能信息（默认构造）
 * 
 * @param AbilityTag 技能标签
 * @param bLogNotFound 是否在未找到时记录日志
 * @return 技能信息结构体（未找到时返回空结构体）
 * 
 * 使用场景：
 * - 在技能菜单中根据技能标签获取技能图标、描述等信息
 * - 由 WidgetController 调用
 * 
 * 注意：
 * - AbilityInformation 在数据资产中配置（包含技能图标、名称、描述等 UI 信息）
 */
FAuraAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FAuraAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("Can't find info for AbilityTag [%s] on AbilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}

	return FAuraAbilityInfo();
}
