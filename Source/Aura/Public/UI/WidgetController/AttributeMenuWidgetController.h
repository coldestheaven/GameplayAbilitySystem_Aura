// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

class UAttributeInfo;
struct FAuraAttributeInfo;
struct FGameplayTag;

/** 属性信息变化时广播（属性菜单 Widget 绑定此委托更新属性显示） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FAuraAttributeInfo&, Info);

/**
 * 属性菜单 Widget 控制器
 *
 * 职责：
 * - 监听所有主属性（力量、智力、韧性、活力）的变化并广播给 Widget
 * - 处理玩家升级属性的请求（UpgradeAttribute）
 * - 广播属性点数量变化给 Widget
 *
 * 属性信息广播流程：
 *   1. BroadcastInitialValues：遍历所有属性，广播初始值
 *   2. BindCallbacksToDependencies：绑定属性变化回调
 *   3. 属性变化时：调用 BroadcastAttributeInfo 广播新值
 *
 * 使用示例（Widget 中）：
 *   // 绑定属性信息委托
 *   AttributeMenuWidgetController->AttributeInfoDelegate.AddDynamic(this, &UMyWidget::OnAttributeInfoChanged);
 *   // 升级力量属性
 *   AttributeMenuWidgetController->UpgradeAttribute(StrengthTag);
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	/**
	 * 绑定回调到依赖项（重写基类）
	 * 绑定所有主属性的变化回调，属性变化时广播 AttributeInfoDelegate
	 */
	virtual void BindCallbacksToDependencies() override;

	/**
	 * 广播初始属性值（重写基类）
	 * 遍历 TagsToAttributes 映射，广播每个属性的当前值和描述信息
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * 属性信息广播委托（蓝图可绑定）
	 * 当任意属性值变化时广播，携带属性的完整信息（名称、描述、当前值）
	 * Widget 绑定此委托来更新属性行的显示
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;

	/**
	 * 属性点数量变化时广播（蓝图可绑定）
	 * Widget 绑定此委托来更新可用属性点的显示
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnPlayerStatChangedSignature AttributePointsChangedDelegate;

	/**
	 * 升级指定属性（蓝图可调用）
	 * 向 ASC 发送升级请求，ASC 转发到服务端 RPC 处理
	 * @param AttributeTag 要升级的属性标签（如 Attributes.Primary.Strength）
	 */
	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

protected:
	/**
	 * 属性信息数据资产
	 * 定义每个属性的显示名称、描述文本等 UI 信息
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;

private:
	/**
	 * 广播指定属性的信息
	 * 从 AttributeInfo 数据资产中查找属性描述，结合当前属性值，通过 AttributeInfoDelegate 广播
	 * @param AttributeTag 要广播的属性标签
	 * @param Attribute    对应的 FGameplayAttribute 对象（用于获取当前值）
	 */
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
};
