// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "GameFramework/ProjectileMovementComponent.h"

/**
 * 获取技能描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算等级对应的伤害值（从曲线获取）
 * 2. 获取法力消耗和冷却时间
 * 3. 根据等级返回不同格式的描述：
 *    - 等级 1：单发火球描述
 *    - 等级 >1：多发火球描述（数量 = Min(等级, NumProjectiles)）
 * 
 * @param Level 技能等级
 * @return 技能描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能描述
 * 
 * 注意：
 * - 伤害值通过 Damage 曲线根据等级计算
 * - 投射物数量受 NumProjectiles 限制（不会超过配置的最大数量）
 */
FString UAuraFireBolt::GetDescription(int32 Level)
{
const int32 ScaledDamage = GetDamageScalable().GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of FireBolts
			"<Default>Launches %d bolts of fire, "
			"exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, NumProjectiles),
			ScaledDamage);		
	}
}

/**
 * 获取下一等级描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算下一等级的伤害值、法力消耗、冷却时间
 * 2. 返回下一等级的技能描述（包含投射物数量和伤害）
 * 
 * @param Level 下一等级
 * @return 下一等级描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示下一等级效果
 */
FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
const int32 ScaledDamage = GetDamageScalable().GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			// Title
			"<Title>NEXT LEVEL: </>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of FireBolts
			"<Default>Launches %d bolts of fire, "
			"exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, NumProjectiles),
			ScaledDamage);
}

/**
 * 生成多个投射物（重写基类 SpawnProjectile）
 * 
 * 实现流程：
 * 1. 检查是否为服务端（仅服务端生成）
 * 2. 获取战斗插槽位置
 * 3. 计算基础旋转（从插槽指向目标）
 * 4. 如果启用 Pitch 覆盖，使用指定的 Pitch
 * 5. 计算有效投射物数量（Min(等级, NumProjectiles)）
 * 6. 使用 EvenlySpacedRotators 计算均匀分布的旋转（扇形发射）
 * 7. 遍历每个旋转：
 *    - 创建生成变换
 *    - 从对象池获取或直接生成投射物
 *    - 设置伤害效果参数
 *    - 设置追踪目标（HomingTarget 或目标位置）
 *    - 设置追踪加速度（随机范围）
 *    - 启用/禁用追踪
 *    - 如果直接生成，调用 FinishSpawning
 * 
 * @param ProjectileTargetLocation 投射物目标位置
 * @param SocketTag 战斗插槽标签
 * @param bOverridePitch 是否覆盖 Pitch 角度
 * @param PitchOverride Pitch 角度覆盖值
 * @param HomingTarget 追踪目标 Actor（可选，如果提供则追踪该 Actor）
 * 
 * 使用场景：
 * - FireBolt 技能激活时调用（生成多个火球）
 * 
 * 网络同步说明：
 * - 仅在服务端执行，投射物会自动复制到客户端
 * 
 * 注意：
 * - 投射物以扇形分布发射（ProjectileSpread 控制角度）
 * - 如果提供 HomingTarget，投射物会追踪该 Actor
 * - 否则追踪目标位置（通过 SceneComponent）
 */
void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 获取战斗插槽位置
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	
	// 计算基础旋转
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch) Rotation.Pitch = PitchOverride;
	
	// 计算有效投射物数量（受等级和配置限制）
	const FVector Forward = Rotation.Vector();
	const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
	
	// 计算均匀分布的旋转（扇形发射）
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);

	// 获取对象池子系统
	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();

	// 为每个旋转生成投射物
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

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
	
		if (!Projectile) continue;
		
		// 设置伤害效果参数
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		// 设置追踪目标
		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			// 追踪指定的 Actor
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			// 追踪目标位置（通过 SceneComponent）
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		
		// 设置追踪加速度（随机范围）
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
		
		if (!PoolSubsystem)
		{
			// 只有在直接生成时才需要调用 FinishSpawning
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
}
