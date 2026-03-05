// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraBeamSpell.generated.h"

/**
 * 光束技能基类
 *
 * 所有光束类技能（如电击 Electrocute）的基类
 * 光束技能会在施法者和目标之间连接一条持续伤害的光束
 * 并可以在主目标死亡后自动跳转到附近的其他目标
 *
 * 技能流程：
 *   1. StoreMouseDataInfo：记录鼠标点击位置和命中 Actor
 *   2. StoreOwnerVariables：缓存施法者的控制器和角色引用
 *   3. TraceFirstTarget：从施法者到目标位置进行射线检测，确定主目标
 *   4. StoreAdditionalTargets：在主目标附近查找额外目标（链式闪电）
 *   5. 对所有目标持续造成伤害，直到技能结束
 *   6. 主目标死亡时触发 PrimaryTargetDied 蓝图事件（跳转到下一个目标）
 *   7. 附加目标死亡时触发 AdditionalTargetDied 蓝图事件
 */
UCLASS()
class AURA_API UAuraBeamSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 存储鼠标命中数据（蓝图可调用）
	 * 记录鼠标光标射线检测的命中位置和命中 Actor
	 * 在技能激活时由 AbilityTask（TargetDataUnderMouse）回调
	 * @param HitResult 鼠标光标的射线检测结果
	 */
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);

	/**
	 * 存储施法者变量（蓝图可调用）
	 * 缓存施法者的 PlayerController 和 Character 引用，避免每帧重复查找
	 * 在技能激活时调用一次
	 */
	UFUNCTION(BlueprintCallable)
	void StoreOwnerVariables();

	/**
	 * 追踪第一个目标（蓝图可调用）
	 * 从施法者位置向 BeamTargetLocation 进行射线检测
	 * 如果命中有效目标（有 ASC 的 Actor），将其设为主目标
	 * 如果未命中，使用 BeamTargetLocation 作为光束终点
	 * @param BeamTargetLocation 光束目标位置（鼠标点击位置）
	 */
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget(const FVector& BeamTargetLocation);

	/**
	 * 查找附加目标（蓝图可调用）
	 * 在主目标附近的球形范围内查找其他有效目标（链式跳转）
	 * 最多查找 MaxNumShockTargets 个附加目标
	 * @param OutAdditionalTargets 输出：找到的附加目标 Actor 数组
	 */
	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);

	/**
	 * 蓝图事件：主目标死亡时调用
	 * 在蓝图中实现光束跳转逻辑（将光束连接到下一个目标）
	 * @param DeadActor 死亡的主目标 Actor
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void PrimaryTargetDied(AActor* DeadActor);

	/**
	 * 蓝图事件：附加目标死亡时调用
	 * 在蓝图中实现光束断开逻辑（移除对应的光束特效）
	 * @param DeadActor 死亡的附加目标 Actor
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void AdditionalTargetDied(AActor* DeadActor);

protected:
	/**
	 * 鼠标命中位置（世界坐标）
	 * 由 StoreMouseDataInfo 设置，作为光束的初始目标位置
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	FVector MouseHitLocation;

	/**
	 * 鼠标命中的 Actor
	 * 由 StoreMouseDataInfo 设置，如果命中了有效 Actor 则作为主目标
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<AActor> MouseHitActor;

	/**
	 * 施法者的玩家控制器（缓存引用）
	 * 由 StoreOwnerVariables 设置，用于获取鼠标位置等输入信息
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<APlayerController> OwnerPlayerController;

	/**
	 * 施法者的角色（缓存引用）
	 * 由 StoreOwnerVariables 设置，用于获取施法者位置和插槽位置
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<ACharacter> OwnerCharacter;

	/**
	 * 最大附加目标数量（链式闪电跳转数）
	 * 主目标周围最多可以跳转到此数量的附加目标
	 * 默认 5 个
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxNumShockTargets = 5;
};
