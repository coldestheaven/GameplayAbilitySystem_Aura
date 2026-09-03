// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

/**
 * 属性访问器宏
 * 为每个 GameplayAttribute 自动生成以下四个函数：
 *   - Get[PropertyName]Attribute()  返回 FGameplayAttribute 对象（用于注册监听）
 *   - Get[PropertyName]()           返回属性当前值（float）
 *   - Set[PropertyName](float)      设置属性值
 *   - Init[PropertyName](float)     初始化属性基础值
 *
 * 使用示例：
 *   // 获取生命值属性对象（用于绑定变化回调）
 *   FGameplayAttribute HealthAttr = UAuraAttributeSet::GetHealthAttribute();
 *   // 获取当前生命值
 *   float CurrentHP = AttributeSet->GetHealth();
 */
// #define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
// 	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
// 	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
// 	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
// 	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	ATTRIBUTE_ACCESSORS_BASIC(ClassName, PropertyName)

/**
 * GameplayEffect 执行时的上下文属性结构体
 * 在 PostGameplayEffectExecute 中填充，包含效果来源和目标的完整信息
 *
 * 使用示例：
 *   FEffectProperties Props;
 *   SetEffectProperties(Data, Props);
 *   // 之后可通过 Props.SourceCharacter、Props.TargetASC 等访问相关对象
 */
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	/** GameplayEffect 上下文句柄（包含来源、目标、命中结果等信息） */
	FGameplayEffectContextHandle EffectContextHandle;

	/** 效果来源的 AbilitySystemComponent */
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	/** 效果来源的 Avatar Actor（通常为角色 Pawn） */
	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	/** 效果来源的控制器（玩家控制器或 AI 控制器） */
	UPROPERTY()
	AController* SourceController = nullptr;

	/** 效果来源的角色（ACharacter 类型，方便访问动画等） */
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	/** 效果目标的 AbilitySystemComponent */
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	/** 效果目标的 Avatar Actor */
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	/** 效果目标的控制器 */
	UPROPERTY()
	AController* TargetController = nullptr;

	/** 效果目标的角色 */
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 * 静态函数指针模板别名
 * 用于 TagsToAttributes 映射，将 GameplayTag 映射到返回 FGameplayAttribute 的静态函数指针
 *
 * 使用示例：
 *   // TagsToAttributes 的类型为 TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>>
 *   // 可通过 Tag 查找对应的属性：
 *   auto FuncPtr = TagsToAttributes.Find(SomeTag);
 *   if (FuncPtr) FGameplayAttribute Attr = (*FuncPtr)();
 */
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * Aura 游戏的属性集
 *
 * 包含以下四类属性：
 * 1. 主属性（Primary）：力量、智力、韧性、活力 —— 由玩家分配属性点直接提升
 * 2. 次属性（Secondary）：护甲、穿甲、格挡率、暴击率等 —— 由主属性通过 GE 计算派生
 * 3. 生命/法力（Vital）：当前生命值、最大生命值、当前法力值、最大法力值
 * 4. 抗性（Resistance）：火焰、闪电、奥术、物理抗性
 * 5. 元属性（Meta）：IncomingDamage、IncomingXP —— 临时中间值，不直接显示
 *
 * 属性变化流程：
 *   GE 应用 → PreAttributeChange（钳制值范围）
 *           → PostGameplayEffectExecute（处理伤害/XP 等特殊逻辑）
 *           → PostAttributeChange（处理生命/法力补满等后处理）
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAuraAttributeSet();

	/** 注册需要网络同步的属性（所有带 ReplicatedUsing 的属性） */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 属性变化前的钳制处理
	 * 在 GE 修改属性之前调用，用于限制属性值范围
	 * 例如：生命值不能超过最大生命值，不能低于 0
	 * @param Attribute 即将变化的属性
	 * @param NewValue  即将设置的新值（可修改此引用来钳制范围）
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * GameplayEffect 执行完成后的回调（核心伤害/治疗处理入口）
	 * 在此处理 IncomingDamage 和 IncomingXP 元属性：
	 * - IncomingDamage > 0：扣除生命值、触发 Debuff、显示伤害数字、检测死亡
	 * - IncomingXP > 0：增加玩家 XP，触发升级检测
	 * @param Data 包含 GE 修改数据和上下文信息
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/**
	 * 属性变化后的回调
	 * 用于处理最大值变化时同步当前值（如最大生命值提升时补满生命）
	 * @param Attribute 已变化的属性
	 * @param OldValue  变化前的值
	 * @param NewValue  变化后的值
	 */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/**
	 * Tag 到属性的映射表
	 * 用于通过 GameplayTag 动态查找对应的属性（如属性菜单升级时使用）
	 * Key: 属性对应的 GameplayTag（如 Attributes.Primary.Strength）
	 * Value: 返回该属性 FGameplayAttribute 对象的静态函数指针
	 */
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	/* ======================== 主属性（Primary Attributes） ======================== */

	/** 力量：影响物理伤害和护甲值 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);

	/** 智力：影响法术伤害和最大法力值 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);

	/** 韧性：影响护甲穿透抗性和格挡率 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);

	/** 活力：影响最大生命值和生命恢复速率 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);

	/* ======================== 次属性（Secondary Attributes） ======================== */

	/** 护甲值：减少受到的物理伤害，由力量和韧性派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);

	/** 护甲穿透：忽略目标护甲的百分比，由力量派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);

	/** 格挡率：有概率将伤害减半，由韧性派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);

	/** 暴击率：触发暴击的概率，由智力派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);

	/** 暴击伤害：暴击时额外增加的伤害倍率，由智力派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);

	/** 暴击抗性：降低被暴击的概率，由韧性派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResistance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);

	/** 生命恢复：每秒恢复的生命值，由活力派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);

	/** 法力恢复：每秒恢复的法力值，由智力派生 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);

	/** 最大生命值：由活力派生，决定生命值上限 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);

	/** 最大法力值：由智力派生，决定法力值上限 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);

	/* ======================== 抗性属性（Resistance Attributes） ======================== */

	/** 火焰抗性：减少受到的火焰伤害百分比（0~100） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireResistance, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, FireResistance);
	
	/** 闪电抗性：减少受到的闪电伤害百分比 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningResistance, Category = "Resistance Attributes")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, LightningResistance);
	
	/** 奥术抗性：减少受到的奥术伤害百分比 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArcaneResistance, Category = "Resistance Attributes")
	FGameplayAttributeData ArcaneResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArcaneResistance);
	
	/** 物理抗性：减少受到的物理伤害百分比 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalResistance, Category = "Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, PhysicalResistance);

	/* ======================== 生命/法力属性（Vital Attributes） ======================== */

	/** 当前生命值（0 ~ MaxHealth，降为 0 时触发死亡） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	
	/** 当前法力值（0 ~ MaxMana，释放技能时消耗） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	/* ======================== 元属性（Meta Attributes） ======================== */

	/**
	 * 传入伤害（临时中间值）
	 * 由 ExecCalc_Damage 计算后写入，在 PostGameplayEffectExecute 中消费
	 * 消费后立即清零，不参与网络同步
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);

	/**
	 * 传入经验值（临时中间值）
	 * 击杀敌人后由 GE 写入，在 PostGameplayEffectExecute 中消费并增加到 PlayerState
	 * 消费后立即清零，不参与网络同步
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingXP);

	/* ======================== 网络同步回调（OnRep） ======================== */
	// 每个属性的 OnRep 函数负责调用 GAMEPLAYATTRIBUTE_REPNOTIFY 宏，
	// 确保 GAS 内部的预测系统正确处理属性变化

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;

	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;

	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;

	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;	

	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;

	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;

	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;

	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;

private:
	/**
	 * 处理传入伤害（IncomingDamage > 0 时调用）
	 * 流程：扣除生命值 → 检测死亡 → 触发 Debuff → 显示伤害数字
	 * @param Props 包含来源和目标完整信息的上下文结构体
	 */
	void HandleIncomingDamage(const FEffectProperties& Props);

	/**
	 * 处理传入经验值（IncomingXP > 0 时调用）
	 * 流程：增加 PlayerState 的 XP → 检测升级
	 * @param Props 包含来源和目标完整信息的上下文结构体
	 */
	void HandleIncomingXP(const FEffectProperties& Props);

	/**
	 * 应用 Debuff 效果
	 * 根据 GE 上下文中的 Debuff 参数（类型、伤害、持续时间、频率）创建并应用 Debuff GE
	 * @param Props 包含来源和目标完整信息的上下文结构体
	 */
	void Debuff(const FEffectProperties& Props);

	/**
	 * 从 GE 回调数据中提取并填充 FEffectProperties 结构体
	 * @param Data GE 执行回调数据
	 * @param Props 输出参数，填充后包含来源和目标的完整信息
	 */
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;

	/**
	 * 在目标位置显示浮动伤害数字
	 * @param Props    上下文信息（用于获取目标位置）
	 * @param Damage   伤害数值
	 * @param bBlockedHit   是否为格挡命中
	 * @param bCriticalHit  是否为暴击
	 */
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;

	/**
	 * 发送 XP 获取事件（通过 GameplayEvent 通知 PlayerInterface）
	 * @param Props 上下文信息（用于获取目标角色的 ASC）
	 */
	void SendXPEvent(const FEffectProperties& Props);

	/** 标记是否需要在 PostAttributeChange 中将生命值补满（最大生命值提升时使用） */
	bool bTopOffHealth = false;

	/** 标记是否需要在 PostAttributeChange 中将法力值补满（最大法力值提升时使用） */
	bool bTopOffMana = false;
};

