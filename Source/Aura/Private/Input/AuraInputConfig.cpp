// Copyright Druid Mechanics


#include "Input/AuraInputConfig.h"

/**
 * 根据输入标签查找对应的 InputAction
 * 
 * 实现流程：
 * 1. 遍历 AbilityInputActions 数组
 * 2. 查找匹配 InputTag 的 InputAction
 * 3. 如果找到，返回对应的 InputAction
 * 4. 如果未找到且 bLogNotFound 为 true，记录错误日志
 * 5. 返回 nullptr
 * 
 * @param InputTag 输入标签（如 InputTag_LMB、InputTag_RMB 等）
 * @param bLogNotFound 是否在未找到时记录日志
 * @return InputAction 指针，未找到返回 nullptr
 * 
 * 使用场景：
 * - 绑定技能输入时查找对应的 InputAction
 * - 由 AuraInputComponent 调用
 * 
 * 注意：
 * - AbilityInputActions 在数据资产中配置（包含输入标签到 InputAction 的映射）
 */
const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FAuraInputAction& Action: AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s], on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
