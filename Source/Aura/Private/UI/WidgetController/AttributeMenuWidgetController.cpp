// Copyright Druid Mechanics


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

/**
 * 绑定回调到依赖项（重写基类）
 * 
 * 实现流程：
 * 1. 遍历所有属性标签到属性的映射
 * 2. 为每个属性绑定变化委托：
 *    - 属性值变化时调用 BroadcastAttributeInfo 更新 UI
 * 3. 绑定属性点变化委托：
 *    - 属性点变化时广播更新
 * 
 * 使用场景：
 * - WidgetController 创建后调用
 * 
 * 注意：
 * - 所有属性（主属性、次属性、生命值、法力值等）都会绑定回调
 */
void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	check(AttributeInfo);
	
	// 为每个属性绑定变化委托
	for (auto& Pair : GetAuraAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
		[this, Pair](const FOnAttributeChangeData& Data)
		{
			BroadcastAttributeInfo(Pair.Key, Pair.Value());
		}
	);
	}
	
	// 绑定属性点变化委托
	GetAuraPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

/**
 * 广播初始值（重写基类）
 * 
 * 实现流程：
 * 1. 遍历所有属性标签到属性的映射
 * 2. 为每个属性广播初始值（BroadcastAttributeInfo）
 * 3. 广播属性点数量
 * 
 * 使用场景：
 * - WidgetController 创建后调用，确保 UI 显示正确的初始状态
 */
void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	// 广播所有属性的初始值
	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
	
	AttributePointsChangedDelegate.Broadcast(GetAuraPS()->GetAttributePoints());
}

/**
 * 升级属性（消耗属性点）
 * 
 * 实现流程：
 * 1. 调用 ASC 的 UpgradeAttribute（服务端 RPC）
 * 
 * @param AttributeTag 要升级的属性标签（如 Attributes_Primary_Strength）
 * 
 * 使用场景：
 * - 玩家点击属性菜单中的升级按钮时调用
 * 
 * 网络同步说明：
 * - 通过 ASC 的 Server RPC 在服务端执行，确保权威性
 */
void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}

/**
 * 广播属性信息（更新 UI）
 * 
 * 实现流程：
 * 1. 从 AttributeInfo 数据资产获取属性信息（名称、描述等）
 * 2. 获取属性的当前数值
 * 3. 广播属性信息委托（更新 UI 显示）
 * 
 * @param AttributeTag 属性标签
 * @param Attribute 属性定义（FGameplayAttribute）
 * 
 * 使用场景：
 * - 属性值变化时调用
 * - 初始化时调用
 */
void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
