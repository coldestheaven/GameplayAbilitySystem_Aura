// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * Aura 资产管理器
 *
 * 继承自 UAssetManager，在游戏启动时执行初始化工作
 * 在 DefaultEngine.ini 中配置为项目的 AssetManagerClassName
 *
 * 主要职责：
 * - 重写 StartInitialLoading，在资产加载完成后初始化原生 GameplayTag
 * - 调用 FAuraGameplayTags::InitializeNativeGameplayTags() 注册所有原生 Tag
 *
 * 配置方式（DefaultEngine.ini）：
 *   [/Script/Engine.Engine]
 *   AssetManagerClassName=/Script/AuraCore.AuraAssetManager
 *
 * 注意：必须在所有 GAS 系统初始化之前完成 Tag 注册
 * StartInitialLoading 是最早可以安全注册 Tag 的时机
 */
UCLASS()
class AURACORE_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	/**
	 * 获取 AssetManager 单例（静态方法）
	 * @return UAuraAssetManager 实例引用
	 */
	static UAuraAssetManager& Get();

protected:
	/**
	 * 初始加载完成回调（重写基类）
	 * 在引擎完成初始资产加载后调用
	 * 在此处调用 FAuraGameplayTags::InitializeNativeGameplayTags() 注册所有原生 GameplayTag
	 */
	virtual void StartInitialLoading() override;
};
