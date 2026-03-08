// Copyright Druid Mechanics


#include "Game/AuraGameModeBase.h"

#include "EngineUtils.h"
#include "Aura/AuraLogChannels.h"
#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "GameFramework/Character.h"

/**
 * 保存存档槽数据
 * 
 * 实现流程：
 * 1. 检查存档槽是否已存在，如果存在则先删除旧存档
 * 2. 创建新的 ULoadScreenSaveGame 对象
 * 3. 从 LoadSlot ViewModel 中复制数据到存档对象：
 *    - 玩家名称、地图名称、地图资产名称、玩家出生点标签
 *    - 设置存档槽状态为 Taken（已占用）
 * 4. 将存档对象序列化并保存到磁盘
 * 
 * @param LoadSlot  包含存档信息的 ViewModel（从 UI 加载界面获取）
 * @param SlotIndex 存档槽索引（0、1、2，对应三个存档槽位）
 * 
 * 存档文件命名规则：
 * - 文件名 = SlotName + "_" + SlotIndex
 * - 例如：SlotName="SaveSlot", SlotIndex=0 → 文件名为 "SaveSlot_0"
 * 
 * 使用场景：
 * - 在加载界面创建新游戏时保存初始存档
 * - 在加载界面覆盖已有存档时更新存档数据
 */
void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// 如果存档槽已存在，先删除旧存档（避免数据残留）
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	
	// 创建新的存档对象（使用 LoadScreenSaveGameClass 指定的类）
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	
	// 从 ViewModel 复制数据到存档对象
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->SaveSlotStatus = Taken;  // 设置为"已占用"状态
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->MapAssetName = LoadSlot->MapAssetName;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

	// 将存档对象序列化并保存到磁盘
	// 引擎会自动处理序列化，只保存标记了 SaveGame 说明符的 UPROPERTY
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

/**
 * 获取存档槽数据
 * 
 * 实现流程：
 * 1. 检查存档槽是否存在
 * 2. 如果存在：从磁盘加载存档数据
 * 3. 如果不存在：创建新的空存档对象（所有成员为默认值）
 * 4. 将 USaveGame* 转换为 ULoadScreenSaveGame* 并返回
 * 
 * @param SlotName  存档槽名称（如 "SaveSlot"）
 * @param SlotIndex 存档槽索引（0、1、2）
 * @return 存档数据对象指针：
 *         - 如果存档存在：返回加载的存档数据
 *         - 如果存档不存在：返回新创建的空存档对象（所有成员为默认值）
 * 
 * 使用场景：
 * - 加载界面读取存档槽信息（显示存档名称、地图等）
 * - 游戏内保存进度时获取当前存档数据
 * - 首次进入游戏时创建新存档
 * 
 * 注意：
 * - 返回的对象生命周期由调用者管理
 * - 如果存档不存在，返回的是空对象，需要调用者填充数据后再保存
 */
ULoadScreenSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	
	// 检查存档槽是否存在
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// 存档存在：从磁盘加载存档数据
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		// 存档不存在：创建新的空存档对象（所有成员为默认值）
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	
	// 将基类指针转换为具体类型并返回
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;
}

/**
 * 删除指定存档槽
 * 
 * 实现流程：
 * 1. 检查存档槽是否存在
 * 2. 如果存在：调用引擎 API 删除存档文件
 * 3. 如果不存在：不执行任何操作（静默失败）
 * 
 * @param SlotName  存档槽名称（如 "SaveSlot"）
 * @param SlotIndex 存档槽索引（0、1、2）
 * 
 * 使用场景：
 * - 在加载界面删除存档槽
 * - 覆盖存档前清理旧数据（SaveSlotData 中已包含此逻辑）
 * 
 * 注意：
 * - 删除操作不可逆，删除后存档数据无法恢复
 * - 删除后存档槽状态会变为 Vacant（空槽）
 */
void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// 检查存档槽是否存在
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// 删除存档文件（从磁盘永久删除）
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
	// 如果存档不存在，不执行任何操作（静默失败）
}

/**
 * 获取当前游戏会话的存档数据
 * 
 * 实现流程：
 * 1. 从 GameInstance 获取当前存档槽的名称和索引
 * 2. 调用 GetSaveSlotData 加载或创建存档数据
 * 
 * @return 当前游戏会话的存档数据对象指针
 * 
 * 使用场景：
 * - 游戏内保存进度时获取当前存档数据
 * - 玩家死亡后重生时获取存档数据（确定重生位置）
 * - 加载玩家进度时获取存档数据（恢复等级、XP、技能等）
 * 
 * 注意：
 * - GameInstance 在游戏启动时设置 LoadSlotName 和 LoadSlotIndex
 * - 这些值在游戏会话期间保持不变（直到切换存档槽）
 */
ULoadScreenSaveGame* AAuraGameModeBase::RetrieveInGameSaveData()
{
	// 获取 GameInstance（存储当前存档槽信息）
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	// 从 GameInstance 获取当前存档槽的名称和索引
	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	// 加载或创建存档数据
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

/**
 * 保存游戏内进度数据
 * 
 * 实现流程：
 * 1. 从 GameInstance 获取当前存档槽的名称和索引
 * 2. 将存档对象中的 PlayerStartTag 同步到 GameInstance（用于重生位置）
 * 3. 将存档对象序列化并保存到磁盘
 * 
 * @param SaveObject 已填充数据的存档对象（包含玩家进度、技能、关卡状态等）
 * 
 * 使用场景：
 * - 玩家手动保存游戏时
 * - 自动保存系统定期保存时
 * - 切换地图前保存当前进度时
 * 
 * 注意：
 * - SaveObject 应该已经填充了所有需要保存的数据：
 *   - 玩家进度：等级、XP、属性点、技能点、主属性值
 *   - 技能数据：所有已解锁/装备的技能状态
 *   - 关卡状态：当前地图中所有 Actor 的状态（通过 SaveWorldState 保存）
 * - PlayerStartTag 会同步到 GameInstance，用于玩家重生时确定出生点
 */
void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	// 获取 GameInstance（存储当前存档槽信息）
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	// 从 GameInstance 获取当前存档槽的名称和索引
	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;
	
	// 同步 PlayerStartTag 到 GameInstance（用于玩家重生时确定出生点）
	AuraGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	// 将存档对象序列化并保存到磁盘
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

/**
 * 保存关卡状态（序列化所有实现了 SaveInterface 的 Actor）
 * 
 * 实现流程：
 * 1. 获取当前关卡名称（移除 StreamingLevelsPrefix 前缀）
 * 2. 获取当前存档数据对象
 * 3. 如果指定了目标地图资产名称，更新存档中的地图信息
 * 4. 如果该地图的存档数据不存在，创建新的 FSavedMap 条目
 * 5. 遍历关卡中所有 Actor：
 *    a. 检查 Actor 是否有效且实现了 SaveInterface
 *    b. 保存 Actor 的名称和世界变换
 *    c. 使用 FObjectAndNameAsStringProxyArchive 序列化 Actor 的属性
 *       （只序列化标记了 SaveGame 说明符的 UPROPERTY）
 *    d. 将序列化数据保存到 FSavedActor.Bytes 中
 * 6. 更新存档数据中的地图条目
 * 7. 将存档数据保存到磁盘
 * 
 * @param World                   要保存的关卡世界对象
 * @param DestinationMapAssetName 目标地图资产名称（可选，用于更新存档中的地图信息）
 * 
 * 序列化说明：
 * - 只有实现了 SaveInterface 的 Actor 才会被保存
 * - 只有标记了 SaveGame 说明符的 UPROPERTY 才会被序列化
 * - Actor 的 Transform（位置、旋转、缩放）总是被保存
 * - 序列化数据以二进制格式存储在 FSavedActor.Bytes 中
 * 
 * 使用场景：
 * - 玩家手动保存游戏时
 * - 自动保存系统定期保存时
 * - 切换地图前保存当前关卡状态时
 * 
 * 注意：
 * - 此函数会覆盖该地图之前保存的所有 Actor 状态
 * - 序列化过程可能较慢，建议在后台线程或异步执行
 */
void AAuraGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{
	// 获取关卡名称（移除 StreamingLevelsPrefix 前缀，如 "UEDPIE_0_"）
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	// 获取 GameInstance（存储当前存档槽信息）
	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);

	// 获取当前存档数据对象
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		// 如果指定了目标地图资产名称，更新存档中的地图信息
		if (DestinationMapAssetName != FString(""))
		{
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}
		
		// 如果该地图的存档数据不存在，创建新的 FSavedMap 条目
		if (!SaveGame->HasMap(WorldName))
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}

		// 获取该地图的存档数据（如果不存在则返回空结构体）
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // 清空现有 Actor 数据，重新填充

		// 遍历关卡中所有 Actor
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			// 跳过无效的 Actor 或未实现 SaveInterface 的 Actor
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			// 创建 Actor 存档数据结构
			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();  // 保存 Actor 名称（用于加载时查找）
			SavedActor.Transform = Actor->GetTransform(); // 保存世界变换（位置、旋转、缩放）

			// 创建内存写入器（用于序列化）
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			// 创建序列化归档器
			// FObjectAndNameAsStringProxyArchive 会将对象属性序列化为二进制数据
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true; // 标记为存档序列化（只序列化 SaveGame 属性）

			// 序列化 Actor（只序列化标记了 SaveGame 说明符的 UPROPERTY）
			Actor->Serialize(Archive);

			// 将序列化后的 Actor 数据添加到地图存档中
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		// 更新存档数据中的地图条目（替换旧数据）
		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}
		
		// 将更新后的存档数据保存到磁盘
		UGameplayStatics::SaveGameToSlot(SaveGame, AuraGI->LoadSlotName, AuraGI->LoadSlotIndex);
	}
}

/**
 * 加载关卡状态（反序列化所有实现了 SaveInterface 的 Actor）
 * 
 * 实现流程：
 * 1. 获取当前关卡名称（移除 StreamingLevelsPrefix 前缀）
 * 2. 检查存档是否存在
 * 3. 从磁盘加载存档数据
 * 4. 遍历关卡中所有 Actor：
 *    a. 检查 Actor 是否实现了 SaveInterface
 *    b. 在存档数据中查找匹配的 Actor（按名称匹配）
 *    c. 如果找到匹配的 Actor：
 *       - 根据 ShouldLoadTransform 决定是否恢复世界变换
 *       - 使用 FObjectAndNameAsStringProxyArchive 反序列化 Actor 的属性
 *       - 调用 LoadActor 接口函数，让 Actor 执行加载后的初始化逻辑
 * 
 * @param World 要加载的关卡世界对象
 * 
 * 反序列化说明：
 * - 只有实现了 SaveInterface 的 Actor 才会被加载
 * - Actor 必须与存档中的名称完全匹配才能恢复状态
 * - 反序列化会恢复所有标记了 SaveGame 说明符的 UPROPERTY
 * - LoadActor 接口函数允许 Actor 执行自定义的加载后初始化逻辑
 * 
 * 使用场景：
 * - 玩家加载存档时恢复关卡状态
 * - 切换地图后恢复之前保存的 Actor 状态
 * - 玩家死亡后重生时恢复关卡状态
 * 
 * 注意：
 * - 此函数在关卡加载完成后调用（通常在 GameMode::BeginPlay 或 PostInitializeComponents 中）
 * - 如果存档不存在或加载失败，函数会静默返回（不恢复任何状态）
 * - Actor 的 Transform 恢复是可选的（由 ShouldLoadTransform 决定）
 */
void AAuraGameModeBase::LoadWorldState(UWorld* World) const
{
	// 获取关卡名称（移除 StreamingLevelsPrefix 前缀）
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	// 获取 GameInstance（存储当前存档槽信息）
	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);

	// 检查存档是否存在
	if (UGameplayStatics::DoesSaveGameExist(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		// 从磁盘加载存档数据
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogAura, Error, TEXT("Failed to load slot"));
			return;
		}
		
		// 遍历关卡中所有 Actor
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			// 跳过未实现 SaveInterface 的 Actor
			if (!Actor->Implements<USaveInterface>()) continue;

			// 在存档数据中查找匹配的 Actor（按名称匹配）
			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					// 根据 ShouldLoadTransform 决定是否恢复世界变换
					// 某些 Actor（如玩家）可能不需要恢复 Transform
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
					}

					// 创建内存读取器（用于反序列化）
					FMemoryReader MemoryReader(SavedActor.Bytes);

					// 创建反序列化归档器
					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true; // 标记为存档反序列化

					// 反序列化 Actor（将二进制数据恢复为属性值）
					Actor->Serialize(Archive);

					// 调用 LoadActor 接口函数，让 Actor 执行加载后的初始化逻辑
					// 例如：重新初始化组件、恢复状态机、重新激活效果等
					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
	}
}

/**
 * 切换到指定地图（关卡切换）
 * 
 * 实现流程：
 * 1. 从 LoadSlot ViewModel 获取存档槽名称和索引
 * 2. 从 Maps 映射表中查找对应的地图资产
 * 3. 使用 OpenLevelBySoftObjectPtr 切换到目标地图
 * 
 * @param Slot 包含地图信息的 LoadSlot ViewModel
 * 
 * 使用场景：
 * - 在加载界面选择存档槽后切换到对应地图
 * - 玩家通过地图入口切换到其他地图时
 * 
 * 注意：
 * - 关卡切换会触发关卡卸载和加载流程
 * - 切换前会保存当前关卡状态（如果已实现）
 */
void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	// 获取存档槽名称和索引（用于后续保存/加载）
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	// 从 Maps 映射表中查找地图资产并切换关卡
	// Maps.FindChecked 会在找不到时触发断言（确保地图配置正确）
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

/**
 * 根据地图资产名称获取地图显示名称
 * 
 * 实现流程：
 * 1. 遍历 Maps 映射表
 * 2. 比较每个地图资产的资产名称与输入参数
 * 3. 找到匹配的地图则返回对应的显示名称（Map.Key）
 * 4. 如果未找到，返回空字符串
 * 
 * @param MapAssetName 地图资产名称（如 "/Game/Maps/Dungeon"）
 * @return 地图显示名称（如 "Dungeon"），如果未找到则返回空字符串
 * 
 * 使用场景：
 * - 保存存档时，将地图资产名称转换为显示名称
 * - 在 UI 中显示地图名称时
 */
FString AAuraGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	// 遍历 Maps 映射表（Key=显示名称, Value=地图资产引用）
	for (auto& Map : Maps)
	{
		// 比较地图资产的资产名称（不包含路径）
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			// 找到匹配的地图，返回显示名称
			return Map.Key;
		}
	}
	// 未找到匹配的地图，返回空字符串
	return FString();
}

/**
 * 选择玩家出生点（重写 GameMode 的默认实现）
 * 
 * 实现流程：
 * 1. 从 GameInstance 获取 PlayerStartTag（记录玩家最近到达的检查点）
 * 2. 获取关卡中所有 PlayerStart Actor
 * 3. 遍历所有 PlayerStart，查找 PlayerStartTag 匹配的出生点
 * 4. 如果找到匹配的出生点，返回该出生点
 * 5. 如果未找到匹配的出生点，返回第一个出生点（默认出生点）
 * 
 * @param Player 玩家控制器
 * @return 选中的玩家出生点 Actor，如果未找到则返回 nullptr
 * 
 * 使用场景：
 * - 玩家首次进入关卡时确定出生位置
 * - 玩家死亡后重生时确定重生位置
 * - 从存档加载时恢复玩家位置
 * 
 * 注意：
 * - PlayerStartTag 在玩家到达检查点时更新
 * - 如果关卡中没有 PlayerStart，函数返回 nullptr（可能导致玩家无法生成）
 */
AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 获取 GameInstance（存储 PlayerStartTag）
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	
	// 获取关卡中所有 PlayerStart Actor
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	
	if (Actors.Num() > 0)
	{
		// 默认选择第一个出生点
		AActor* SelectedActor = Actors[0];
		
		// 遍历所有出生点，查找 PlayerStartTag 匹配的出生点
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// 如果找到匹配的出生点，选择它并退出循环
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	
	// 如果关卡中没有 PlayerStart，返回 nullptr
	return nullptr;
}

/**
 * 处理玩家死亡后的重生逻辑
 * 
 * 实现流程：
 * 1. 获取当前存档数据
 * 2. 如果存档数据有效，切换到存档中记录的地图
 * 3. 玩家会在新地图的出生点重生（由 ChoosePlayerStart 确定位置）
 * 
 * @param DeadCharacter 死亡的玩家角色
 * 
 * 使用场景：
 * - 玩家死亡后自动重生
 * - 玩家选择"重新开始"时
 * 
 * 注意：
 * - 此函数会触发关卡切换，玩家会回到存档中记录的地图
 * - 如果存档数据无效，函数会静默返回（玩家不会重生）
 */
void AAuraGameModeBase::PlayerDied(ACharacter* DeadCharacter)
{
	// 获取当前存档数据
	ULoadScreenSaveGame* SaveGame = RetrieveInGameSaveData();
	if (!IsValid(SaveGame)) return;

	// 切换到存档中记录的地图（玩家会在该地图的出生点重生）
	UGameplayStatics::OpenLevel(DeadCharacter, FName(SaveGame->MapAssetName));
}

/**
 * 游戏模式开始时的初始化
 * 
 * 实现流程：
 * 1. 调用父类的 BeginPlay
 * 2. 将默认地图添加到 Maps 映射表中
 * 
 * 使用场景：
 * - 游戏模式初始化时设置默认地图配置
 * 
 * 注意：
 * - 默认地图在 Details 面板中配置
 * - Maps 映射表用于关卡切换和地图名称查找
 */
void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	// 将默认地图添加到 Maps 映射表（Key=显示名称, Value=地图资产引用）
	Maps.Add(DefaultMapName, DefaultMap);
}
