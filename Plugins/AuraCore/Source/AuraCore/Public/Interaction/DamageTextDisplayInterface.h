// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageTextDisplayInterface.generated.h"

class ACharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UDamageTextDisplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 飘字（伤害数字）显示接口
 *
 * 用途：解耦 GAS 数据层（AttributeSet）与具体业务层（PlayerController）
 *      使 AttributeSet 不再 #include "Player/AuraPlayerController.h"
 *      并避免 Cast<AAuraPlayerController>(...) 这种向上耦合
 *
 * 实现者：
 *   - 通常由 PlayerController 实现（保留 Client RPC 网络行为）
 *   - 也可由其他 Controller 或 HUD 实现，扩展性更好
 *
 * 调用约定：
 *   - 在服务器侧由 AttributeSet::PostGameplayEffectExecute 调用
 *   - 实现者内部应通过 Client RPC 在对应客户端生成飘字 UI
 *
 * 设计动机：见 Aura 系统耦合重构计划——切断"GAS 底层 → 业务上层"反向依赖
 */
class AURACORE_API IDamageTextDisplayInterface
{
	GENERATED_BODY()

public:
	/**
	 * 显示一段伤害飘字
	 *
	 * @param DamageAmount    伤害数值（已含护甲/格挡/暴击计算）
	 * @param TargetCharacter 飘字定位的目标角色
	 * @param bBlockedHit     是否被格挡（影响 UI 颜色/样式）
	 * @param bCriticalHit    是否暴击（影响 UI 颜色/样式）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DamageText")
	void DisplayDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);
};
