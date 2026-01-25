// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UEnemyHealthBarWidgetController;

/**
 * 敌人头顶生命值条 Widget
 * 用于在敌人头顶显示生命值、名称、等级等信息
 * 
 * 使用方法：
 * 1. 在蓝图中继承此类
 * 2. 绑定 HealthBar、HealthText、EnemyNameText、LevelText 等组件
 * 3. 在 WidgetControllerSet 事件中绑定委托
 */
UCLASS()
class AURA_API UEnemyHealthBarWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	// 蓝图可绑定的 UI 组件
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

protected:
	// 属性变化回调 - 可在蓝图中重写
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnHealthPercentChanged(float HealthPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnHealthValueChanged(float Health, float MaxHealth);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnEnemyNameChanged(const FText& Name);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnEnemyLevelChanged(int32 Level);

	// 获取 Widget Controller
	UFUNCTION(BlueprintCallable, Category = "Enemy HUD")
	UEnemyHealthBarWidgetController* GetEnemyHealthBarController() const;
};
