// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class UAbilityInfo;
class USpellMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;
struct FWidgetControllerParams;

/**
 * Aura 能力系统工具库（蓝图函数库）
 *
 * 提供全局静态工具函数，分为以下几类：
 * 1. Widget Controller 获取（通过 WorldContext 获取各种 WidgetController）
 * 2. 角色默认属性初始化（根据职业和等级应用 GE）
 * 3. GameplayEffect 上下文读写（自定义 EffectContext 字段的 Getter/Setter）
 * 4. 游戏机制工具（范围内存活玩家、最近目标、阵营判断等）
 * 5. 伤害效果参数工具（构建和修改 FDamageEffectParams）
 *
 * 使用示例（蓝图中）：
 *   // 获取 OverlayWidgetController
 *   UOverlayWidgetController* OWC = UAuraAbilitySystemLibrary::GetOverlayWidgetController(this);
 *   // 判断是否为暴击
 *   bool bCrit = UAuraAbilitySystemLibrary::IsCriticalHit(EffectContextHandle);
 *   // 获取范围内存活玩家
 *   TArray<AActor*> Players;
 *   UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, Players, IgnoreActors, 500.f, Origin);
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/* ======================== Widget Controller 获取 ======================== */

	/**
	 * 构建 WidgetControllerParams 并获取 AuraHUD
	 * 从 WorldContext 中获取 PlayerController、PlayerState、ASC、AttributeSet
	 * @param WorldContextObject 世界上下文对象（通常传 self）
	 * @param OutWCParams        输出：填充好的 WidgetControllerParams
	 * @param OutAuraHUD         输出：AuraHUD 引用
	 * @return true 表示成功获取所有必要对象
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD);
	
	/**
	 * 获取 OverlayWidgetController（主 HUD 控制器）
	 * @param WorldContextObject 世界上下文对象
	 * @return OverlayWidgetController 实例
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/**
	 * 获取 AttributeMenuWidgetController（属性菜单控制器）
	 * @param WorldContextObject 世界上下文对象
	 * @return AttributeMenuWidgetController 实例
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * 获取 SpellMenuWidgetController（技能菜单控制器）
	 * @param WorldContextObject 世界上下文对象
	 * @return SpellMenuWidgetController 实例
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	/* ======================== 角色默认属性初始化 ======================== */

	/**
	 * 根据职业类型和等级初始化角色默认属性
	 * 从 GameMode 的 CharacterClassInfo 数据资产中读取对应职业的 GE，并应用到 ASC
	 * @param WorldContextObject 世界上下文对象
	 * @param CharacterClass     角色职业类型
	 * @param Level              角色等级（影响 ScalableFloat 曲线取值）
	 * @param ASC                目标 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	/**
	 * 从存档数据初始化角色属性（用于加载存档时恢复玩家属性）
	 * @param WorldContextObject 世界上下文对象
	 * @param ASC                目标 AbilitySystemComponent
	 * @param SaveGame           存档数据对象
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame);
	
	/**
	 * 赋予角色初始技能（通用技能 + 职业专属技能）
	 * @param WorldContextObject 世界上下文对象
	 * @param ASC                目标 AbilitySystemComponent
	 * @param CharacterClass     角色职业类型
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	/**
	 * 获取 CharacterClassInfo 数据资产（从 GameMode 获取）
	 * @param WorldContextObject 世界上下文对象
	 * @return CharacterClassInfo 数据资产指针
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/**
	 * 获取 AbilityInfo 数据资产（从 GameMode 获取）
	 * @param WorldContextObject 世界上下文对象
	 * @return AbilityInfo 数据资产指针
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	/**
	 * 获取 LootTiers 数据资产（从 GameMode 获取，用于战利品掉落计算）
	 * @param WorldContextObject 世界上下文对象
	 * @return LootTiers 数据资产指针
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults", meta = (DefaultToSelf = "WorldContextObject"))
	static ULootTiers* GetLootTiers(const UObject* WorldContextObject);
	
	/* ======================== Effect Context Getters（读取自定义 EffectContext 字段） ======================== */

	/**
	 * 判断此次命中是否被格挡
	 * @param EffectContextHandle GE 上下文句柄
	 * @return true 表示被格挡（伤害减半）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 判断此次 GE 是否成功触发了 Debuff
	 * @param EffectContextHandle GE 上下文句柄
	 * @return true 表示 Debuff 触发成功
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取 Debuff 的伤害数值
	 * @param EffectContextHandle GE 上下文句柄
	 * @return Debuff 每次触发的伤害值
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取 Debuff 的持续时间（秒）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return Debuff 总持续时间
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取 Debuff 的触发频率（秒/次）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return Debuff 每隔多少秒触发一次
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取伤害类型标签（火焰/闪电/奥术/物理）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 伤害类型 GameplayTag
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取死亡冲量向量（击杀时施加的物理冲量）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 死亡冲量方向和大小
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取击退力向量（命中时施加的击退力）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 击退力方向和大小
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);
	
	/**
	 * 判断此次命中是否为暴击
	 * @param EffectContextHandle GE 上下文句柄
	 * @return true 表示暴击
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 判断此次伤害是否为范围伤害
	 * @param EffectContextHandle GE 上下文句柄
	 * @return true 表示范围伤害（使用内外半径衰减）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取范围伤害的内半径（内半径内受到全额伤害）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 内半径（单位：cm）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取范围伤害的外半径（外半径外不受伤害）
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 外半径（单位：cm）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取范围伤害的爆炸中心位置
	 * @param EffectContextHandle GE 上下文句柄
	 * @return 爆炸中心世界坐标
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);

	/* ======================== Effect Context Setters（写入自定义 EffectContext 字段） ======================== */

	/**
	 * 设置是否为格挡命中
	 * @param EffectContextHandle GE 上下文句柄（引用，会被修改）
	 * @param bInIsBlockedHit     true 表示格挡命中
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	/**
	 * 设置是否为暴击
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param bInIsCriticalHit    true 表示暴击
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);
	
	/**
	 * 设置是否成功触发 Debuff
	 * @param EffectContextHandle  GE 上下文句柄（引用）
	 * @param bInSuccessfulDebuff  true 表示 Debuff 触发成功
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);

	/**
	 * 设置 Debuff 伤害数值
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InDamage            Debuff 每次触发的伤害值
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDamage);

	/**
	 * 设置 Debuff 持续时间
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InDuration          Debuff 总持续时间（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDuration);

	/**
	 * 设置 Debuff 触发频率
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InFrequency         Debuff 触发间隔（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InFrequency);

	/**
	 * 设置伤害类型标签
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InDamageType        伤害类型 GameplayTag
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);

	/**
	 * 设置死亡冲量向量
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InImpulse           死亡冲量（方向 * 大小）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse);

	/**
	 * 设置击退力向量
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InForce             击退力（方向 * 大小）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce);

	/**
	 * 设置是否为范围伤害
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param bInIsRadialDamage   true 表示范围伤害
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsRadialDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage);

	/**
	 * 设置范围伤害内半径
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InInnerRadius       内半径（单位：cm）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageInnerRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius);

	/**
	 * 设置范围伤害外半径
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InOuterRadius       外半径（单位：cm）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOuterRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius);

	/**
	 * 设置范围伤害爆炸中心位置
	 * @param EffectContextHandle GE 上下文句柄（引用）
	 * @param InOrigin            爆炸中心世界坐标
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOrigin(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin);
	
	/* ======================== 游戏机制工具 ======================== */
	
	/**
	 * 获取指定球形范围内所有存活的玩家（实现了 ICombatInterface 且未死亡）
	 * @param WorldContextObject    世界上下文对象
	 * @param OutOverlappingActors  输出：范围内的存活 Actor 列表
	 * @param ActorsToIgnore        要忽略的 Actor 列表（通常包含施法者自身）
	 * @param Radius                球形范围半径（单位：cm）
	 * @param SphereOrigin          球形范围中心世界坐标
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	/**
	 * 从 Actor 列表中获取距离原点最近的 N 个目标
	 * @param MaxTargets        最多返回的目标数量
	 * @param Actors            候选 Actor 列表
	 * @param OutClosestTargets 输出：最近的 N 个 Actor
	 * @param Origin            参考原点位置
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);
	
	/**
	 * 判断两个 Actor 是否不是友方（通过比较 GameplayTag 中的阵营标签）
	 * @param FirstActor  第一个 Actor
	 * @param SecondActor 第二个 Actor
	 * @return true 表示两者不是同一阵营（可以互相攻击）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	/**
	 * 应用伤害效果（创建并应用 DamageGameplayEffect）
	 * 封装了完整的伤害应用流程，包括上下文设置和 GE 应用
	 * @param DamageEffectParams 伤害效果参数结构体（包含来源、目标、伤害类型等）
	 * @return 应用后的 GE 上下文句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	/**
	 * 生成均匀分布的旋转数组（用于多方向投射物）
	 * 例如：FireBolt 技能发射多个火球时，计算每个火球的发射方向
	 * @param Forward      基准前向方向
	 * @param Axis         旋转轴（通常为 Z 轴）
	 * @param Spread       总扩散角度（度）
	 * @param NumRotators  要生成的旋转数量
	 * @return 均匀分布在扩散角度内的旋转数组
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	/**
	 * 生成均匀分布的方向向量数组（EvenlySpacedRotators 的向量版本）
	 * @param Forward    基准前向方向
	 * @param Axis       旋转轴
	 * @param Spread     总扩散角度（度）
	 * @param NumVectors 要生成的向量数量
	 * @return 均匀分布的方向向量数组
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);

	/**
	 * 根据职业类型和等级获取击杀 XP 奖励
	 * @param WorldContextObject 世界上下文对象
	 * @param CharacterClass     被击杀角色的职业类型
	 * @param CharacterLevel     被击杀角色的等级
	 * @return XP 奖励数量
	 */
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	/* ======================== 伤害效果参数工具 ======================== */

	/**
	 * 设置伤害效果参数为范围伤害模式
	 * @param DamageEffectParams 伤害参数（引用，会被修改）
	 * @param bIsRadial          true 表示启用范围伤害
	 * @param InnerRadius        内半径（全额伤害区域）
	 * @param OuterRadius        外半径（伤害衰减到 0 的边界）
	 * @param Origin             爆炸中心位置
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static void SetIsRadialDamageEffectParam(UPARAM(ref) FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin);

	/**
	 * 设置伤害效果参数的击退方向
	 * @param DamageEffectParams    伤害参数（引用）
	 * @param KnockbackDirection    击退方向（会被归一化）
	 * @param Magnitude             击退力大小（0 表示使用默认值）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static void SetKnockbackDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude = 0.f);
	
	/**
	 * 设置伤害效果参数的死亡冲量方向
	 * @param DamageEffectParams       伤害参数（引用）
	 * @param ImpulseDirection         死亡冲量方向（会被归一化）
	 * @param Magnitude                冲量大小（0 表示使用默认值）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static void SetDeathImpulseDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude = 0.f);

	/**
	 * 设置伤害效果参数的目标 ASC
	 * 用于在蓝图中动态指定伤害目标（如追踪导弹命中时）
	 * @param DamageEffectParams 伤害参数（引用）
	 * @param InASC              目标的 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static void SetTargetEffectParamsASC(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC);
};


