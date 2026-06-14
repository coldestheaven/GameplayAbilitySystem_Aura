// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "AuraAssetLoadTypes.h"
#include "AuraAssetManager.generated.h"

/**
 * Aura 资产管理器
 *
 * 继承自 UAssetManager，在游戏启动时执行初始化工作
 * 在 DefaultEngine.ini 中配置为项目的 AssetManagerClassName
 *
 * 主要职责：
 * - 重写 StartInitialLoading，初始化原生 GameplayTag 与 GAS 全局数据
 * - 提供软引用资产的异步预加载入口（基于 FAuraLoadRequest）
 * - 通过 GroupKey 管理不同业务模块的资源生命周期，支持整体卸载
 * - 携带成功/失败信息的回调（FAuraLoadResult），便于业务侧处理加载失败
 * - 避免业务代码中频繁调用 LoadSynchronous() 导致游戏线程阻塞
 *
 * 配置方式（DefaultEngine.ini）：
 *   [/Script/Engine.Engine]
 *   AssetManagerClassName=/Script/AuraCore.AuraAssetManager
 */
UCLASS()
class AURACORE_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	/**
	 * 获取 AssetManager 单例（静态方法）
	 * 内部使用 checkf 校验 AssetManager 类型，配置错误时给出明确报错
	 * @return UAuraAssetManager 实例引用
	 */
	static UAuraAssetManager& Get();

	/**
	 * 提交资源加载请求（推荐入口）
	 *
	 * 行为说明：
	 * - SoftPaths 为空：立即以 bSuccess=true 触发 OnCompleted（保持调用方逻辑一致）
	 * - GroupKey 非空：内部持有 Handle 到 GroupHandles，调用 ReleaseGroup 可整体释放
	 * - GroupKey 为空：返回 Handle 由调用方自行持有（否则可能被 GC）
	 * - 加载完成后逐个 ResolveObject 检测失败路径，回调携带 FAuraLoadResult
	 *
	 * @param Request      加载请求（路径 / 分组 / 优先级 / 调试上下文）
	 * @param OnCompleted  完成回调（可选）；无论成功失败均会触发一次
	 * @return             StreamableHandle，无任务或匿名空请求时为 nullptr
	 */
	TSharedPtr<FStreamableHandle> SubmitLoadRequest(
		const FAuraLoadRequest& Request,
		FAuraLoadCompleted OnCompleted = FAuraLoadCompleted());

	/**
	 * 释放某个分组持有的所有资产引用
	 * 调用后 GroupHandles[GroupKey] 被移除，对应的 Handle 引用计数归零
	 * 资产是否真正卸载取决于其他持有者；本方法只解除 Manager 的持有
	 *
	 * @param GroupKey 分组键，NAME_None 无效
	 * @return         是否实际释放了某个分组（false = 该分组不存在）
	 */
	bool ReleaseGroup(FName GroupKey);

	/**
	 * 查询某个分组当前是否持有 Handle
	 * @param GroupKey 分组键
	 * @return         true = 该分组存在且 Handle 仍有效
	 */
	bool HasGroup(FName GroupKey) const;

	/**
	 * 【已弃用】异步预加载一组软引用资产
	 *
	 * 仅为兼容旧调用，新代码请使用 SubmitLoadRequest(FAuraLoadRequest)
	 * 内部会构造一个匿名（GroupKey=NAME_None）请求转发到新接口
	 *
	 * @param SoftPaths   要预加载的资产软路径列表
	 * @param OnComplete  原生 FStreamableDelegate 回调（不携带失败信息）
	 * @return            加载句柄
	 */
	UE_DEPRECATED(5.3, "Use SubmitLoadRequest(FAuraLoadRequest) instead. "
		"This legacy API does not report load failures and uses anonymous group.")
	TSharedPtr<FStreamableHandle> PreloadAssets(
		const TArray<FSoftObjectPath>& SoftPaths,
		FStreamableDelegate OnComplete = FStreamableDelegate());

protected:
	/**
	 * 初始加载完成回调（重写基类）
	 * 在引擎完成初始资产加载后调用，注册原生 GameplayTag 与 GAS Target Data
	 */
	virtual void StartInitialLoading() override;

private:
	/**
	 * 全局 StreamableManager
	 * 收紧为 private，禁止外部模块绕过统一接口直接调用 RequestAsyncLoad
	 * 所有加载必须经由 SubmitLoadRequest，便于统一日志、失败处理、生命周期管理
	 */
	FStreamableManager StreamableManager;

	/**
	 * 分组 Handle 持有表
	 * Key = FAuraLoadRequest::GroupKey；Value = 对应分组的最近一次加载 Handle
	 * 注：同一 GroupKey 多次提交会覆盖旧 Handle（旧 Handle 引用计数自动归零）
	 *     如需"累加而非替换"语义，可在后续扩展为 TMap<FName, TArray<Handle>>
	 */
	TMap<FName, TSharedPtr<FStreamableHandle>> GroupHandles;
};
