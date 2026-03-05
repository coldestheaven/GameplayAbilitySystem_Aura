// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;
class UGameplayEffect;
struct FGameplayTag;

/**
 * 投射物技能基类
 *
 * 所有发射投射物的技能均继承自此类（如火焰箭、奥术导弹等）
 *
 * 功能：
 * - 在技能激活时播放攻击蒙太奇
 * - 监听蒙太奇的 GameplayEvent（如 Montage.Attack.Weapon）
 * - 在事件触发时调用 SpawnProjectile 生成投射物
 * - 支持多发投射物（NumProjectiles）
 *
 * 技能激活流程：
 *   ActivateAbility → 播放攻击蒙太奇 → 等待 GameplayEvent
 *   → SpawnProjectile（在武器插槽位置生成投射物）→ 结束技能
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

protected:
	/**
	 * 技能激活（重写基类）
	 * 播放攻击蒙太奇，等待 GameplayEvent 触发后生成投射物
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 在指定插槽位置生成投射物（蓝图可调用）
	 * 在服务端生成 ProjectileClass 类型的投射物，设置伤害参数后发射
	 * @param ProjectileTargetLocation 投射物的目标位置（鼠标点击位置）
	 * @param SocketTag                生成位置的插槽标签（如武器尖端、左手等）
	 * @param bOverridePitch           是否覆盖俯仰角
	 * @param PitchOverride            覆盖的俯仰角（度）
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0.f);

	/**
	 * 投射物 Actor 类
	 * 在 Details 面板中指定要生成的投射物类型（如 BP_FireBolt、BP_ArcaneShards 等）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAuraProjectile> ProjectileClass;

	/**
	 * 投射物数量
	 * 每次激活技能时生成的投射物数量
	 * 默认 5 发，子类可在 Details 面板中调整
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 5;
};
