// Copyright Druid Mechanics


#include "UI/Widget/AuraUserWidget.h"

/**
 * 设置 WidgetController（基类实现）
 * 
 * 实现流程：
 * 1. 保存 WidgetController 引用
 * 2. 调用 WidgetControllerSet（蓝图可重写事件）
 * 
 * @param InWidgetController WidgetController 对象（通常是 WidgetController 子类）
 * 
 * 使用场景：
 * - Widget 初始化时由 HUD 调用
 * - 在 AuraHUD::InitOverlay 中调用
 * 
 * 注意：
 * - WidgetControllerSet 是蓝图事件，可以在蓝图中实现额外逻辑
 * - WidgetController 提供数据绑定和回调功能（MVVM 模式）
 */
void UAuraUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
