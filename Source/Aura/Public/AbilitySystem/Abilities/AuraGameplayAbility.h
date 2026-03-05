// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * Aura 技能基类
 *
 * 所有 Aura 游戏技能的根基类，扩展了 UGameplayAbility 的以下功能：
 * 1. 启动输入标签（StartupInputTag）：技能赋予时自动绑定到对应输入槽
 * 2. 技能描述接口（GetDescription/GetNextLevelDescription）：供技能菜单 UI 显示
 * 3. 法力消耗/冷却时间的便捷获取函数
 *
 * 继承层次：
 *   UAuraGameplayAbility
 *   ├── UAuraDamageGameplayAbility（伤害型技能）
 *   │   ├── UAuraProjectileSpell（投射物技能）
 *   │   │   ├── UAuraFireBolt（火焰箭）
 *   │   ├── UAuraBeamSpell（光束技能）
 *   │   │   └── UElectrocute（电击）
 *   │   ├── UAuraFireBlast（火焰爆炸）
 *   │   ├── UArcaneShards（奥术碎片）
 *   │   └── UAuraMeleeAttack（近战攻击）
 *   └── UAuraSummonAbility（召唤技能）
 *   └── UAuraPassiveAbility（被动技能）
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	/**
	 * 启动输入标签
	 * 技能赋予时，此标签会被添加到技能 Spec 的 DynamicAbilityTags 中
	 * 用于将技能绑定到对应的输入槽（如 InputTag.LMB、InputTag.RMB 等）
	 * 在 Details 面板中配置，决定技能默认绑定到哪个按键
	 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

	/**
	 * 获取当前等级的技能描述文本
	 * 子类重写此函数以提供具体的技能描述（包含伤害数值、冷却时间等）
	 * 在技能菜单中显示，帮助玩家了解技能效果
	 * @param Level 要查询的技能等级
	 * @return 格式化的技能描述字符串（支持富文本标签）
	 */
	virtual FString GetDescription(int32 Level);

	/**
	 * 获取下一等级的技能描述文本
	 * 在技能菜单中显示"升级后效果"，帮助玩家决策是否消耗技能点升级
	 * @param Level 当前等级（返回 Level+1 的描述）
	 * @return 下一等级的技能描述字符串
	 */
	virtual FString GetNextLevelDescription(int32 Level);

	/**
	 * 获取技能锁定时的描述文本（静态函数）
	 * 当技能尚未解锁时显示此文本（如"需要达到 X 级才能解锁"）
	 * @param Level 解锁所需的等级
	 * @return 锁定状态的描述字符串
	 */
	static FString GetLockedDescription(int32 Level);

protected:
	/**
	 * 获取当前等级的法力消耗值
	 * 从技能的 Cost GameplayEffect 中读取 Mana 属性的修改量
	 * @param InLevel 要查询的技能等级（默认为 1）
	 * @return 法力消耗值（正数，已取绝对值）
	 */
	float GetManaCost(float InLevel = 1.f) const;

	/**
	 * 获取当前等级的冷却时间
	 * 从技能的 Cooldown GameplayEffect 中读取持续时间
	 * @param InLevel 要查询的技能等级（默认为 1）
	 * @return 冷却时间（秒）
	 */
	float GetCooldown(float InLevel = 1.f) const;
};
