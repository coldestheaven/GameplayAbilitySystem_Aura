// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraBeamSpell.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * 存储鼠标数据信息（从 TargetDataUnderMouse 获取）
 * 
 * 实现流程：
 * 1. 如果命中结果有效（bBlockingHit），保存命中位置和 Actor
 * 2. 如果未命中，取消技能
 * 
 * @param HitResult 鼠标光标下的命中结果
 * 
 * 使用场景：
 * - 技能激活时从 TargetDataUnderMouse 任务获取目标信息
 * 
 * 注意：
 * - 如果未命中有效目标，技能会被取消
 */
void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

/**
 * 存储拥有者变量（缓存常用引用）
 * 
 * 实现流程：
 * 1. 从 CurrentActorInfo 获取 PlayerController
 * 2. 从 CurrentActorInfo 获取 Character（AvatarActor）
 * 
 * 使用场景：
 * - 技能激活时调用，缓存常用引用以提高性能
 */
void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

/**
 * 追踪第一个目标（主目标）
 * 
 * 实现流程：
 * 1. 检查 OwnerCharacter 是否实现了 CombatInterface
 * 2. 获取武器组件
 * 3. 从武器插槽（TipSocket）位置到目标位置进行球体追踪：
 *    - 半径：10
 *    - 忽略拥有者
 *    - 使用 TraceTypeQuery1（可配置的追踪类型）
 * 4. 如果命中，保存命中位置和 Actor
 * 5. 如果目标实现了 CombatInterface，绑定死亡委托（PrimaryTargetDied）
 * 
 * @param BeamTargetLocation 光束目标位置（鼠标位置）
 * 
 * 使用场景：
 * - 光束技能激活时调用，确定主目标
 * 
 * 注意：
 * - 使用球体追踪确保即使目标移动也能命中
 * - 绑定死亡委托用于目标死亡时切换目标
 */
void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
			
			// 从武器插槽位置到目标位置进行球体追踪
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult,
				true);

			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
	
	// 绑定目标死亡委托（用于目标死亡时切换目标）
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::PrimaryTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::PrimaryTargetDied);
		}
	}
}

/**
 * 存储额外目标（光束传播目标）
 * 
 * 实现流程：
 * 1. 创建忽略列表（拥有者和主目标）
 * 2. 在主目标位置周围 850 单位范围内查找存活角色（GetLivePlayersWithinRadius）
 * 3. 计算额外目标数量（Min(等级-1, MaxNumShockTargets)）
 * 4. 从重叠角色中获取最近的 N 个目标（GetClosestTargets）
 * 5. 为每个额外目标绑定死亡委托（AdditionalTargetDied）
 * 
 * @param OutAdditionalTargets 输出的额外目标数组
 * 
 * 使用场景：
 * - 光束技能激活时调用，确定光束传播的额外目标
 * 
 * 注意：
 * - 额外目标数量受等级限制（等级越高，可传播的目标越多）
 * - 额外目标从主目标位置开始查找（光束传播）
 * - 绑定死亡委托用于目标死亡时更新光束连接
 */
void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);
	
	// 在主目标位置周围查找存活角色
	TArray<AActor*> OverlappingActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation());
	
	// 计算额外目标数量（等级越高，可传播的目标越多）
	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	
	// 获取最近的 N 个目标
	UAuraAbilitySystemLibrary::GetClosestTargets(
		NumAdditionalTargets,
		OverlappingActors,
		OutAdditionalTargets,
		MouseHitActor->GetActorLocation());

	// 为每个额外目标绑定死亡委托
	for (AActor* Target : OutAdditionalTargets)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDied);
			}
		}
	}
}
