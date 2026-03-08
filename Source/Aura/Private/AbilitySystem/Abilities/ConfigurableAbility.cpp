// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/ConfigurableAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actor/AuraProjectile.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "Interaction/CombatInterface.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"

/**
 * 构造函数：初始化可配置技能
 * 
 * 实现流程：
 * 1. 初始化动作索引为 0
 * 2. 初始化目标缓存标志为 false
 * 3. 初始化目标位置为 ZeroVector
 */
UConfigurableAbility::UConfigurableAbility()
{
	CurrentActionIndex = 0;
	bHasCachedTarget = false;
	CachedTargetLocation = FVector::ZeroVector;
}

/**
 * 激活技能（重写基类）
 * 
 * 实现流程：
 * 1. 提交技能（CommitAbility，检查消耗和冷却）
 * 2. 校验 AbilityConfig 已配置
 * 3. 校验动作列表不为空
 * 4. 如果技能需要目标，缓存目标位置
 * 5. 播放施法视觉效果（粒子系统和音效）
 * 6. 重置动作索引
 * 7. 开始执行动作序列（ExecuteNextAction）
 * 
 * 使用场景：
 * - 技能激活时自动调用
 * 
 * 注意：
 * - ConfigurableAbility 通过 AbilityConfig 数据资产配置技能行为
 * - 技能由一系列动作组成，按顺序执行
 */
void UConfigurableAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!AbilityConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfigurableAbility] No AbilityConfig assigned!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (AbilityConfig->Actions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConfigurableAbility] No actions defined in config!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	// 缓存目标位置（如果技能需要目标）
	if (AbilityConfig->bRequiresTarget)
	{
		CachedTargetLocation = GetTargetLocation();
		bHasCachedTarget = true;
	}
	
	// 播放施法视觉效果
	if (AbilityConfig->CastVisualEffect.ParticleSystem.IsValid() || 
		AbilityConfig->CastVisualEffect.Sound.IsValid())
	{
		FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
		FRotator Rotation = GetAvatarActorFromActorInfo()->GetActorRotation();
		PlayVisualEffect(AbilityConfig->CastVisualEffect, Location, Rotation);
	}
	
	// 重置动作索引
	CurrentActionIndex = 0;
	
	// 开始执行动作序列
	ExecuteNextAction();
}

/**
 * 结束技能（重写基类）
 * 
 * 实现流程：
 * 1. 清理动作定时器（如果有效）
 * 2. 重置动作索引和目标缓存
 * 3. 调用父类 EndAbility
 * 
 * 使用场景：
 * - 技能结束时自动调用
 */
void UConfigurableAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 清理定时器
	if (ActionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);
		ActionTimerHandle.Invalidate();
	}
	
	// 重置状态
	CurrentActionIndex = 0;
	bHasCachedTarget = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/**
 * 执行下一个动作
 * 
 * 实现流程：
 * 1. 检查是否所有动作已执行完毕
 * 2. 如果完毕，结束技能
 * 3. 获取当前动作配置
 * 4. 如果动作有延迟：
 *    - 设置定时器，延迟执行动作
 *    - 执行后递增索引并继续下一个动作
 * 5. 如果动作无延迟：
 *    - 立即执行动作
 *    - 递增索引并继续下一个动作
 * 
 * 使用场景：
 * - 技能激活时调用（开始执行序列）
 * - 每个动作执行完成后调用（继续下一个动作）
 * 
 * 注意：
 * - 动作按顺序执行，支持延迟配置
 * - 所有动作执行完毕后技能自动结束
 */
void UConfigurableAbility::ExecuteNextAction()
{
	if (!AbilityConfig || CurrentActionIndex >= AbilityConfig->Actions.Num())
	{
		// 所有动作执行完毕
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	const FAbilityActionConfig& ActionConfig = AbilityConfig->Actions[CurrentActionIndex];
	
	if (ActionConfig.Delay > 0.f)
	{
		// 延迟执行
		GetWorld()->GetTimerManager().SetTimer(
			ActionTimerHandle,
			[this, ActionConfig]()
			{
				ExecuteAction(ActionConfig);
				CurrentActionIndex++;
				ExecuteNextAction();
			},
			ActionConfig.Delay,
			false
		);
	}
	else
	{
		// 立即执行
		ExecuteAction(ActionConfig);
		CurrentActionIndex++;
		ExecuteNextAction();
	}
}

void UConfigurableAbility::ExecuteAction(const FAbilityActionConfig& ActionConfig)
{
	switch (ActionConfig.ActionType)
	{
	case EAbilityActionType::SpawnProjectile:
		ExecuteSpawnProjectile(ActionConfig);
		break;
		
	case EAbilityActionType::ApplyEffect:
		ExecuteApplyEffect(ActionConfig);
		break;
		
	case EAbilityActionType::PlayMontage:
		ExecutePlayMontage(ActionConfig);
		break;
		
	case EAbilityActionType::SpawnBeam:
		ExecuteSpawnBeam(ActionConfig);
		break;
		
	case EAbilityActionType::AreaOfEffect:
		ExecuteAreaOfEffect(ActionConfig);
		break;
		
	case EAbilityActionType::Teleport:
		ExecuteTeleport(ActionConfig);
		break;
		
	case EAbilityActionType::WaitForEvent:
		ExecuteWaitForEvent(ActionConfig);
		break;
		
	default:
		UE_LOG(LogTemp, Warning, TEXT("[ConfigurableAbility] Unknown action type!"));
		break;
	}
}

void UConfigurableAbility::ExecuteSpawnProjectile(const FAbilityActionConfig& ActionConfig)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	const FProjectileConfig& Config = ActionConfig.ProjectileConfig;
	
	if (!Config.ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfigurableAbility] No ProjectileClass defined!"));
		return;
	}
	
	// 获取生成位置
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		Config.SocketTag);
	
	// 计算目标位置
	FVector TargetLocation = bHasCachedTarget ? CachedTargetLocation : GetTargetLocation();
	
	// 生成多个投射物
	const int32 NumProjectiles = FMath::Max(1, Config.NumProjectiles);
	const float SpreadAngle = Config.SpreadAngle;
	
	for (int32 i = 0; i < NumProjectiles; ++i)
	{
		// 计算扩散角度
		FRotator Rotation = (TargetLocation - SocketLocation).Rotation();
		
		if (NumProjectiles > 1 && SpreadAngle > 0.f)
		{
			const float AngleStep = SpreadAngle / (NumProjectiles - 1);
			const float CurrentAngle = -SpreadAngle / 2.f + AngleStep * i;
			Rotation.Yaw += CurrentAngle;
		}
		
		if (Config.bOverridePitch)
		{
			Rotation.Pitch = Config.PitchOverride;
		}
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		
		// 从对象池获取投射物
		UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
		AAuraProjectile* Projectile = nullptr;
		
		if (PoolSubsystem)
		{
			Projectile = Cast<AAuraProjectile>(PoolSubsystem->AcquireActor(Config.ProjectileClass, SpawnTransform));
		}
		else
		{
			Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
				Config.ProjectileClass,
				SpawnTransform,
				GetOwningActorFromActorInfo(),
				Cast<APawn>(GetOwningActorFromActorInfo()),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		}
		
		if (Projectile)
		{
			// 设置伤害参数
			Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
			
			if (!PoolSubsystem)
			{
				Projectile->FinishSpawning(SpawnTransform);
			}
		}
	}
	
	// 播放视觉效果
	if (ActionConfig.VisualEffect.ParticleSystem.IsValid() || 
		ActionConfig.VisualEffect.Sound.IsValid())
	{
		PlayVisualEffect(ActionConfig.VisualEffect, SocketLocation, FRotator::ZeroRotator);
	}
}

void UConfigurableAbility::ExecuteApplyEffect(const FAbilityActionConfig& ActionConfig)
{
	const FEffectConfig& Config = ActionConfig.EffectConfig;
	
	if (!Config.EffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfigurableAbility] No EffectClass defined!"));
		return;
	}
	
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;
	
	// 应用到自己
	if (Config.bApplyToSelf)
	{
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
			Config.EffectClass,
			Config.EffectLevel,
			ContextHandle);
		
		if (SpecHandle.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	// 应用到目标
	if (Config.bApplyToTarget)
	{
		// TODO: 实现目标选择逻辑
		UE_LOG(LogTemp, Log, TEXT("[ConfigurableAbility] Apply effect to target not yet implemented"));
	}
}

void UConfigurableAbility::ExecutePlayMontage(const FAbilityActionConfig& ActionConfig)
{
	const FMontageConfig& Config = ActionConfig.MontageConfig;
	
	if (!Config.Montage)
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfigurableAbility] No Montage defined!"));
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;
	
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;
	
	AnimInstance->Montage_Play(Config.Montage, Config.PlayRate);
	
	if (Config.StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(Config.StartSectionName, Config.Montage);
	}
}

void UConfigurableAbility::ExecuteSpawnBeam(const FAbilityActionConfig& ActionConfig)
{
	// TODO: 实现光束生成逻辑
	UE_LOG(LogTemp, Log, TEXT("[ConfigurableAbility] Spawn beam not yet implemented"));
}

void UConfigurableAbility::ExecuteAreaOfEffect(const FAbilityActionConfig& ActionConfig)
{
	const FAOEConfig& Config = ActionConfig.AOEConfig;
	
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	
	// 查找范围内的目标
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
	
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Config.Radius),
		QueryParams);
	
	int32 TargetsAffected = 0;
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (Config.MaxTargets > 0 && TargetsAffected >= Config.MaxTargets)
		{
			break;
		}
		
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor) continue;
		
		// TODO: 检查友军/敌人
		
		// 应用效果
		if (Config.Effect.EffectClass)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC)
			{
				FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
				ContextHandle.AddSourceObject(this);
				
				const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
					Config.Effect.EffectClass,
					Config.Effect.EffectLevel,
					ContextHandle);
				
				if (SpecHandle.IsValid())
				{
					GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
						*SpecHandle.Data.Get(),
						TargetASC);
					
					TargetsAffected++;
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[ConfigurableAbility] AOE affected %d targets"), TargetsAffected);
}

void UConfigurableAbility::ExecuteTeleport(const FAbilityActionConfig& ActionConfig)
{
	// TODO: 实现传送逻辑
	UE_LOG(LogTemp, Log, TEXT("[ConfigurableAbility] Teleport not yet implemented"));
}

void UConfigurableAbility::ExecuteWaitForEvent(const FAbilityActionConfig& ActionConfig)
{
	// TODO: 实现事件等待逻辑
	UE_LOG(LogTemp, Log, TEXT("[ConfigurableAbility] Wait for event not yet implemented"));
}

void UConfigurableAbility::PlayVisualEffect(const FVisualEffectConfig& VisualConfig, const FVector& Location, const FRotator& Rotation)
{
	// 播放粒子效果
	if (VisualConfig.ParticleSystem.IsValid())
	{
		UNiagaraSystem* ParticleSystem = VisualConfig.ParticleSystem.LoadSynchronous();
		if (ParticleSystem)
		{
			FVector SpawnLocation = Location + VisualConfig.LocationOffset;
			FRotator SpawnRotation = Rotation + VisualConfig.RotationOffset;
			
			if (VisualConfig.bAttachToCharacter)
			{
				// TODO: 实现附加到角色的逻辑
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ParticleSystem,
					SpawnLocation,
					SpawnRotation,
					VisualConfig.Scale);
			}
			else
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ParticleSystem,
					SpawnLocation,
					SpawnRotation,
					VisualConfig.Scale);
			}
		}
	}
	
	// 播放音效
	if (VisualConfig.Sound.IsValid())
	{
		USoundBase* Sound = VisualConfig.Sound.LoadSynchronous();
		if (Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(),
				Sound,
				Location + VisualConfig.LocationOffset);
		}
	}
}

FVector UConfigurableAbility::GetTargetLocation() const
{
	// 从鼠标光标获取目标位置
	APlayerController* PC = Cast<APlayerController>(GetAvatarActorFromActorInfo()->GetInstigatorController());
	if (!PC) return GetAvatarActorFromActorInfo()->GetActorLocation();
	
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	
	return HitResult.bBlockingHit ? HitResult.Location : GetAvatarActorFromActorInfo()->GetActorLocation();
}

FString UConfigurableAbility::GetDescription(int32 Level)
{
	if (!AbilityConfig)
	{
		return FString::Printf(TEXT("<Default>未配置技能数据</>"));
	}
	
	return AbilityConfig->AbilityDescription.ToString();
}

FString UConfigurableAbility::GetNextLevelDescription(int32 Level)
{
	if (!AbilityConfig)
	{
		return FString::Printf(TEXT("<Default>未配置技能数据</>"));
	}
	
	return FString::Printf(TEXT("<Default>下一级: %s</>"), *AbilityConfig->AbilityDescription.ToString());
}
