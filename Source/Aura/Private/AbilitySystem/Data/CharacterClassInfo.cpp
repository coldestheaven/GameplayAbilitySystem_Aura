// Copyright Druid Mechanics


#include "AbilitySystem/Data/CharacterClassInfo.h"

/**
 * 获取职业的默认信息
 * 
 * 实现流程：
 * 1. 从 CharacterClassInformation 映射中查找对应职业的信息
 * 2. 使用 FindChecked（如果未找到会崩溃，确保数据完整性）
 * 3. 返回职业默认信息（包含初始属性、技能、XP 奖励等）
 * 
 * @param CharacterClass 角色职业类型
 * @return 职业默认信息结构体
 * 
 * 使用场景：
 * - 初始化角色属性时获取职业配置
 * - 在 AuraAbilitySystemLibrary::InitializeDefaultAttributes 中调用
 * 
 * 注意：
 * - FindChecked 会在未找到时崩溃，确保数据资产配置完整
 * - CharacterClassInformation 在数据资产中配置（包含所有职业的初始数据）
 */
FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(ECharacterClass CharacterClass)
{
	return CharacterClassInformation.FindChecked(CharacterClass);
}
