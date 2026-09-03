// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"

/**
 * 阵营 Actor 标签常量（AuraCore 插件）
 *
 * 统一 Player / Enemy 阵营标签的获取入口，消除散落在各处的
 * FName("Player") / FName("Enemy") 硬编码字符串（代码/数据分离）。
 *
 * 说明：
 * - 这些是 Actor Tag（供 ActorHasTag / GetAllActorsWithTag 使用），
 *   不是 GameplayTag，故不入 FAuraGameplayTags 单例。
 * - 使用函数内静态缓存，避免跨模块静态初始化顺序问题。
 * - 若未来改名标签，只需改此处，所有调用点自动生效。
 *
 * 使用示例：
 *   if (TargetActor->ActorHasTag(AuraFaction::Enemy())) { ... }
 *   UGameplayStatics::GetAllActorsWithTag(this, AuraFaction::Player(), OutActors);
 */
namespace AuraFaction
{
	/** 玩家阵营 Actor 标签 */
	inline FName Player()
	{
		static const FName Tag(TEXT("Player"));
		return Tag;
	}

	/** 敌人阵营 Actor 标签 */
	inline FName Enemy()
	{
		static const FName Tag(TEXT("Enemy"));
		return Tag;
	}
}
