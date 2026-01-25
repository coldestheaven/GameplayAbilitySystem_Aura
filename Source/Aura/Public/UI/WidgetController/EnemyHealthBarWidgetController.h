// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "EnemyHealthBarWidgetController.generated.h"

class AAuraEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthPercentChangedSignature, float, HealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyNameChangedSignature, FText, EnemyName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyLevelChangedSignature, int32, Level);

/**
 * 敌人头顶 HUD Widget Controller
 * 负责管理敌人头顶显示的生命值条、名称、等级等信息
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UEnemyHealthBarWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	// 委托 - 用于通知 UI 更新
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthPercentChangedSignature OnHealthPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy Info")
	FOnEnemyNameChangedSignature OnEnemyNameChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy Info")
	FOnEnemyLevelChangedSignature OnEnemyLevelChanged;

	// 初始化并绑定属性变化回调
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// 设置敌人引用
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetEnemy(AAuraEnemy* InEnemy);

protected:
	// 属性变化回调
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;

	// 敌人引用
	UPROPERTY()
	TObjectPtr<AAuraEnemy> Enemy;
};
