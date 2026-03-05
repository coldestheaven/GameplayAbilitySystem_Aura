// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "EnemyHealthBarWidgetController.generated.h"

class AAuraEnemy;

/** 生命值变化时广播（携带新的生命值绝对值） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);

/** 最大生命值变化时广播（携带新的最大生命值） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);

/** 生命值百分比变化时广播（携带 0.0~1.0 的百分比，用于进度条） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthPercentChangedSignature, float, HealthPercent);

/** 敌人名称变化时广播（携带名称文本） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyNameChangedSignature, FText, EnemyName);

/** 敌人等级变化时广播（携带等级整数） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyLevelChangedSignature, int32, Level);

/**
 * 敌人头顶血条 Widget 控制器
 *
 * 负责管理敌人头顶显示的生命值条、名称、等级等信息
 * 由 AAuraEnemy 在 BeginPlay 时创建并初始化
 *
 * 职责：
 * - 绑定 ASC 的生命值属性变化回调
 * - 广播生命值、最大生命值、生命值百分比给 Widget
 * - 广播敌人名称和等级给 Widget
 *
 * 与 OverlayWidgetController 的区别：
 * - 此控制器专用于敌人头顶 Widget，不需要 PlayerState
 * - 通过 SetEnemy 设置敌人引用，从敌人获取名称和等级
 * - 每个敌人实例都有独立的控制器实例
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UEnemyHealthBarWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	/** 生命值变化时广播（血条 Widget 绑定此委托更新数值显示） */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	/** 最大生命值变化时广播（血条 Widget 绑定此委托更新上限） */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	/** 生命值百分比变化时广播（进度条 Widget 绑定此委托更新填充比例） */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthPercentChangedSignature OnHealthPercentChanged;

	/** 敌人名称广播（名称文本 Widget 绑定此委托） */
	UPROPERTY(BlueprintAssignable, Category = "Enemy Info")
	FOnEnemyNameChangedSignature OnEnemyNameChanged;

	/** 敌人等级广播（等级文本 Widget 绑定此委托） */
	UPROPERTY(BlueprintAssignable, Category = "Enemy Info")
	FOnEnemyLevelChangedSignature OnEnemyLevelChanged;

	/**
	 * 广播初始值（重写基类）
	 * 广播当前生命值、最大生命值、生命值百分比、敌人名称和等级
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * 绑定回调到依赖项（重写基类）
	 * 绑定 ASC 的生命值和最大生命值属性变化回调
	 */
	virtual void BindCallbacksToDependencies() override;

	/**
	 * 设置敌人引用（蓝图可调用）
	 * 从敌人获取名称和等级信息，用于初始广播
	 * @param InEnemy 要关联的敌人 Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetEnemy(AAuraEnemy* InEnemy);

protected:
	/**
	 * 生命值属性变化回调
	 * 计算生命值百分比并广播 OnHealthChanged 和 OnHealthPercentChanged
	 * @param Data 属性变化数据（包含新值）
	 */
	void HealthChanged(const FOnAttributeChangeData& Data) const;

	/**
	 * 最大生命值属性变化回调
	 * 重新计算生命值百分比并广播 OnMaxHealthChanged 和 OnHealthPercentChanged
	 * @param Data 属性变化数据（包含新值）
	 */
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;

	/** 关联的敌人 Actor 引用（用于获取名称和等级） */
	UPROPERTY()
	TObjectPtr<AAuraEnemy> Enemy;
};
