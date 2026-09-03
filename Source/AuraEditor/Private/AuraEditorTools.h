// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"

/**
 * Aura 游戏级编辑器工具集（AuraEditor 模块）
 *
 * 与插件级 AuraCoreEditor 的分工：
 * - AuraCoreEditor：只依赖 AuraCore，做通用工具（经接口操作玩家）
 * - AuraEditor（本模块）：可依赖游戏模块，做【游戏专属】工具——
 *   直接引用 AAuraEnemySpawnPoint / UAuraAttributeSet 等游戏类
 *
 * 控制台命令（均需 PIE 运行中）：
 *   AuraEditor.SpawnEnemies [数量]   在所有敌人生成点各刷 N 个敌人（复用 SpawnPoint 的配置）
 *   AuraEditor.DumpAttributes [all]  反射枚举 GAS 属性当前值（all=场上全部角色，默认仅玩家）
 *   AuraEditor.Heal                  回满玩家血/蓝（PIE 服务器直改，调试豁免场景）
 */
namespace AuraEditorTools
{
	/** 在所有敌人生成点刷敌人（每个点 Count 个）。返回实际触发的生成次数 */
	bool SpawnEnemies(int32 Count);

	/** 打印 GAS 属性快照（bAll=true 时包含场上所有带 ASC 的 Pawn，否则仅本地玩家） */
	bool DumpAttributes(bool bAll);

	/** 回满玩家生命/法力（需 PIE 服务器） */
	bool HealPlayer();
}
