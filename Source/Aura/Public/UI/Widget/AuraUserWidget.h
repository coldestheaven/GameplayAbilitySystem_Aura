// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Aura 游戏 UserWidget 基类
 *
 * 所有 Aura 游戏 UI Widget 的根基类
 * 实现了 WidgetController 的设置机制，支持 MVVM 模式
 *
 * 工作原理：
 * - 每个 Widget 持有一个 WidgetController（UObject 类型）
 * - WidgetController 负责数据处理和委托广播
 * - Widget 通过绑定 WidgetController 的委托来更新 UI 显示
 * - SetWidgetController 设置控制器后触发 WidgetControllerSet 蓝图事件
 *
 * 使用方式（蓝图子类中）：
 *   // 在 WidgetControllerSet 事件中绑定委托
 *   // 例如：
 *   UOverlayWidgetController* OWC = Cast<UOverlayWidgetController>(WidgetController);
 *   OWC->OnHealthChanged.AddDynamic(this, &UMyWidget::OnHealthChanged);
 *   OWC->BroadcastInitialValues(); // 触发初始值广播
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * 设置 Widget 控制器（蓝图可调用）
	 * 设置 WidgetController 引用后，触发 WidgetControllerSet 蓝图事件
	 * 蓝图子类在 WidgetControllerSet 中绑定控制器的委托
	 * @param InWidgetController 要设置的 WidgetController 对象
	 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	/**
	 * Widget 控制器引用（蓝图只读）
	 * 持有此 Widget 对应的 WidgetController
	 * 蓝图中通过 Cast 转换为具体类型后访问其委托
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	/**
	 * 蓝图事件：WidgetController 设置完成时调用
	 * 在蓝图子类中重写此事件，绑定 WidgetController 的委托
	 * 并调用 BroadcastInitialValues 触发初始数据广播
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
