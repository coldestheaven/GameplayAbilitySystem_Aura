// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxMana.generated.h"

/**
 * 最大法力值修改量计算类（MMC）
 *
 * 用于计算 MaxMana 属性的修改量
 * 在次属性初始化 GE（SecondaryAttributes）中使用
 *
 * 计算公式：
 *   MaxMana = Intelligence * 2.5 + 15 + (Level - 1) * 5
 *   （具体系数在 CalculateBaseMagnitude_Implementation 中定义）
 *
 * 工作原理：
 * - 在构造函数中声明需要捕获的属性（Intelligence）
 * - CalculateBaseMagnitude_Implementation 在 GE 应用时被调用
 * - 从 GE Spec 中获取施法者等级，从 ASC 中获取 Intelligence 当前值
 * - 返回计算出的 MaxMana 修改量
 *
 * 注意：与 MMC_MaxHealth 类似，使用 Snapshot=false 确保实时更新
 */
UCLASS()
class AURA_API UMMC_MaxMana : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_MaxMana();

	/**
	 * 计算 MaxMana 的基础修改量（重写基类）
	 * 根据 Intelligence 属性值和角色等级计算最大法力值
	 * @param Spec GE 规格（包含施法者信息和等级）
	 * @return 计算出的 MaxMana 修改量
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	/**
	 * Intelligence 属性捕获定义
	 * 在构造函数中初始化，声明需要从目标 ASC 捕获 Intelligence 属性
	 * 用于在 CalculateBaseMagnitude_Implementation 中获取 Intelligence 当前值
	 */
	FGameplayEffectAttributeCaptureDefinition IntDef;
};
