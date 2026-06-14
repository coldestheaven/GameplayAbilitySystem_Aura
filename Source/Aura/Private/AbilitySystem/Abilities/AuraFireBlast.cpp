// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraFireBlast.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

/**
 * 获取技能描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算等级对应的伤害值、法力消耗、冷却时间
 * 2. 返回技能描述（包含火球数量和伤害）
 * 
 * @param Level 技能等级
 * @return 技能描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能描述
 * 
 * 注意：
 * - FireBlast 会向所有方向发射火球，火球会返回并爆炸
 * - 火球数量固定为 NumFireBalls（不受等级影响）
 */
FString UAuraFireBlast::GetDescription(int32 Level)
{
const int32 ScaledDamage = GetDamageScalable().GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BLAST</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of Fire Balls
			"<Default>Launches %d </>"
			"<Default>fire balls in all directions, each coming back and </>"
			"<Default>exploding upon return, causing </>"

			// Damage
			"<Damage>%d</><Default> radial fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			NumFireBalls,
			ScaledDamage);
}

/**
 * 获取下一等级描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算下一等级的伤害值、法力消耗、冷却时间
 * 2. 返回下一等级的技能描述
 * 
 * @param Level 下一等级
 * @return 下一等级描述字符串（包含富文本格式）
 */
FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
const int32 ScaledDamage = GetDamageScalable().GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			// Title
			"<Title>NEXT LEVEL:</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of Fire Balls
			"<Default>Launches %d </>"
			"<Default>fire balls in all directions, each coming back and </>"
			"<Default>exploding upon return, causing </>"

			// Damage
			"<Damage>%d</><Default> radial fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			NumFireBalls,
			ScaledDamage);
}

/**
 * 生成火球（向所有方向发射）
 * 
 * 实现流程：
 * 1. 获取角色前进方向和位置
 * 2. 使用 EvenlySpacedRotators 计算均匀分布的旋转（360度，NumFireBalls 个方向）
 * 3. 为每个旋转：
 *    - 创建生成变换（位置和旋转）
 *    - 延迟生成火球（SpawnActorDeferred）
 *    - 设置伤害效果参数（飞行伤害和爆炸伤害）
 *    - 设置返回目标（ReturnToActor，火球会返回角色）
 *    - 设置 Owner
 *    - 完成生成（FinishSpawning）
 * 4. 返回所有生成的火球数组
 * 
 * @return 生成的火球数组
 * 
 * 使用场景：
 * - FireBlast 技能激活时调用
 * 
 * 注意：
 * - 火球会向所有方向发射（360度均匀分布）
 * - 火球会返回角色位置并爆炸（ReturnToActor）
 * - 爆炸伤害在火球返回时应用
 */
TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	
	// 计算均匀分布的旋转（360度，NumFireBalls 个方向）
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		
		// 延迟生成火球
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		// 设置伤害效果参数（飞行伤害和爆炸伤害）
		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}
	
	return FireBalls;
}
