// Copyright Druid Mechanics


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "GameplayTags/AuraGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogAuraAssetLoad, Log, All);

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);

	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	checkf(AuraAssetManager,
		TEXT("AssetManagerClassName 未配置为 UAuraAssetManager，请检查 DefaultEngine.ini 的 [/Script/Engine.Engine] 节"));
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FAuraGameplayTags::InitializeNativeGameplayTags();

	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}

TSharedPtr<FStreamableHandle> UAuraAssetManager::SubmitLoadRequest(
	const FAuraLoadRequest& Request,
	FAuraLoadCompleted OnCompleted)
{
	const int32 RequestedCount = Request.SoftPaths.Num();

	// 空请求：立即触发"成功"回调，保持调用方逻辑一致
	if (RequestedCount == 0)
	{
		if (OnCompleted.IsBound())
		{
			FAuraLoadResult EmptyResult;
			EmptyResult.bSuccess = true;
			EmptyResult.RequestedCount = 0;
			EmptyResult.LoadedCount = 0;
			OnCompleted.Execute(EmptyResult);
		}
		return nullptr;
	}

	// 拷贝路径数组进 lambda（Request 是引用，外部可能立即销毁）
	const TArray<FSoftObjectPath> PathsCopy = Request.SoftPaths;
	const FString DebugCtx = Request.DebugContext;
	const FName GroupKey = Request.GroupKey;

	// 构造内部 FStreamableDelegate，加载完成后扫描失败路径并组装 FAuraLoadResult
	FStreamableDelegate InternalDelegate = FStreamableDelegate::CreateLambda(
		[PathsCopy, DebugCtx, OnCompleted]()
		{
			FAuraLoadResult Result;
			Result.RequestedCount = PathsCopy.Num();

			for (const FSoftObjectPath& Path : PathsCopy)
			{
				// ResolveObject 仅返回已加载到内存的对象指针，不触发同步加载
				if (Path.ResolveObject() != nullptr)
				{
					++Result.LoadedCount;
				}
				else
				{
					Result.FailedPaths.Add(Path);
				}
			}

			Result.bSuccess = (Result.FailedPaths.Num() == 0);

			// 失败路径写入日志，便于定位资源缺失问题
			if (!Result.bSuccess)
			{
				UE_LOG(LogAuraAssetLoad, Warning,
					TEXT("[AssetLoad] %s : %d/%d 加载失败"),
					DebugCtx.IsEmpty() ? TEXT("Anonymous") : *DebugCtx,
					Result.FailedPaths.Num(), Result.RequestedCount);
				for (const FSoftObjectPath& Failed : Result.FailedPaths)
				{
					UE_LOG(LogAuraAssetLoad, Warning,
						TEXT("[AssetLoad]   失败路径: %s"), *Failed.ToString());
				}
			}

			if (OnCompleted.IsBound())
			{
				OnCompleted.Execute(Result);
			}
		});

	// 发起异步加载
	TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
		PathsCopy,
		InternalDelegate,
		Request.Priority,
		/*bManageActiveHandle=*/false);

	// 分组持有 Handle —— 同一 GroupKey 再次提交会替换旧 Handle
	if (GroupKey != NAME_None && Handle.IsValid())
	{
		GroupHandles.Add(GroupKey, Handle);
	}

	return Handle;
}

bool UAuraAssetManager::ReleaseGroup(FName GroupKey)
{
	if (GroupKey == NAME_None)
	{
		return false;
	}
	// Remove 返回被移除的元素数量；TSharedPtr 析构后自动减少 StreamableHandle 引用计数
	return GroupHandles.Remove(GroupKey) > 0;
}

bool UAuraAssetManager::HasGroup(FName GroupKey) const
{
	if (GroupKey == NAME_None)
	{
		return false;
	}
	const TSharedPtr<FStreamableHandle>* Found = GroupHandles.Find(GroupKey);
	return Found != nullptr && Found->IsValid();
}

// 兼容旧接口：转发到 SubmitLoadRequest，匿名分组（GroupKey=NAME_None）
PRAGMA_DISABLE_DEPRECATION_WARNINGS
TSharedPtr<FStreamableHandle> UAuraAssetManager::PreloadAssets(
	const TArray<FSoftObjectPath>& SoftPaths,
	FStreamableDelegate OnComplete)
{
	FAuraLoadRequest Request;
	Request.SoftPaths = SoftPaths;
	Request.DebugContext = TEXT("LegacyPreloadAssets");

	// 把旧的 FStreamableDelegate 适配成新的 FAuraLoadCompleted（丢弃 Result 信息）
	FAuraLoadCompleted Adapter;
	if (OnComplete.IsBound())
	{
		Adapter = FAuraLoadCompleted::CreateLambda(
			[OnComplete](const FAuraLoadResult& /*Result*/)
			{
				OnComplete.ExecuteIfBound();
			});
	}

	return SubmitLoadRequest(Request, Adapter);
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS