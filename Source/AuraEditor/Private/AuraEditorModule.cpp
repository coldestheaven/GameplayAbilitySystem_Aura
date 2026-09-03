// Copyright Druid Mechanics

#include "AuraEditorModule.h"
#include "Modules/ModuleManager.h"

void FAuraEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[AuraEditor] 游戏级编辑器模块已加载（工具命令: AuraEditor.SpawnEnemies / DumpAttributes / Heal）"));
}

void FAuraEditorModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FAuraEditorModule, AuraEditor)
