// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPath.h"
#include "AuraAssetLoadTypes.generated.h"

/**
 * 资源加载请求
 *
 * 设计目的：
 * - 把"加载一组资源"这件事封装成对象，新增字段（优先级、超时、依赖等）
 *   不需要修改 Manager 接口签名，调用方代码也不必变动
 * - 通过 GroupKey 让不同业务模块的资源生命周期相互独立，便于整体卸载
 * - DebugContext 让加载失败日志可以精确定位到来源
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraLoadRequest
{
	GENERATED_BODY()

	/** 待加载的软引用资产路径列表（空数组 = 立即触发完成回调） */
	UPROPERTY(BlueprintReadWrite, Category = "Aura|AssetLoad")
	TArray<FSoftObjectPath> SoftPaths;

	/**
	 * 分组键
	 * - 同一 GroupKey 的多次请求共享 / 累加同一个 Handle 集合
	 * - 调用 ReleaseGroup(GroupKey) 即可整体释放该分组的所有资产引用
	 * - NAME_None 表示匿名加载，由调用方自行管理返回的 Handle
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Aura|AssetLoad")
	FName GroupKey = NAME_None;

	/**
	 * 加载优先级
	 * 默认使用 FStreamableManager::DefaultAsyncLoadPriority
	 * 由于 TAsyncLoadPriority 不是反射类型，此字段不暴露给蓝图
	 */
	TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority;

	/** 调试上下文（用于日志定位，例如数据资产名 / 模块名） */
	UPROPERTY(BlueprintReadWrite, Category = "Aura|AssetLoad")
	FString DebugContext;
};

/**
 * 资源加载结果
 *
 * 在异步加载完成后回调给调用方，相比裸的 FStreamableDelegate
 * 携带了"哪些路径加载成功 / 哪些失败"的明确信息
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraLoadResult
{
	GENERATED_BODY()

	/** 是否全部成功（FailedPaths 为空时为 true） */
	UPROPERTY(BlueprintReadOnly, Category = "Aura|AssetLoad")
	bool bSuccess = false;

	/** 请求加载的资产数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Aura|AssetLoad")
	int32 RequestedCount = 0;

	/** 实际加载成功的资产数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Aura|AssetLoad")
	int32 LoadedCount = 0;

	/** 加载失败（无法 ResolveObject）的软路径列表 */
	UPROPERTY(BlueprintReadOnly, Category = "Aura|AssetLoad")
	TArray<FSoftObjectPath> FailedPaths;
};

/**
 * 资源加载完成委托
 * 替代裸 FStreamableDelegate，回调时携带 FAuraLoadResult
 */
DECLARE_DELEGATE_OneParam(FAuraLoadCompleted, const FAuraLoadResult& /*Result*/);
