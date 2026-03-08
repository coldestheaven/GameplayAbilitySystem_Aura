// Copyright Druid Mechanics

#include "UI/Widget/EnemyHealthBarWidget.h"
#include "UI/WidgetController/EnemyHealthBarWidgetController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

/**
 * 生命值百分比变化回调（蓝图可重写）
 * 
 * 实现流程：
 * 1. 更新生命值条进度（SetPercent）
 * 
 * @param HealthPercent 生命值百分比（0.0 - 1.0）
 * 
 * 使用场景：
 * - 敌人生命值变化时由 WidgetController 调用
 */
void UEnemyHealthBarWidget::OnHealthPercentChanged_Implementation(float HealthPercent)
{
	// 更新生命值条
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}
}

/**
 * 生命值数值变化回调（蓝图可重写）
 * 
 * 实现流程：
 * 1. 格式化生命值文本（"当前值 / 最大值"）
 * 2. 更新生命值文本显示
 * 
 * @param Health 当前生命值
 * @param MaxHealth 最大生命值
 * 
 * 使用场景：
 * - 敌人生命值变化时由 WidgetController 调用
 */
void UEnemyHealthBarWidget::OnHealthValueChanged_Implementation(float Health, float MaxHealth)
{
	// 更新生命值文本
	if (HealthText)
	{
		const FText HealthTextFormat = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth));
		HealthText->SetText(HealthTextFormat);
	}
}

/**
 * 敌人名称变化回调（蓝图可重写）
 * 
 * 实现流程：
 * 1. 更新敌人名称文本显示
 * 
 * @param Name 敌人名称
 * 
 * 使用场景：
 * - 敌人名称设置时由 WidgetController 调用
 */
void UEnemyHealthBarWidget::OnEnemyNameChanged_Implementation(const FText& Name)
{
	// 更新敌人名称
	if (EnemyNameText)
	{
		EnemyNameText->SetText(Name);
	}
}

/**
 * 敌人等级变化回调（蓝图可重写）
 * 
 * 实现流程：
 * 1. 格式化等级文本（"Lv.等级"）
 * 2. 更新等级文本显示
 * 
 * @param Level 敌人等级
 * 
 * 使用场景：
 * - 敌人等级设置时由 WidgetController 调用
 */
void UEnemyHealthBarWidget::OnEnemyLevelChanged_Implementation(int32 Level)
{
	// 更新等级文本
	if (LevelText)
	{
		const FText LevelTextFormat = FText::FromString(FString::Printf(TEXT("Lv.%d"), Level));
		LevelText->SetText(LevelTextFormat);
	}
}

/**
 * 获取 EnemyHealthBarWidgetController（辅助函数）
 * 
 * @return EnemyHealthBarWidgetController 指针
 */
UEnemyHealthBarWidgetController* UEnemyHealthBarWidget::GetEnemyHealthBarController() const
{
	return Cast<UEnemyHealthBarWidgetController>(WidgetController);
}
