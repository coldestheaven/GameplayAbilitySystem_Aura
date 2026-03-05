// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScalableFloat.h"
#include "Interaction/CombatInterface.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * 单个职业的默认配置结构体
 * 定义某个职业的初始属性 GE、初始技能列表和 XP 奖励曲线
 */
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	/**
	 * 主属性初始化 GameplayEffect
	 * 使用 SetByCaller 机制，根据职业和等级设置力量、智力、韧性、活力的初始值
	 * 在 CharacterClassInfo 数据资产中为每个职业单独配置
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	/**
	 * 职业专属初始技能列表
	 * 这些技能只有此职业的角色才会获得（如战士的近战攻击、法师的火焰箭等）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/**
	 * XP 奖励曲线（ScalableFloat）
	 * 击杀此职业的敌人时，玩家获得的 XP 数量
	 * 通过 CurveTable 按敌人等级缩放
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	FScalableFloat XPReward = FScalableFloat();
};

/**
 * 角色职业信息数据资产
 *
 * 存储所有职业的配置信息，是 GAS 初始化系统的核心数据资产
 *
 * 包含内容：
 * 1. 每个职业的专属配置（主属性 GE、专属技能、XP 奖励）
 * 2. 所有职业共享的配置（次属性 GE、生命/法力 GE、通用技能）
 * 3. 伤害计算系数曲线表（用于 ExecCalc_Damage）
 *
 * 使用方式：
 *   在 GameMode 的 Details 面板中指定此数据资产
 *   通过 UAuraAbilitySystemLibrary::GetCharacterClassInfo 全局访问
 *   通过 GetClassDefaultInfo(CharacterClass) 获取特定职业的配置
 *
 * 初始化流程：
 *   1. 应用 PrimaryAttributes（职业专属，使用 SetByCaller 设置主属性值）
 *   2. 应用 SecondaryAttributes（通用，由主属性派生次属性）
 *   3. 应用 VitalAttributes（通用，将生命/法力设为最大值）
 *   4. 赋予 CommonAbilities（通用技能，如受击反应）
 *   5. 赋予职业专属 StartupAbilities
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 职业信息映射表
	 * Key: 职业枚举（Warrior/Ranger/Elementalist）
	 * Value: 该职业的默认配置（主属性 GE、专属技能、XP 奖励）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
	
	/**
	 * 主属性 SetByCaller GE（通用版本）
	 * 用于从存档加载时恢复属性值（直接设置具体数值，不使用曲线）
	 * 与职业专属的 PrimaryAttributes 不同，此 GE 接受外部传入的具体数值
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;

	/**
	 * 次属性初始化 GE（所有职业共用）
	 * 根据主属性（力量/智力/韧性/活力）计算并设置次属性
	 * （护甲、暴击率、格挡率等）
	 * 使用 MMC（Modifier Magnitude Calculation）计算派生值
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	/**
	 * 次属性无限持续 GE（所有职业共用）
	 * 与 SecondaryAttributes 类似，但使用 Infinite Duration
	 * 用于需要持续监听主属性变化并实时更新次属性的场景
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes_Infinite;

	/**
	 * 生命/法力初始化 GE（所有职业共用）
	 * 将当前生命值和法力值设置为对应的最大值
	 * 在次属性初始化后应用（确保最大值已计算完毕）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;

	/**
	 * 所有职业共用的初始技能列表
	 * 包含所有角色都需要的基础技能（如受击反应、眩晕反应等）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	/**
	 * 伤害计算系数曲线表
	 * 用于 ExecCalc_Damage 中计算各属性对伤害的影响系数
	 * 每行对应一个属性（如 Armor、ArmorPenetration 等），列对应等级
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults|Damage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficients;

	/**
	 * 获取指定职业的默认配置信息
	 * @param CharacterClass 要查询的职业类型
	 * @return 对应职业的 FCharacterClassDefaultInfo 结构体
	 */
	FCharacterClassDefaultInfo GetClassDefaultInfo(ECharacterClass CharacterClass);
};
