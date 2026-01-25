// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuraCoreModule.h"

#define LOCTEXT_NAMESPACE "FAuraCoreModule"

void FAuraCoreModule::StartupModule()
{
	// 模块启动时的初始化代码
	UE_LOG(LogTemp, Log, TEXT("AuraCore Module Started"));
}

void FAuraCoreModule::ShutdownModule()
{
	// 模块关闭时的清理代码
	UE_LOG(LogTemp, Log, TEXT("AuraCore Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAuraCoreModule, AuraCore)
