// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
// EffectContext 读写功能已迁移到专用库，此处 include 以便调用方无需修改 include 路径
#include "AbilitySystem/AuraEffectContextLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class UAbilityInfo;
class UAttributeInfo;
class UAbilitySystemComponent;

/**
 * Aura 能力系统工具库（蓝图函数库）
 *
 * 提供全局静态工具函数，分为以下几类：
 * 1. 角色默认属性初始化（根据职业和等级应用 GE）
 * 2. 游戏机制工具（范围内存活玩家、最近目标、阵营判断等；实际实现位于
 *    AuraCore 插件的 UAuraGameplayMechanicsLibrary，此处为兼容转发）
 * 3. 伤害效果参数工具（构建和修改 FDamageEffectParams）
 *
 * ⚠️ 已迁出的职责（调用点经 CoreRedirects 自动重定向，无需修改）：
 * - EffectContext 读写 → UAuraEffectContextLibrary（AuraCore 插件）
 * - WidgetController 获取 → UAuraUILibrary（UI 层，关注点分离）
 *
 * 使用示例（蓝图中）：
 *   // 获取范围内存活玩家
 *   TArray<AActor*> Players;
 *   UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, Players, IgnoreActors, 500.f, Origin);
 *   // 获取 WidgetController（已迁移至 UI 库）
 *   UOverlayWidgetController* OWC = UAuraUILibrary::GetOverlayWidgetController(this);
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

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
	 * 获取 AttributeInfo 数据资产（C1 重构新增 · 2026-06-14）
	 *
	 * 统一访问入口：所有需要属性 UI 信息（名称、描述、图标）的代码可走此函数，
	 * 内部从 AAuraGameModeBase::AttributeInfo 取值，避免散落在各 WidgetController 中
	 *
	 * 兼容性：
	 * - 不影响 UAttributeMenuWidgetController::AttributeInfo 现有用法
	 * - 旧蓝图配置完全保留
	 *
	 * @param WorldContextObject 世界上下文对象
	 * @return AttributeInfo 数据资产指针，未配置时返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAttributeInfo* GetAttributeInfo(const UObject* WorldContextObject);

	/**
	 * 获取 LootTiers 数据资产（从 GameMode 获取，用于战利品掉落计算）
	 * @param WorldContextObject 世界上下文对象
	 * @return LootTiers 数据资产指针
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults", meta = (DefaultToSelf = "WorldContextObject"))
	static ULootTiers* GetLootTiers(const UObject* WorldContextObject);
	
	/* ======================== 游戏机制工具 ======================== */
	/* 注意：以下游戏机制函数均为「兼容转发」，实际实现位于 UAuraGameplayMechanicsLibrary
	   （该库为游戏机制的权威实现）。本库保留转发以维持既有 C++/蓝图调用点不变，
	   避免重复维护两份相同逻辑。 */
	
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
	 * @param Forward      基准前向方向
	 * @param Axis         旋转轴（通常为 Z 轴）
	 * @param Spread       总扩散角度（度）
	 * @param NumRotators  要生成的旋转数量
	 * @return 均匀分布在扩散角度内的旋转数组
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	/**
	 * 生成均匀分布的方向向量数组
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
	 * @param DamageEffectParams 伤害参数（引用）
	 * @param InASC              目标的 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|DamageEffect")
	static void SetTargetEffectParamsASC(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC);
};