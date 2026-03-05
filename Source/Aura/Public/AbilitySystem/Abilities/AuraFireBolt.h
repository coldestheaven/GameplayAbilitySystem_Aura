// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/**
 * 火焰箭技能
 *
 * 继承自 UAuraProjectileSpell，是玩家的基础远程攻击技能
 *
 * 特性：
 * - 支持多发同时发射（最多 MaxNumProjectiles 发）
 * - 支持追踪目标（HomingProjectile）：投射物会自动追踪敌人
 * - 支持扇形扩散（ProjectileSpread）：多发时均匀分布在扇形范围内
 * - 追踪加速度随机化（HomingAccelerationMin ~ HomingAccelerationMax）
 *
 * 升级效果：
 * - 等级提升时增加发射数量（最多 MaxNumProjectiles 发）
 * - 伤害值随等级提升（通过 ScalableFloat 曲线）
 *
 * 使用示例（蓝图中）：
 *   // 在攻击蒙太奇的 AnimNotify 中调用
 *   SpawnProjectiles(MouseHitLocation, WeaponTipSocketTag, false, 0.f, HomingTarget);
 */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()
public:
	/**
	 * 获取当前等级的技能描述（重写基类）
	 * 包含：伤害值、发射数量、法力消耗、冷却时间
	 * @param Level 技能等级
	 * @return 格式化的描述字符串
	 */
	virtual FString GetDescription(int32 Level) override;

	/**
	 * 获取下一等级的技能描述（重写基类）
	 * @param Level 当前等级
	 * @return 下一等级的描述字符串
	 */
	virtual FString GetNextLevelDescription(int32 Level) override;

	/**
	 * 生成多发火焰箭投射物（蓝图可调用）
	 * 根据当前等级计算发射数量，均匀分布在扇形范围内
	 * 如果有追踪目标，投射物会自动追踪
	 * @param ProjectileTargetLocation 目标位置（鼠标点击位置）
	 * @param SocketTag                生成位置的插槽标签
	 * @param bOverridePitch           是否覆盖俯仰角
	 * @param PitchOverride            覆盖的俯仰角（度）
	 * @param HomingTarget             追踪目标 Actor（nullptr 表示直线飞行）
	 */
	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);

protected:
	/**
	 * 投射物扩散角度（度）
	 * 多发时，所有投射物均匀分布在此角度范围内
	 * 默认 90 度（左右各 45 度）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float ProjectileSpread = 90.f;

	/**
	 * 最大投射物数量
	 * 技能等级达到最大时发射此数量的投射物
	 * 默认 5 发
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	int32 MaxNumProjectiles = 5;

	/**
	 * 追踪加速度最小值（单位：cm/s²）
	 * 追踪投射物的最小转向加速度，值越大追踪越灵敏
	 * 默认 1600
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float HomingAccelerationMin = 1600.f;

	/**
	 * 追踪加速度最大值（单位：cm/s²）
	 * 追踪投射物的最大转向加速度，实际值在 Min~Max 之间随机
	 * 默认 3200
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float HomingAccelerationMax = 3200.f;

	/**
	 * 是否发射追踪投射物
	 * true：投射物会追踪 HomingTarget（如果有目标）
	 * false：投射物直线飞行
	 * 默认 true
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	bool bLaunchHomingProjectiles = true;
};
