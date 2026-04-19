// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraGameplayMechanicsLibrary.generated.h"

struct FDamageEffectParams;

/**
 * Aura 游戏机制工具库
 *
 * 专门负责游戏机制相关的静态工具函数，从 UAuraAbilitySystemLibrary 中拆分出来：
 * - 范围查询（球形范围内存活玩家、最近目标）
 * - 阵营判断（IsNotFriend）
 * - 伤害效果应用（ApplyDamageEffect）
 * - 投射物方向计算（EvenlySpacedRotators / EvenlyRotatedVectors）
 * - 伤害效果参数工具（SetIsRadialDamageEffectParam 等）
 *
 * 使用示例：
 *   TArray<AActor*> Players;
 *   UAuraGameplayMechanicsLibrary::GetLivePlayersWithinRadius(this, Players, Ignore, 500.f, Origin);
 *   bool bEnemy = UAuraGameplayMechanicsLibrary::IsNotFriend(ActorA, ActorB);
 */
UCLASS()
class AURA_API UAuraGameplayMechanicsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/* ======================== 范围查询 ======================== */

	/**
	 * 获取指定球形范围内所有存活的玩家（实现了 ICombatInterface 且未死亡）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	/**
	 * 从 Actor 列表中获取距离原点最近的 N 个目标
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|GameplayMechanics")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	/* ======================== 阵营判断 ======================== */

	/**
	 * 判断两个 Actor 是否不是友方（通过比较 GameplayTag 中的阵营标签）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraGameplayMechanicsLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	/* ======================== 伤害效果应用 ======================== */

	/**
	 * 应用伤害效果（创建并应用 DamageGameplayEffect）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	/* ======================== 投射物方向计算 ======================== */

	/**
	 * 生成均匀分布的旋转数组（用于多方向投射物）
	 */
	UFUNCTION(BlueprintPure, Category = "AuraGameplayMechanicsLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	/**
	 * 生成均匀分布的方向向量数组
	 */
	UFUNCTION(BlueprintPure, Category = "AuraGameplayMechanicsLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);

	/**
	 * 根据职业类型和等级获取击杀 XP 奖励
	 */
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	/* ======================== 伤害效果参数工具 ======================== */

	/**
	 * 设置伤害效果参数为范围伤害模式
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|DamageEffect")
	static void SetIsRadialDamageEffectParam(UPARAM(ref) FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin);

	/**
	 * 设置伤害效果参数的击退方向
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|DamageEffect")
	static void SetKnockbackDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude = 0.f);

	/**
	 * 设置伤害效果参数的死亡冲量方向
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|DamageEffect")
	static void SetDeathImpulseDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude = 0.f);

	/**
	 * 设置伤害效果参数的目标 ASC
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraGameplayMechanicsLibrary|DamageEffect")
	static void SetTargetEffectParamsASC(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC);
};
