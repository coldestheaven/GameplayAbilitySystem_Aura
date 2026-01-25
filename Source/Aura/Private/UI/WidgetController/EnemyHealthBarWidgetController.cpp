// Copyright Druid Mechanics

#include "UI/WidgetController/EnemyHealthBarWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Character/AuraEnemy.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UEnemyHealthBarWidgetController::BroadcastInitialValues()
{
	if (!IsValid(Enemy)) return;

	const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

	// 广播初始生命值
	OnHealthChanged.Broadcast(AuraAS->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());

	// 计算并广播生命值百分比
	const float HealthPercent = AuraAS->GetMaxHealth() > 0.f 
		? AuraAS->GetHealth() / AuraAS->GetMaxHealth() 
		: 0.f;
	OnHealthPercentChanged.Broadcast(HealthPercent);

	// 广播敌人等级
	if (Enemy)
	{
		OnEnemyLevelChanged.Broadcast(Enemy->GetPlayerLevel());
	}
}

void UEnemyHealthBarWidgetController::BindCallbacksToDependencies()
{
	if (!IsValid(AbilitySystemComponent)) return;

	const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

	// 绑定生命值变化回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAS->GetHealthAttribute()
	).AddUObject(this, &UEnemyHealthBarWidgetController::HealthChanged);

	// 绑定最大生命值变化回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAS->GetMaxHealthAttribute()
	).AddUObject(this, &UEnemyHealthBarWidgetController::MaxHealthChanged);
}

void UEnemyHealthBarWidgetController::SetEnemy(AAuraEnemy* InEnemy)
{
	Enemy = InEnemy;
}

void UEnemyHealthBarWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	// 广播新的生命值
	OnHealthChanged.Broadcast(Data.NewValue);

	// 计算并广播生命值百分比
	const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);
	const float HealthPercent = AuraAS->GetMaxHealth() > 0.f 
		? Data.NewValue / AuraAS->GetMaxHealth() 
		: 0.f;
	OnHealthPercentChanged.Broadcast(HealthPercent);
}

void UEnemyHealthBarWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	// 广播新的最大生命值
	OnMaxHealthChanged.Broadcast(Data.NewValue);

	// 重新计算并广播生命值百分比
	const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);
	const float HealthPercent = Data.NewValue > 0.f 
		? AuraAS->GetHealth() / Data.NewValue 
		: 0.f;
	OnHealthPercentChanged.Broadcast(HealthPercent);
}
