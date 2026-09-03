// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * AuraEditor 游戏级编辑器模块入口
 *
 * 与插件级 AuraCoreEditor 的分工：
 * - 本模块依赖游戏模块（Aura），可引用游戏类做【专属工具】
 *   （刷怪/属性快照/治疗，见 Private/AuraEditorTools.cpp）
 * - 通用工具（经接口操作玩家）仍在插件级 AuraCoreEditor
 *
 * 控制台命令以 FAutoConsoleCommand 静态注册，模块无需额外初始化逻辑。
 */
class FAuraEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
