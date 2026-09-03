// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;

/**
 * Aura 开发者工具面板（Slate 停靠页签内容）
 *
 * 布局：数量输入框 + 快捷按钮组（加 XP / 属性点 / 技能点 / 升级 / 状态报告）
 * 全部经由 AuraDevTools（接口调用 PIE 玩家），带右下角通知反馈。
 */
class SAuraDevToolsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAuraDevToolsPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** 读取输入框数量（非法输入回退 1） */
	int32 GetAmount() const;

	/** 按钮回调（返回值适配 SButton） */
	FReply OnGiveXP();
	FReply OnGiveAttributePoints();
	FReply OnGiveSpellPoints();
	FReply OnAddLevels();
	FReply OnPrintReport();

	/** 数量输入框 */
	TSharedPtr<SEditableTextBox> AmountInput;
};
