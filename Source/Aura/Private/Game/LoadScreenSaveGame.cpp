// Copyright Druid Mechanics


#include "Game/LoadScreenSaveGame.h"

/**
 * 根据地图资产名称获取对应的地图存档数据
 * 
 * 实现流程：
 * 1. 遍历 SavedMaps 数组中的所有地图存档数据
 * 2. 比较每个地图的 MapAssetName 是否与输入参数匹配
 * 3. 找到匹配的地图则返回该地图的存档数据
 * 4. 如果未找到匹配的地图，返回一个空的 FSavedMap 结构体
 * 
 * @param InMapName 地图资产名称（如 "/Game/Maps/Dungeon"）
 * @return 匹配的 FSavedMap 结构体，如果不存在则返回空结构体（所有成员为默认值）
 * 
 * 使用示例：
 * FSavedMap DungeonMap = SaveGame->GetSavedMapWithMapName(TEXT("/Game/Maps/Dungeon"));
 * if (DungeonMap.MapAssetName.IsEmpty() == false)
 * {
 *     // 找到了该地图的存档数据，可以恢复其中的 Actor 状态
 * }
 */
FSavedMap ULoadScreenSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	// 遍历所有已保存的地图数据
	for (const FSavedMap& Map : SavedMaps)
	{
		// 检查地图资产名称是否匹配（精确匹配）
		if (Map.MapAssetName == InMapName)
		{
			// 找到匹配的地图，返回其存档数据
			return Map;
		}
	}
	// 未找到匹配的地图，返回空结构体（所有成员为默认值）
	return FSavedMap();
}

/**
 * 检查是否存在指定地图的存档数据
 * 
 * 实现流程：
 * 1. 遍历 SavedMaps 数组中的所有地图存档数据
 * 2. 比较每个地图的 MapAssetName 是否与输入参数匹配
 * 3. 找到匹配的地图则立即返回 true
 * 4. 如果遍历完所有地图都未找到匹配项，返回 false
 * 
 * @param InMapName 地图资产名称（如 "/Game/Maps/Dungeon"）
 * @return true 表示该地图有存档数据，false 表示该地图没有存档数据
 * 
 * 使用示例：
 * if (SaveGame->HasMap(TEXT("/Game/Maps/Dungeon")))
 * {
 *     // 该地图有存档数据，可以加载其中的 Actor 状态
 * }
 * else
 * {
 *     // 该地图没有存档数据，需要初始化默认状态
 * }
 */
bool ULoadScreenSaveGame::HasMap(const FString& InMapName)
{
	// 遍历所有已保存的地图数据
	for (const FSavedMap& Map : SavedMaps)
	{
		// 检查地图资产名称是否匹配（精确匹配）
		if (Map.MapAssetName == InMapName)
		{
			// 找到匹配的地图，返回 true
			return true;
		}
	}
	// 未找到匹配的地图，返回 false
	return false;
}
