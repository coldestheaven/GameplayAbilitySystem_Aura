// Copyright Druid Mechanics


#include "Actor/AuraFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 启动外出时间轴（StartOutgoingTimeline，火球飞出的动画）
 * 
 * 使用场景：
 * - 火球生成后自动调用
 */
void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

/**
 * 球体重叠事件处理（火球命中目标）
 * 
 * 实现流程：
 * 1. 校验重叠是否有效（IsValidOverlap）
 * 2. 仅服务端：应用伤害效果：
 *    - 计算死亡冲量（火球前进方向 * 冲量大小）
 *    - 设置目标 ASC
 *    - 调用 ApplyDamageEffect 应用伤害
 * 
 * @param OverlappedComponent 重叠的组件（Sphere）
 * @param OtherActor 重叠的 Actor（目标）
 * @param OtherComp 其他组件的碰撞组件
 * @param OtherBodyIndex 其他组件的 Body 索引
 * @param bFromSweep 是否来自扫描
 * @param SweepResult 扫描结果
 * 
 * 使用场景：
 * - 火球与目标重叠时自动调用
 * 
 * 网络同步说明：
 * - 伤害应用仅在服务端执行
 */
void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;

	// 仅服务端：应用伤害效果
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// 计算死亡冲量（火球前进方向 * 冲量大小）
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

/**
 * 火球命中处理（返回时爆炸）
 * 
 * 实现流程：
 * 1. 如果 Owner 有效，执行 GameplayCue（FireBlast 特效）
 * 2. 停止并销毁循环音效组件
 * 3. 设置 bHit = true（标记已命中）
 * 
 * 使用场景：
 * - 火球返回角色位置时调用
 * - 火球与目标重叠时调用
 * 
 * 注意：
 * - GameplayCue 用于播放爆炸特效（非复制，本地执行）
 */
void AAuraFireBall::OnHit()
{
	if (GetOwner())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FAuraGameplayTags::Get().GameplayCue_FireBlast, CueParams);
	}
	
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}
