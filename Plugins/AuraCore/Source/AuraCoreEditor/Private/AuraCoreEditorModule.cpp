// Copyright Druid Mechanics

#include "AuraCoreEditorModule.h"

#include "SAuraDevToolsPanel.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "AuraCoreEditor"

static const FName AuraDevToolsTabName(TEXT("AuraDevTools"));

void FAuraCoreEditorModule::StartupModule()
{
	// 注册 Nomad 停靠页签（可在编辑器任意布局中停靠/浮动）
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		AuraDevToolsTabName,
		FOnSpawnTab::CreateRaw(this, &FAuraCoreEditorModule::SpawnDevToolsTab))
		.SetDisplayName(LOCTEXT("DevToolsTabTitle", "Aura 开发者工具"))
		.SetTooltipText(LOCTEXT("DevToolsTabTooltip", "PIE 玩家快捷操作：XP/等级/属性点/技能点"));

	// ToolMenus 就绪后注册菜单（成员委托绑定，ShutdownModule 可按 owner 反注册）
	UToolMenus::RegisterStartupCallback(
		FSimpleDelegate::CreateRaw(this, &FAuraCoreEditorModule::RegisterMenus));
}

void FAuraCoreEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AuraDevToolsTabName);
}

void FAuraCoreEditorModule::RegisterMenus()
{
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (!ToolsMenu)
	{
		return;
	}

	FToolMenuSection& Section = ToolsMenu->FindOrAddSection("AuraDevTools");
	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
		TEXT("OpenAuraDevTools"),
		LOCTEXT("OpenDevTools", "Aura 开发者工具面板"),
		LOCTEXT("OpenDevToolsTip", "打开 PIE 玩家快捷操作面板"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FTabId(AuraDevToolsTabName));
		}))));
}

TSharedRef<SDockTab> FAuraCoreEditorModule::SpawnDevToolsTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAuraDevToolsPanel)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAuraCoreEditorModule, AuraCoreEditor)
