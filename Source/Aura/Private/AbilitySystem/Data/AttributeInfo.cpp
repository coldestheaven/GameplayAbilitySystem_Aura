// Copyright Druid Mechanics


#include "AbilitySystem/Data/AttributeInfo.h"

#include "Aura/AuraLogChannels.h"

/**
 * 根据属性标签查找属性信息
 * 
 * 实现流程：
 * 1. 遍历 AttributeInformation 数组
 * 2. 查找匹配 AttributeTag 的属性信息（使用 MatchesTagExact）
 * 3. 如果找到，返回对应的属性信息
 * 4. 如果未找到且 bLogNotFound 为 true，记录错误日志
 * 5. 返回空的属性信息（默认构造）
 * 
 * @param AttributeTag 属性标签
 * @param bLogNotFound 是否在未找到时记录日志
 * @return 属性信息结构体（未找到时返回空结构体）
 * 
 * 使用场景：
 * - 在属性菜单中根据属性标签获取属性名称、描述等信息
 * - 由 AttributeMenuWidgetController 调用
 * 
 * 注意：
 * - AttributeInformation 在数据资产中配置（包含属性名称、描述等 UI 信息）
 */
FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FAuraAttributeInfo& Info : AttributeInformation)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(),*GetNameSafe(this));
	}

	return FAuraAttributeInfo();
}
