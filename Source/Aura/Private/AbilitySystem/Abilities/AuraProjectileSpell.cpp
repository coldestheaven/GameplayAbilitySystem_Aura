// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "Interaction/CombatInterface.h"

/**
 * 激活技能（重写基类）
 * 
 * 实现流程：
 * 1. 调用父类 ActivateAbility
 * 
 * 使用场景：
 * - 技能激活时自动调用
 * 
 * 注意：
 * - 此函数为空实现，实际逻辑在子类的 SpawnProjectile 中
 * - 子类应在 ActivateAbility 中调用 SpawnProjectile
 */
void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

/**
 * 生成投射物
 * 
 * 实现流程：
 * 1. 检查是否为服务端（仅服务端生成）
 * 2. 获取战斗插槽位置（通过 CombatInterface）
 * 3. 计算旋转（从插槽位置指向目标位置）
 * 4. 如果启用 Pitch 覆盖，使用指定的 Pitch 角度
 * 5. 创建生成变换（位置和旋转）
 * 6. 尝试从对象池获取投射物：
 *    - 如果对象池可用，使用对象池获取（提高性能）
 *    - 否则直接生成（降级方案）
 * 7. 设置投射物的伤害效果参数
 * 8. 如果直接生成，调用 FinishSpawning
 * 
 * @param ProjectileTargetLocation 投射物目标位置（鼠标位置或目标位置）
 * @param SocketTag 战斗插槽标签（如 Weapon、RightHand 等）
 * @param bOverridePitch 是否覆盖 Pitch 角度
 * @param PitchOverride Pitch 角度覆盖值（度）
 * 
 * 使用场景：
 * - 投射物技能激活时调用
 * - 由子类的 ActivateAbility 调用
 * 
 * 网络同步说明：
 * - 仅在服务端执行，投射物会自动复制到客户端
 * - 投射物使用对象池管理，提高性能
 * 
 * 注意：
 * - 投射物的伤害参数通过 MakeDamageEffectParamsFromClassDefaults 设置
 * - 对象池中的投射物不需要调用 FinishSpawning
 */
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 获取战斗插槽位置（武器或手部位置）
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	
	// 计算旋转（从插槽位置指向目标位置）
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}
	
	// 创建生成变换
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());
	
	// 从对象池获取投射物
	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	AAuraProjectile* Projectile = nullptr;
	
	if (PoolSubsystem)
	{
		// 使用对象池（提高性能）
		Projectile = Cast<AAuraProjectile>(PoolSubsystem->AcquireActor(ProjectileClass, SpawnTransform));
	}
	else
	{
		// 降级方案：直接生成
		Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	}
	
	if (Projectile)
	{
		// 设置伤害效果参数
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		if (!PoolSubsystem)
		{
			// 只有在直接生成时才需要调用 FinishSpawning
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
}
