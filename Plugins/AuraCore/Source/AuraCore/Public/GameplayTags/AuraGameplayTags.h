// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Aura 游戏 GameplayTag 单例
 *
 * 以单例模式管理所有原生 GameplayTag，避免在代码中硬编码字符串
 * 在 UAuraAssetManager::StartInitialLoading 中调用 InitializeNativeGameplayTags 初始化
 *
 * 使用方式：
 *   // 获取单例
 *   const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
 *   // 使用标签
 *   FGameplayTag StrengthTag = Tags.Attributes_Primary_Strength;
 *   // 在技能中检查伤害类型
 *   if (DamageType == Tags.Damage_Fire) { ... }
 *
 * Tag 命名规范：
 *   Attributes.Primary.Strength → Attributes_Primary_Strength（点替换为下划线）
 */
struct AURACORE_API FAuraGameplayTags
{
public:
	/** 获取单例实例 */
	static const FAuraGameplayTags& Get() { return GameplayTags;}

	/** 初始化所有原生 GameplayTag（在 AssetManager 启动时调用） */
	static void InitializeNativeGameplayTags();

	/* ======================== 主属性标签 ======================== */
	FGameplayTag Attributes_Primary_Strength;       // 力量
	FGameplayTag Attributes_Primary_Intelligence;   // 智力
	FGameplayTag Attributes_Primary_Resilience;     // 韧性
	FGameplayTag Attributes_Primary_Vigor;          // 活力

	/* ======================== 次属性标签 ======================== */
	FGameplayTag Attributes_Secondary_Armor;                  // 护甲值
	FGameplayTag Attributes_Secondary_ArmorPenetration;       // 护甲穿透
	FGameplayTag Attributes_Secondary_BlockChance;            // 格挡率
	FGameplayTag Attributes_Secondary_CriticalHitChance;      // 暴击率
	FGameplayTag Attributes_Secondary_CriticalHitDamage;      // 暴击伤害
	FGameplayTag Attributes_Secondary_CriticalHitResistance;  // 暴击抗性
	FGameplayTag Attributes_Secondary_HealthRegeneration;     // 生命恢复
	FGameplayTag Attributes_Secondary_ManaRegeneration;       // 法力恢复
	FGameplayTag Attributes_Secondary_MaxHealth;              // 最大生命值
	FGameplayTag Attributes_Secondary_MaxMana;                // 最大法力值
	
	/** 传入经验值元属性标签（用于 GE 写入 XP 到 AttributeSet） */
	FGameplayTag Attributes_Meta_IncomingXP;

	/* ======================== 输入标签 ======================== */
	FGameplayTag InputTag_LMB;        // 鼠标左键（主攻击/移动）
	FGameplayTag InputTag_RMB;        // 鼠标右键（副技能）
	FGameplayTag InputTag_1;          // 数字键 1（技能槽 1）
	FGameplayTag InputTag_2;          // 数字键 2（技能槽 2）
	FGameplayTag InputTag_3;          // 数字键 3（技能槽 3）
	FGameplayTag InputTag_4;          // 数字键 4（技能槽 4）
	FGameplayTag InputTag_Passive_1;  // 被动技能槽 1
	FGameplayTag InputTag_Passive_2;  // 被动技能槽 2

	/* ======================== 伤害类型标签 ======================== */
	FGameplayTag Damage;           // 伤害基础标签
	FGameplayTag Damage_Fire;      // 火焰伤害
	FGameplayTag Damage_Lightning; // 闪电伤害
	FGameplayTag Damage_Arcane;    // 奥术伤害
	FGameplayTag Damage_Physical;  // 物理伤害

	/* ======================== 抗性属性标签 ======================== */
	FGameplayTag Attributes_Resistance_Fire;      // 火焰抗性
	FGameplayTag Attributes_Resistance_Lightning; // 闪电抗性
	FGameplayTag Attributes_Resistance_Arcane;    // 奥术抗性
	FGameplayTag Attributes_Resistance_Physical;  // 物理抗性

	/* ======================== Debuff 类型标签 ======================== */
	FGameplayTag Debuff_Burn;     // 燃烧 Debuff（火焰伤害触发）
	FGameplayTag Debuff_Stun;     // 眩晕 Debuff（闪电伤害触发）
	FGameplayTag Debuff_Arcane;   // 奥术 Debuff（奥术伤害触发）
	FGameplayTag Debuff_Physical; // 物理 Debuff（物理伤害触发）

	/* ======================== Debuff 参数标签（用于 SetByCaller） ======================== */
	FGameplayTag Debuff_Chance;    // Debuff 触发概率
	FGameplayTag Debuff_Damage;    // Debuff 伤害值
	FGameplayTag Debuff_Duration;  // Debuff 持续时间
	FGameplayTag Debuff_Frequency; // Debuff 触发频率

	/* ======================== 技能标签 ======================== */
	FGameplayTag Abilities_None;    // 空技能标签（表示未选中任何技能）
	FGameplayTag Abilities_Attack;  // 普通攻击技能
	FGameplayTag Abilities_Summon;  // 召唤技能
	FGameplayTag Abilities_HitReact; // 受击反应技能

	/* ======================== 技能状态标签 ======================== */
	FGameplayTag Abilities_Status_Locked;   // 已锁定（等级不足，无法解锁）
	FGameplayTag Abilities_Status_Eligible; // 可解锁（等级足够，可消耗技能点解锁）
	FGameplayTag Abilities_Status_Unlocked; // 已解锁（可以装备到槽位）
	FGameplayTag Abilities_Status_Equipped; // 已装备（已分配到输入槽位）

	/* ======================== 技能类型标签 ======================== */
	FGameplayTag Abilities_Type_Offensive; // 主动攻击技能
	FGameplayTag Abilities_Type_Passive;   // 被动技能
	FGameplayTag Abilities_Type_None;      // 无类型

	/* ======================== 具体技能标签 ======================== */
	FGameplayTag Abilities_Fire_FireBolt;          // 火焰箭技能
	FGameplayTag Abilities_Fire_FireBlast;          // 火焰爆炸技能
	FGameplayTag Abilities_Lightning_Electrocute;  // 电击技能
	FGameplayTag Abilities_Arcane_ArcaneShards;    // 奥术碎片技能

	/* ======================== 被动技能标签 ======================== */
	FGameplayTag Abilities_Passive_HaloOfProtection; // 保护光环（减少受到的伤害）
	FGameplayTag Abilities_Passive_LifeSiphon;        // 生命虹吸（攻击时恢复生命）
	FGameplayTag Abilities_Passive_ManaSiphon;        // 法力虹吸（攻击时恢复法力）

	/* ======================== 冷却标签 ======================== */
	FGameplayTag Cooldown_Fire_FireBolt; // 火焰箭冷却

	/* ======================== 战斗插槽标签 ======================== */
	FGameplayTag CombatSocket_Weapon;     // 武器插槽（武器尖端）
	FGameplayTag CombatSocket_RightHand;  // 右手插槽
	FGameplayTag CombatSocket_LeftHand;   // 左手插槽
	FGameplayTag CombatSocket_Tail;       // 尾部插槽（特殊怪物）

	/* ======================== 攻击蒙太奇标签 ======================== */
	FGameplayTag Montage_Attack_1; // 攻击动画 1
	FGameplayTag Montage_Attack_2; // 攻击动画 2
	FGameplayTag Montage_Attack_3; // 攻击动画 3
	FGameplayTag Montage_Attack_4; // 攻击动画 4
	
	/**
	 * 伤害类型到抗性属性的映射表
	 * Key: 伤害类型标签（如 Damage_Fire）
	 * Value: 对应的抗性属性标签（如 Attributes_Resistance_Fire）
	 * 用于 ExecCalc_Damage 中查找目标的对应抗性
	 */
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;

	/**
	 * 伤害类型到 Debuff 类型的映射表
	 * Key: 伤害类型标签（如 Damage_Fire）
	 * Value: 对应的 Debuff 标签（如 Debuff_Burn）
	 * 用于 AttributeSet 中根据伤害类型创建对应的 Debuff GE
	 */
	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

	/** 受击反应效果标签（应用到目标时触发受击动画） */
	FGameplayTag Effects_HitReact;

	/* ======================== 玩家输入阻断标签 ======================== */
	FGameplayTag Player_Block_InputPressed;   // 阻断输入按下事件（眩晕时使用）
	FGameplayTag Player_Block_InputHeld;      // 阻断输入持续事件
	FGameplayTag Player_Block_InputReleased;  // 阻断输入释放事件
	FGameplayTag Player_Block_CursorTrace;    // 阻断光标追踪（眩晕时禁用鼠标交互）

	/** 火焰爆炸 GameplayCue 标签（触发火焰爆炸的视觉/音效） */
	FGameplayTag GameplayCue_FireBlast;

private:
	/** 单例实例（在 InitializeNativeGameplayTags 中初始化） */
	static FAuraGameplayTags GameplayTags;
};
