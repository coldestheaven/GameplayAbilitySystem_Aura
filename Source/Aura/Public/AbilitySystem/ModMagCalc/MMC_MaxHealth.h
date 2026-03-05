// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * 最大生命值修改量计算类（MMC）
 *
 * 用于计算 MaxHealth 属性的修改量
 * 在次属性初始化 GE（SecondaryAttributes）中使用
 *
 * 计算公式：
 *   MaxHealth = Vigor * 2.5 + 80 + (Level - 1) * 10
 *   （具体系数在 CalculateBaseMagnitude_Implementation 中定义）
 *
 * 工作原理：
 * - 在构造函数中声明需要捕获的属性（Vigor）
 * - CalculateBaseMagnitude_Implementation 在 GE 应用时被调用
 * - 从 GE Spec 中获取施法者等级，从 ASC 中获取 Vigor 当前值
 * - 返回计算出的 MaxHealth 修改量
 *
 * 注意：此 MMC 使用 Snapshot=false，即每次 GE 重新评估时都重新计算
 * 这确保了当 Vigor 属性变化时，MaxHealth 会自动更新
 */
UCLASS()
class AURA_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_MaxHealth();

	/**
	 * 计算 MaxHealth 的基础修改量（重写基类）
	 * 根据 Vigor 属性值和角色等级计算最大生命值
	 * @param Spec GE 规格（包含施法者信息和等级）
	 * @return 计算出的 MaxHealth 修改量
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	/**
	 * Vigor 属性捕获定义
	 * 在构造函数中初始化，声明需要从目标 ASC 捕获 Vigor 属性
	 * 用于在 CalculateBaseMagnitude_Implementation 中获取 Vigor 当前值
	 */
	FGameplayEffectAttributeCaptureDefinition VigorDef;
};
