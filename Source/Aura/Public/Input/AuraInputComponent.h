// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AuraInputConfig.h"
#include "AuraInputComponent.generated.h"

/**
 * Aura 增强输入组件
 *
 * 继承自 UEnhancedInputComponent，扩展了技能输入绑定功能
 *
 * 核心功能：
 * - 提供 BindAbilityActions 模板函数，批量绑定技能输入动作
 * - 将 InputAction 的三种触发事件（Started/Completed/Triggered）
 *   分别映射到 Pressed/Released/Held 三个回调函数
 * - 每个 InputAction 绑定时携带对应的 GameplayTag，
 *   回调函数通过 Tag 识别是哪个技能的输入
 *
 * 使用示例（在 PlayerController 的 SetupInputComponent 中）：
 *   UAuraInputComponent* AuraIC = CastChecked<UAuraInputComponent>(InputComponent);
 *   AuraIC->BindAbilityActions(
 *       InputConfig,           // 输入配置数据资产
 *       this,                  // 回调对象
 *       &AAuraPlayerController::AbilityInputTagPressed,   // 按下回调
 *       &AAuraPlayerController::AbilityInputTagReleased,  // 释放回调
 *       &AAuraPlayerController::AbilityInputTagHeld       // 持续按住回调
 *   );
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	/**
	 * 批量绑定技能输入动作
	 * 遍历 InputConfig 中的所有 AbilityInputActions，为每个有效的 InputAction 绑定三种事件
	 *
	 * @tparam UserClass       回调函数所属的类类型
	 * @tparam PressedFuncType 按下回调函数类型（接受 FGameplayTag 参数）
	 * @tparam ReleasedFuncType 释放回调函数类型（接受 FGameplayTag 参数）
	 * @tparam HeldFuncType    持续按住回调函数类型（接受 FGameplayTag 参数）
	 *
	 * @param InputConfig   技能输入配置数据资产（包含 InputAction 和对应的 GameplayTag）
	 * @param Object        回调函数所属的对象实例
	 * @param PressedFunc   按下时的回调函数（ETriggerEvent::Started，可为 nullptr）
	 * @param ReleasedFunc  释放时的回调函数（ETriggerEvent::Completed，可为 nullptr）
	 * @param HeldFunc      持续按住时的回调函数（ETriggerEvent::Triggered，可为 nullptr）
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UAuraInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	// 确保 InputConfig 有效
	check(InputConfig);

	// 遍历所有技能输入动作配置
	for (const FAuraInputAction& Action : InputConfig->AbilityInputActions)
	{
		// 只处理有效的 InputAction 和 InputTag
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// 绑定按下事件（ETriggerEvent::Started）
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			// 绑定释放事件（ETriggerEvent::Completed）
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}
			
			// 绑定持续按住事件（ETriggerEvent::Triggered，每帧触发）
			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
