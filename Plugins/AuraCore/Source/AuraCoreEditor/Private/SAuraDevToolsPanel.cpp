// Copyright Druid Mechanics

#include "SAuraDevToolsPanel.h"

#include "AuraDevTools.h"

#include "SlateFwd.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SAuraDevToolsPanel::Construct(const FArguments& InArgs)
{
	const FMargin Padding(4.f);

	ChildSlot
	[
		SNew(SVerticalBox)

		// 标题
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("AuraDev", "PanelTitle", "Aura 开发者工具（PIE 玩家快捷操作）"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// 说明
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("AuraDev", "PanelDesc", "需先运行 PIE。按钮操作本地玩家，结果见右下角通知与 Output Log。"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]

		// 数量输入框
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SAssignNew(AmountInput, SEditableTextBox)
			.HintText(NSLOCTEXT("AuraDev", "AmountHint", "数量（默认 1）"))
			.Text(FText::FromString(TEXT("100")))
			.SelectAllTextWhenFocused(true)
		]

		// 快捷按钮组（2 列网格）
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(Padding)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SAuraDevToolsPanel::OnGiveXP)
				[
					SNew(STextBlock).Text(NSLOCTEXT("AuraDev", "BtnXP", "+ XP"))
				]
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SAuraDevToolsPanel::OnAddLevels)
				[
					SNew(STextBlock).Text(NSLOCTEXT("AuraDev", "BtnLevel", "+ 等级"))
				]
			]
			+ SUniformGridPanel::Slot(0, 1)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SAuraDevToolsPanel::OnGiveAttributePoints)
				[
					SNew(STextBlock).Text(NSLOCTEXT("AuraDev", "BtnAttr", "+ 属性点"))
				]
			]
			+ SUniformGridPanel::Slot(1, 1)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SAuraDevToolsPanel::OnGiveSpellPoints)
				[
					SNew(STextBlock).Text(NSLOCTEXT("AuraDev", "BtnSpell", "+ 技能点"))
				]
			]
		]

		// 状态报告
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.OnClicked(this, &SAuraDevToolsPanel::OnPrintReport)
			[
				SNew(STextBlock).Text(NSLOCTEXT("AuraDev", "BtnReport", "打印玩家状态报告"))
			]
		]
	];
}

int32 SAuraDevToolsPanel::GetAmount() const
{
	if (!AmountInput.IsValid())
	{
		return 1;
	}
	const int32 Parsed = FCString::Atoi(*AmountInput->GetText().ToString());
	return Parsed > 0 ? Parsed : 1;
}

FReply SAuraDevToolsPanel::OnGiveXP()
{
	AuraDevTools::GiveXP(GetAmount());
	return FReply::Handled();
}

FReply SAuraDevToolsPanel::OnGiveAttributePoints()
{
	AuraDevTools::GiveAttributePoints(GetAmount());
	return FReply::Handled();
}

FReply SAuraDevToolsPanel::OnGiveSpellPoints()
{
	AuraDevTools::GiveSpellPoints(GetAmount());
	return FReply::Handled();
}

FReply SAuraDevToolsPanel::OnAddLevels()
{
	AuraDevTools::AddLevels(GetAmount());
	return FReply::Handled();
}

FReply SAuraDevToolsPanel::OnPrintReport()
{
	AuraDevTools::PrintPlayerReport();
	return FReply::Handled();
}
