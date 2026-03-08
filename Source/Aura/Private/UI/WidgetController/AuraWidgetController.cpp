// Copyright Druid Mechanics


#include "UI/WidgetController/AuraWidgetController.h"

#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"

/**
 * 设置 WidgetController 参数（基类实现）
 * 
 * 实现流程：
 * 1. 从参数结构体中提取并保存所有必要的引用
 * 
 * @param WCParams WidgetController 参数结构体（包含 PC、PS、ASC、AS）
 * 
 * 使用场景：
 * - WidgetController 创建后立即调用
 * - 在 HUD 的 GetXXXWidgetController 中调用
 */
void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

/**
 * 广播初始值（基类空实现，子类重写）
 * 
 * 使用场景：
 * - WidgetController 创建后调用，确保 UI 显示正确的初始状态
 * - 在绑定回调之前调用，避免初始值丢失
 */
void UAuraWidgetController::BroadcastInitialValues()
{
	
}

/**
 * 绑定回调到依赖项（基类空实现，子类重写）
 * 
 * 使用场景：
 * - WidgetController 创建后调用
 * - 绑定属性变化、技能变化等回调
 */
void UAuraWidgetController::BindCallbacksToDependencies()
{
	
}

/**
 * 广播所有技能信息
 * 
 * 实现流程：
 * 1. 检查技能是否已赋予（bStartupAbilitiesGiven）
 * 2. 遍历所有可激活技能
 * 3. 为每个技能：
 *    - 从 AbilityInfo 数据资产获取技能信息
 *    - 设置输入标签和状态标签
 *    - 广播 AbilityInfoDelegate
 * 
 * 使用场景：
 * - 技能菜单初始化时显示所有技能
 * - 技能状态变化后更新 UI
 * 
 * 注意：
 * - 只有技能已赋予后才会广播（避免空数据）
 * - 广播的信息包含技能的图标、描述、状态等 UI 所需数据
 */
void UAuraWidgetController::BroadcastAbilityInfo()
{
	if (!GetAuraASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		Info.StatusTag = AuraAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	GetAuraASC()->ForEachAbility(BroadcastDelegate);
}

AAuraPlayerController* UAuraWidgetController::GetAuraPC()
{
	if (AuraPlayerController == nullptr)
	{
		AuraPlayerController = Cast<AAuraPlayerController>(PlayerController);
	}
	return AuraPlayerController;
}

AAuraPlayerState* UAuraWidgetController::GetAuraPS()
{
	if (AuraPlayerState == nullptr)
	{
		AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	}
	return AuraPlayerState;
}

UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	}
	return AuraAbilitySystemComponent;
}

UAuraAttributeSet* UAuraWidgetController::GetAuraAS()
{
	if (AuraAttributeSet == nullptr)
	{
		AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	}
	return AuraAttributeSet;
}
