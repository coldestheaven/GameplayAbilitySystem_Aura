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
 *
 * 显示在敌人头顶的 UI，包含生命值进度条、生命值数字、敌人名称和等级
 * 继承自 UAuraUserWidget，通过 EnemyHealthBarWidgetController 获取数据
 *
 * 使用方式：
 * 1. 在蓝图中继承此类，绑定 HealthBar、HealthText 等 UI 组件
 * 2. 在 WidgetControllerSet 事件中绑定控制器的委托
 * 3. 由 AAuraEnemy 在 BeginPlay 时创建并设置控制器
 *
 * 注意：
 * - HealthBar 是必须绑定的组件（BindWidget）
 * - HealthText、EnemyNameText、LevelText 是可选绑定（BindWidgetOptional）
 */
UCLASS()
class AURA_API UEnemyHealthBarWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 生命值进度条（必须绑定）
	 * 显示当前生命值百分比，由 OnHealthPercentChanged 回调更新
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/**
	 * 生命值数字文本（可选绑定）
	 * 显示当前生命值数值（如 "150/200"）
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	/**
	 * 敌人名称文本（可选绑定）
	 * 显示敌人的名称（如 "哥布林"、"骷髅战士"）
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyNameText;

	/**
	 * 敌人等级文本（可选绑定）
	 * 显示敌人的等级（如 "Lv.5"）
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

protected:
	/**
	 * 生命值百分比变化回调（蓝图可重写）
	 * 由控制器的 OnHealthPercentChanged 委托触发
	 * 在蓝图中实现进度条更新逻辑
	 * @param HealthPercent 生命值百分比（0.0~1.0）
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnHealthPercentChanged(float HealthPercent);

	/**
	 * 生命值数值变化回调（蓝图可重写）
	 * 由控制器的 OnHealthChanged 和 OnMaxHealthChanged 委托触发
	 * 在蓝图中实现生命值文本更新逻辑
	 * @param Health    当前生命值
	 * @param MaxHealth 最大生命值
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnHealthValueChanged(float Health, float MaxHealth);

	/**
	 * 敌人名称变化回调（蓝图可重写）
	 * 由控制器的 OnEnemyNameChanged 委托触发
	 * @param Name 敌人名称文本
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnEnemyNameChanged(const FText& Name);

	/**
	 * 敌人等级变化回调（蓝图可重写）
	 * 由控制器的 OnEnemyLevelChanged 委托触发
	 * @param Level 敌人等级
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
	void OnEnemyLevelChanged(int32 Level);

	/**
	 * 获取敌人血条 Widget 控制器（蓝图可调用）
	 * 将基类的 WidgetController 转换为 UEnemyHealthBarWidgetController 类型
	 * @return 类型转换后的控制器指针
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy HUD")
	UEnemyHealthBarWidgetController* GetEnemyHealthBarController() const;
};
