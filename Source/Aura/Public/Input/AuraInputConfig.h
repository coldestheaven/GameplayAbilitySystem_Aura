// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AuraInputConfig.generated.h"

/**
 * 单个技能输入动作配置结构体
 * 将一个 InputAction 与一个 GameplayTag 关联
 *
 * 使用示例：
 *   // 配置左键点击触发 FireBolt 技能
 *   InputAction = IA_LMB
 *   InputTag = InputTag.LMB
 */
USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()

	/** 增强输入动作资产（如 IA_LMB、IA_RMB、IA_1 等） */
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	/**
	 * 对应的 GameplayTag（如 InputTag.LMB、InputTag.RMB）
	 * 技能通过此 Tag 识别是哪个输入触发了它
	 * 与技能的 StartupInputTag 匹配时，技能会响应此输入
	 */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * Aura 输入配置数据资产
 *
 * 定义所有技能输入动作与 GameplayTag 的映射关系
 * 在 PlayerController 的 Details 面板中指定
 *
 * 使用方式：
 *   // 在 PlayerController 的 SetupInputComponent 中
 *   AuraInputComponent->BindAbilityActions(InputConfig, this,
 *       &AAuraPlayerController::AbilityInputTagPressed,
 *       &AAuraPlayerController::AbilityInputTagReleased,
 *       &AAuraPlayerController::AbilityInputTagHeld);
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:

	/**
	 * 根据 GameplayTag 查找对应的 InputAction
	 * @param InputTag     要查找的输入标签
	 * @param bLogNotFound 未找到时是否输出警告日志
	 * @return 对应的 InputAction 指针，未找到则返回 nullptr
	 */
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	/**
	 * 所有技能输入动作配置数组
	 * 每个元素将一个 InputAction 与一个 GameplayTag 关联
	 * 在 Details 面板中配置（如 LMB → InputTag.LMB）
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;
};
