// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * AuraCore 编辑器模块入口
 *
 * 职责：
 * 1. 注册 Nomad 停靠页签 "AuraDevTools"（Slate 开发者工具面板）
 * 2. 注册 Tools 主菜单入口（打开上述面板）
 * 3. 控制台命令在 AuraDevTools.cpp 内以 FAutoConsoleCommand 静态注册
 *
 * 架构说明：
 * - 编辑器模块仅依赖 AuraCore（运行时模块），通过 IPlayerInterface
 *   接口操作 PIE 玩家，不依赖游戏模块（保持插件分层可复用）。
 */
class FAuraCoreEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** 生成工具面板页签的内容 */
	TSharedRef<class SDockTab> SpawnDevToolsTab(const class FSpawnTabArgs& Args);

	/** ToolMenus 启动回调：注册 Tools 菜单入口（成员绑定，Shutdown 按 owner 反注册） */
	void RegisterMenus();
};
