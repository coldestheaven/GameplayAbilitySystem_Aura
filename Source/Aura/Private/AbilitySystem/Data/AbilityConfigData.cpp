// Copyright Druid Mechanics


#include "AbilitySystem/Data/AbilityConfigData.h"

#include "AuraAssetManager.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

UNiagaraSystem* FVisualEffectConfig::GetParticleSystemSafe() const
{
	if (ParticleSystem.IsNull()) return nullptr;
	if (UNiagaraSystem* Loaded = ParticleSystem.Get()) return Loaded;
	// 异步预加载尚未完成，回退到同步加载（兜底，正常情况不应走到这里）
	return ParticleSystem.LoadSynchronous();
}

USoundBase* FVisualEffectConfig::GetSoundSafe() const
{
	if (Sound.IsNull()) return nullptr;
	if (USoundBase* Loaded = Sound.Get()) return Loaded;
	return Sound.LoadSynchronous();
}

void UAbilityConfigData::PreloadVisualAssets() const
{
	// 构造加载请求：路径 + 分组键（用数据资产名，让生命周期跟随本资产）
	FAuraLoadRequest Request;
	CollectVisualSoftPaths(Request.SoftPaths);
	if (Request.SoftPaths.Num() == 0)
	{
		return;
	}
	Request.GroupKey = FName(*GetName());
	Request.DebugContext = GetName();

	// 提交到 UAuraAssetManager；失败时由 Manager 内部打 Warning 日志
	// 这里再额外感知一次失败结果，方便后续做"降级 / 占位资源"等业务策略
	UAuraAssetManager::Get().SubmitLoadRequest(
		Request,
		FAuraLoadCompleted::CreateLambda([AssetName = GetName()](const FAuraLoadResult& Result)
		{
			if (!Result.bSuccess)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[AbilityConfig:%s] 视觉资源预加载未全部成功 (%d/%d)"),
					*AssetName, Result.LoadedCount, Result.RequestedCount);
			}
		}));
}
