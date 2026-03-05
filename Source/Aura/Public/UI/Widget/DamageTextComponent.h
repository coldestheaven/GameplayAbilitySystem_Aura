// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

/**
 * 浮动伤害数字 Widget 组件
 *
 * 在受伤角色位置显示浮动的伤害数字
 * 继承自 UWidgetComponent，将 Widget 附加到世界空间中
 *
 * 工作原理：
 * - 由 AAuraPlayerController::ShowDamageNumber（Client RPC）创建
 * - 附加到受伤角色上，然后立即 DetachFromComponent（保持世界位置）
 * - 调用 SetDamageText 蓝图事件设置伤害数值和类型
 * - 蓝图中实现浮动动画（向上飘动后淡出消失）
 *
 * 显示类型：
 * - 普通伤害：白色数字
 * - 格挡命中：黄色数字（伤害减半）
 * - 暴击：红色大号数字
 */
UCLASS()
class AURA_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:
	/**
	 * 设置伤害数字显示（蓝图事件，蓝图可调用）
	 * 在蓝图中实现具体的显示逻辑（数字格式化、颜色、动画等）
	 * @param Damage       伤害数值（显示为整数）
	 * @param bBlockedHit  是否为格挡命中（显示黄色，表示伤害减半）
	 * @param bCriticalHit 是否为暴击（显示红色大号字体）
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage, bool bBlockedHit, bool bCriticalHit);
};
