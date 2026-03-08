// Copyright Druid Mechanics


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

/**
 * 构造函数：初始化投射物组件
 * 
 * 实现流程：
 * 1. 禁用 Tick（投射物不需要每帧更新）
 * 2. 启用网络复制（投射物需要在所有客户端显示）
 * 3. 创建球体碰撞组件，设置为根组件
 * 4. 配置碰撞：
 *    - 碰撞对象类型：ECC_Projectile
 *    - 碰撞模式：QueryOnly（仅查询，不物理模拟）
 *    - 默认忽略所有通道
 *    - 与世界动态、世界静态、Pawn 重叠
 * 5. 创建投射物移动组件：
 *    - 初始速度：550
 *    - 最大速度：550
 *    - 重力缩放：0（无重力，直线飞行）
 * 
 * 使用场景：
 * - 投射物 Actor 在技能激活时创建
 * 
 * 注意：
 * - 投射物使用重叠检测而非碰撞检测（避免弹回）
 * - 无重力设计适用于魔法投射物
 */
AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 创建球体碰撞组件
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 创建投射物移动组件（无重力，直线飞行）
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 启用移动复制（确保客户端看到正确的投射物位置）
 * 3. 绑定重叠事件（OnSphereOverlap）
 * 4. 如果配置了循环音效，播放循环音效
 * 
 * 使用场景：
 * - 投射物生成后自动调用
 */
void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);

	if (LoopingSound)
	{
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
	}
}

/**
 * 投射物命中处理
 * 
 * 实现流程：
 * 1. 在命中位置播放命中音效
 * 2. 在命中位置生成命中特效（Niagara）
 * 3. 停止并销毁循环音效组件
 * 4. 设置 bHit = true（标记已命中）
 * 
 * 使用场景：
 * - 投射物与目标重叠时调用
 * - 投射物销毁时调用（如果未命中）
 * 
 * 注意：
 * - 命中后不会立即销毁，由 OnSphereOverlap 处理销毁逻辑
 */
void AAuraProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}

/**
 * Actor 销毁时调用
 * 
 * 实现流程：
 * 1. 停止并销毁循环音效组件
 * 2. 如果未命中且是客户端，调用 OnHit（确保客户端看到命中效果）
 * 3. 调用父类 Destroyed
 * 
 * 使用场景：
 * - 投射物被销毁时自动调用
 * 
 * 注意：
 * - 客户端在销毁时如果未命中，会播放命中效果（补偿网络延迟）
 */
void AAuraProjectile::Destroyed()
{
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	if (!bHit && !HasAuthority()) OnHit();
	Super::Destroyed();
}

/**
 * 球体重叠事件处理（投射物命中目标）
 * 
 * 实现流程：
 * 1. 校验重叠是否有效（IsValidOverlap，检查是否为友军、是否已命中等）
 * 2. 如果未命中，调用 OnHit（播放命中效果）
 * 3. 仅服务端：应用伤害效果：
 *    - 计算死亡冲量（投射物前进方向 * 冲量大小）
 *    - 随机判定是否击退（根据 KnockbackChance）
 *    - 如果击退，计算击退方向（投射物方向，Pitch=45度，向上击退）
 *    - 设置目标 ASC
 *    - 调用 ApplyDamageEffect 应用伤害
 * 4. 归还投射物到对象池（而不是销毁）
 * 
 * @param OverlappedComponent 重叠的组件（Sphere）
 * @param OtherActor 重叠的 Actor（目标）
 * @param OtherComp 其他组件的碰撞组件
 * @param OtherBodyIndex 其他组件的 Body 索引
 * @param bFromSweep 是否来自扫描
 * @param SweepResult 扫描结果
 * 
 * 使用场景：
 * - 投射物与目标重叠时自动调用
 * 
 * 网络同步说明：
 * - 命中效果在客户端立即播放（OnHit）
 * - 伤害应用仅在服务端执行（确保权威性）
 * 
 * 注意：
 * - 投射物使用对象池管理，命中后归还而非销毁（提高性能）
 * - 击退方向向上 45 度，创造更好的视觉效果
 */
void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;
	if (!bHit) OnHit();
	
	// 仅服务端：应用伤害效果
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// 计算死亡冲量（投射物前进方向 * 冲量大小）
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			
			// 随机判定是否击退
			const bool bKnockback = FMath::RandRange(1, 100) < DamageEffectParams.KnockbackChance;
			if (bKnockback)
			{
				// 计算击退方向（向上 45 度）
				FRotator Rotation = GetActorRotation();
				Rotation.Pitch = 45.f;
				
				const FVector KnockbackDirection = Rotation.Vector();
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
				DamageEffectParams.KnockbackForce = KnockbackForce;
			}
			
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		
		// 归还到对象池而不是销毁（提高性能）
		if (UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
		{
			PoolSubsystem->ReleaseActor(this);
		}
		else
		{
			Destroy();
		}
	}
	else bHit = true;
}

bool AAuraProjectile::IsValidOverlap(AActor* OtherActor)
{
	if (DamageEffectParams.SourceAbilitySystemComponent == nullptr) return false;
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) return false;
	if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return false;

	return true;
}

// IPoolableObject接口实现

void AAuraProjectile::OnAcquiredFromPool_Implementation()
{
	// 重置投射物状态
	bHit = false;
	
	// 激活投射物移动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->Activate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}
	
	// 重新设置生命周期
	SetLifeSpan(LifeSpan);
	
	// 重新生成循环音效
	if (LoopingSound && !LoopingSoundComponent)
	{
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
	}
	
	UE_LOG(LogTemp, Log, TEXT("Projectile %s acquired from pool"), *GetName());
}

void AAuraProjectile::OnReturnedToPool_Implementation()
{
	// 清理投射物状态
	bHit = false;
	
	// 停用投射物移动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}
	
	// 清理音效组件
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
		LoopingSoundComponent = nullptr;
	}
	
	// 清除生命周期定时器
	SetLifeSpan(0.f);
	
	// 重置寻的目标
	if (HomingTargetSceneComponent)
	{
		HomingTargetSceneComponent->DestroyComponent();
		HomingTargetSceneComponent = nullptr;
	}
	
	// 重置伤害参数
	DamageEffectParams = FDamageEffectParams();
	
	UE_LOG(LogTemp, Log, TEXT("Projectile %s returned to pool"), *GetName());
}

bool AAuraProjectile::CanReturnToPool_Implementation() const
{
	// 检查是否可以安全归还到池中
	return IsValid(this) && GetWorld() != nullptr;
}
