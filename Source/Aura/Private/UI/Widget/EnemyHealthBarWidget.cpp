// Copyright Druid Mechanics

#include "UI/Widget/EnemyHealthBarWidget.h"
#include "UI/WidgetController/EnemyHealthBarWidgetController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEnemyHealthBarWidget::OnHealthPercentChanged_Implementation(float HealthPercent)
{
	// 更新生命值条
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}
}

void UEnemyHealthBarWidget::OnHealthValueChanged_Implementation(float Health, float MaxHealth)
{
	// 更新生命值文本
	if (HealthText)
	{
		const FText HealthTextFormat = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth));
		HealthText->SetText(HealthTextFormat);
	}
}

void UEnemyHealthBarWidget::OnEnemyNameChanged_Implementation(const FText& Name)
{
	// 更新敌人名称
	if (EnemyNameText)
	{
		EnemyNameText->SetText(Name);
	}
}

void UEnemyHealthBarWidget::OnEnemyLevelChanged_Implementation(int32 Level)
{
	// 更新等级文本
	if (LevelText)
	{
		const FText LevelTextFormat = FText::FromString(FString::Printf(TEXT("Lv.%d"), Level));
		LevelText->SetText(LevelTextFormat);
	}
}

UEnemyHealthBarWidgetController* UEnemyHealthBarWidget::GetEnemyHealthBarController() const
{
	return Cast<UEnemyHealthBarWidgetController>(WidgetController);
}
