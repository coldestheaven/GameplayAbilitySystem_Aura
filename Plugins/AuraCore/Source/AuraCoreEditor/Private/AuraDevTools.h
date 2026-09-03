// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"

class IPlayerInterface;
class UObject;

/**
 * Aura 开发者小工具集（编辑器模块）
 *
 * 通过插件内的 IPlayerInterface 接口操作 PIE 中的玩家（零游戏模块依赖），
 * 供控制台命令（AuraDev.*）与 Slate 工具面板共用。
 *
 * 控制台命令：
 *   AuraDev.GiveXP <数量>        给玩家增加 XP
 *   AuraDev.GiveAttrPoints <数量> 给玩家增加属性点
 *   AuraDev.GiveSpellPoints <数量> 给玩家增加技能点
 *   AuraDev.LevelUp <次数>       提升玩家等级
 *   AuraDev.PlayerStats          打印玩家状态报告到日志
 */
namespace AuraDevTools
{
	/** 获取 PIE 中本地玩家 Pawn 的 IPlayerInterface（不在 PIE / 无 Pawn 时返回 nullptr） */
	AURACOREEDITOR_API IPlayerInterface* GetPIEPlayerInterface(UObject** OutObject = nullptr);

	/** 加 XP（成功返回 true，下同） */
	AURACOREEDITOR_API bool GiveXP(int32 Amount);
	/** 加属性点 */
	AURACOREEDITOR_API bool GiveAttributePoints(int32 Amount);
	/** 加技能点 */
	AURACOREEDITOR_API bool GiveSpellPoints(int32 Amount);
	/** 提升等级 */
	AURACOREEDITOR_API bool AddLevels(int32 Amount);
	/** 打印玩家状态报告（XP/等级/属性点/技能点） */
	AURACOREEDITOR_API bool PrintPlayerReport();

	/** 编辑器右下角弹出通知 */
	AURACOREEDITOR_API void Notify(const FString& Message, bool bSuccess);

	/* ── 控制台命令处理器（解析首参数为数量，缺省 1） ── */
	AURACOREEDITOR_API void CmdGiveXP(const TArray<FString>& Args);
	AURACOREEDITOR_API void CmdGiveAttributePoints(const TArray<FString>& Args);
	AURACOREEDITOR_API void CmdGiveSpellPoints(const TArray<FString>& Args);
	AURACOREEDITOR_API void CmdAddLevels(const TArray<FString>& Args);
	AURACOREEDITOR_API void CmdPrintPlayerReport(const TArray<FString>& Args);
}
