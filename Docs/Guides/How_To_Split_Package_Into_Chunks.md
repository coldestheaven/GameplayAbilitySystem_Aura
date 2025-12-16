# 如何将包分为两个部分以减少首包大小

## 概述

本文档详细说明如何将 Aura 项目的游戏包分为两个部分（基础包 + 扩展包），以减少首包大小，实现流式下载和 DLC 支持。

---

## 1. 分块策略

### 1.1 资产分类

将游戏资产分为两类：

#### 基础包（Chunk 0）- 必需内容
- **核心系统**: Gameplay Tags、GAS Globals
- **基础数据资产**: CharacterClassInfo、AbilityInfo、AttributeInfo
- **主菜单**: MainMenu、LoadMenu
- **基础地图**: 第一个可玩地图
- **核心能力**: 基础攻击能力
- **UI 系统**: 所有 UI 资源

#### 扩展包（Chunk 1）- 可选内容
- **额外地图**: DungeonMap_One、DungeonMap_Two 等
- **高级能力**: 高级技能和被动能力
- **额外内容**: DLC 内容、额外资源

### 1.2 分块目标

- **首包大小**: 减少 30-50%
- **加载时间**: 减少初始加载时间
- **用户体验**: 快速进入游戏，后台下载扩展内容

---

## 2. 配置 Chunking 系统

### 2.1 启用 Chunk 生成

#### 步骤 1: 修改 DefaultGame.ini

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
# 启用 Chunk 生成
bGenerateChunks=True

# 每个 Chunk 的最大大小（字节，0 = 无限制）
MaxChunkSize=0

# 只对硬引用进行分块
bChunkHardReferencesOnly=False

# 强制每个文件一个 Chunk
bForceOneChunkPerFile=False

# 其他配置保持不变
UsePakFile=True
bUseIoStore=True
bCompressed=True
```

#### 步骤 2: 验证配置

在项目设置中验证：
1. Edit → Project Settings
2. Packaging → Build
3. 确保 "Generate Chunks" 已启用

---

## 3. 实现 Primary Assets 系统

### 3.1 扩展 AuraAssetManager

#### 步骤 1: 定义 Primary Asset 类型

```cpp
// 在 AuraAssetManager.h 中
class UAuraAssetManager : public UAssetManager
{
    GENERATED_BODY()
    
public:
    static UAuraAssetManager& Get();
    
    // Primary Asset 类型定义
    static const FPrimaryAssetType CharacterClassInfoType;
    static const FPrimaryAssetType AbilityInfoType;
    static const FPrimaryAssetType AttributeInfoType;
    static const FPrimaryAssetType MapType;
    
    // Bundle 类型定义
    static const FName BaseGameBundle;      // 基础游戏内容
    static const FName ExpansionBundle;     // 扩展内容
    
    // 获取 Primary Asset
    static UCharacterClassInfo* GetCharacterClassInfo(const FPrimaryAssetId& AssetId);
    static UAbilityInfo* GetAbilityInfo(const FPrimaryAssetId& AssetId);
    
protected:
    virtual void StartInitialLoading() override;
    
    // 扫描和注册 Primary Assets
    void ScanForPrimaryAssets();
};
```

#### 步骤 2: 实现 Primary Asset 扫描

```cpp
// 在 AuraAssetManager.cpp 中
const FPrimaryAssetType UAuraAssetManager::CharacterClassInfoType = 
    TEXT("CharacterClassInfo");
const FPrimaryAssetType UAuraAssetManager::AbilityInfoType = 
    TEXT("AbilityInfo");
const FPrimaryAssetType UAuraAssetManager::AttributeInfoType = 
    TEXT("AttributeInfo");
const FPrimaryAssetType UAuraAssetManager::MapType = 
    TEXT("Map");

const FName UAuraAssetManager::BaseGameBundle = TEXT("BaseGame");
const FName UAuraAssetManager::ExpansionBundle = TEXT("Expansion");

void UAuraAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // 初始化 Gameplay Tags
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // 初始化 GAS Globals
    UAbilitySystemGlobals::Get().InitGlobalData();
    
    // 扫描 Primary Assets
    ScanForPrimaryAssets();
}

void UAuraAssetManager::ScanForPrimaryAssets()
{
    // 扫描数据资产
    ScanPathForPrimaryAssets(
        CharacterClassInfoType,
        TEXT("/Game/DataAssets"),
        UCharacterClassInfo::StaticClass(),
        false
    );
    
    ScanPathForPrimaryAssets(
        AbilityInfoType,
        TEXT("/Game/DataAssets"),
        UAbilityInfo::StaticClass(),
        false
    );
    
    ScanPathForPrimaryAssets(
        AttributeInfoType,
        TEXT("/Game/DataAssets"),
        UAttributeInfo::StaticClass(),
        false
    );
    
    // 扫描地图（可选，如果使用 Primary Assets 管理地图）
    // ScanPathForPrimaryAssets(
    //     MapType,
    //     TEXT("/Game/Maps"),
    //     UWorld::StaticClass(),
    //     false
    // );
}
```

---

## 4. 标记资产的 Chunk ID

### 4.1 数据资产标记

#### 方法 1: 在数据资产类中标记

```cpp
// 在 CharacterClassInfo.h 中
UCLASS()
class UCharacterClassInfo : public UDataAsset
{
    GENERATED_BODY()
    
public:
    // 标记为 Primary Asset
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(
            UAuraAssetManager::CharacterClassInfoType,
            GetFName()
        );
    }
    
    // 指定 Chunk ID（基础内容使用 0）
    virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override
    {
        Super::GetAssetRegistryTags(OutTags);
        
        OutTags.Add(FAssetRegistryTag(
            "ChunkID",
            TEXT("0"),  // 基础包
            FAssetRegistryTag::TT_Numerical
        ));
    }
};
```

#### 方法 2: 在资产编辑器中标记

1. **打开资产**
   - 在 Content Browser 中选择资产
   - 双击打开

2. **设置 Chunk ID**
   - 在资产详情面板中找到 "Chunk ID" 属性
   - 设置值：
     - **0**: 基础包
     - **1**: 扩展包

3. **批量设置**
   - 选择多个资产
   - 在 Details 面板中批量设置 Chunk ID

### 4.2 地图资产标记

#### 方法 1: 在 GameMode 中配置

```cpp
// 在 AuraGameModeBase.h 中
class AAuraGameModeBase : public AGameModeBase
{
    // 基础地图（Chunk 0）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Settings")
    TSoftObjectPtr<UWorld> DefaultMap;  // Chunk 0
    
    // 扩展地图（Chunk 1）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Settings|Expansion")
    TMap<FString, TSoftObjectPtr<UWorld>> ExpansionMaps;  // Chunk 1
};
```

#### 方法 2: 使用 Asset Registry Tags

在资产编辑器中为每个地图设置 Chunk ID：
- MainMenu: Chunk 0
- LoadMenu: Chunk 0
- DungeonMap_One: Chunk 1
- DungeonMap_Two: Chunk 1

---

## 5. 配置 Cook 设置

### 5.1 修改 DefaultGame.ini

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
# Cook 设置
bCookAll=False
bCookMapsOnly=False

# 指定要 Cook 的地图
+MapsToCook=(FilePath="/Game/Maps/MainMenu")
+MapsToCook=(FilePath="/Game/Maps/LoadMenu")
# 基础地图（Chunk 0）
+MapsToCook=(FilePath="/Game/Maps/BaseMap")

# 扩展地图（Chunk 1）- 可选
+MapsToCook=(FilePath="/Game/Maps/Dungeons/DungeonMap_One")
+MapsToCook=(FilePath="/Game/Maps/Dungeons/DungeonMap_Two")
```

### 5.2 使用 Chunk 指定

在 Cook 时指定 Chunk：

```ini
# 基础包（Chunk 0）
+ChunkList=0

# 扩展包（Chunk 1）
+ChunkList=1
```

---

## 6. 运行时加载扩展内容

### 6.1 检查 Chunk 是否已安装

```cpp
// 在 AuraGameModeBase.h 中
class AAuraGameModeBase : public AGameModeBase
{
public:
    // 检查 Chunk 是否已安装
    UFUNCTION(BlueprintCallable)
    bool IsChunkInstalled(int32 ChunkID) const;
    
    // 加载 Chunk
    UFUNCTION(BlueprintCallable)
    void LoadChunk(int32 ChunkID, FStreamableDelegate OnLoaded);
    
    // 卸载 Chunk
    UFUNCTION(BlueprintCallable)
    void UnloadChunk(int32 ChunkID);
};
```

### 6.2 实现 Chunk 加载

```cpp
// 在 AuraGameModeBase.cpp 中
bool AAuraGameModeBase::IsChunkInstalled(int32 ChunkID) const
{
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    // 检查 Chunk 是否可用
    TArray<int32> AvailableChunks;
    AssetManager.GetChunkIDs(AvailableChunks);
    
    return AvailableChunks.Contains(ChunkID);
}

void AAuraGameModeBase::LoadChunk(int32 ChunkID, FStreamableDelegate OnLoaded)
{
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    TArray<int32> ChunkIDs;
    ChunkIDs.Add(ChunkID);
    
    AssetManager.LoadChunks(
        ChunkIDs,
        OnLoaded
    );
}

void AAuraGameModeBase::UnloadChunk(int32 ChunkID)
{
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    TArray<int32> ChunkIDs;
    ChunkIDs.Add(ChunkID);
    
    AssetManager.UnloadChunks(ChunkIDs);
}
```

### 6.3 在 MapEntrance 中使用

```cpp
// 在 MapEntrance.cpp 中
void AMapEntrance::OnSphereOverlap(...)
{
    if (APawn* OverlappingPawn = Cast<APawn>(OtherActor))
    {
        if (OverlappingPawn->IsPlayerControlled())
        {
            AAuraGameModeBase* AuraGM = 
                Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
            
            if (AuraGM)
            {
                // 检查是否需要加载扩展包
                int32 RequiredChunkID = GetRequiredChunkID();
                
                if (RequiredChunkID > 0)
                {
                    // 检查 Chunk 是否已安装
                    if (!AuraGM->IsChunkInstalled(RequiredChunkID))
                    {
                        // 提示用户下载扩展内容
                        ShowDownloadPrompt(RequiredChunkID);
                        return;
                    }
                    
                    // 加载 Chunk
                    AuraGM->LoadChunk(RequiredChunkID, 
                        FStreamableDelegate::CreateUObject(
                            this,
                            &AMapEntrance::OnChunkLoaded
                        )
                    );
                }
                else
                {
                    // 直接切换地图（基础包）
                    SwitchToMap();
                }
            }
        }
    }
}

int32 AMapEntrance::GetRequiredChunkID() const
{
    // 根据目标地图确定所需的 Chunk ID
    FString MapName = DestinationMap.ToSoftObjectPath().GetAssetName();
    
    // 扩展地图使用 Chunk 1
    if (MapName.Contains(TEXT("Dungeon")))
    {
        return 1;
    }
    
    // 基础地图使用 Chunk 0
    return 0;
}

void AMapEntrance::OnChunkLoaded()
{
    // Chunk 加载完成后切换地图
    SwitchToMap();
}

void AMapEntrance::SwitchToMap()
{
    UGameplayStatics::OpenLevelBySoftObjectPtr(
        this,
        DestinationMap,
        true,
        DestinationPlayerStartTag.ToString()
    );
}
```

---

## 7. 使用 Asset Bundles 进行精细控制

### 7.1 定义 Bundle

```cpp
// 在 AuraAssetManager.h 中
class UAuraAssetManager : public UAssetManager
{
public:
    // Bundle 类型
    static const FName BaseGameBundle;      // 基础游戏
    static const FName ExpansionBundle;     // 扩展内容
    static const FName EssentialBundle;     // 必需内容（总是加载）
};
```

### 7.2 在数据资产中指定 Bundle

```cpp
// 在 CharacterClassInfo.h 中
UCLASS()
class UCharacterClassInfo : public UDataAsset
{
    GENERATED_BODY()
    
public:
    virtual void GetAssetBundles(TArray<FAssetBundleEntry>& OutBundles) const override
    {
        Super::GetAssetBundles(OutBundles);
        
        // 添加到基础游戏 Bundle
        FAssetBundleEntry Entry;
        Entry.BundleName = UAuraAssetManager::BaseGameBundle;
        Entry.BundleAssets.Add(FPrimaryAssetId(
            UAuraAssetManager::CharacterClassInfoType,
            GetFName()
        ));
        OutBundles.Add(Entry);
    }
};
```

### 7.3 按 Bundle 加载

```cpp
// 加载基础游戏 Bundle
void LoadBaseGameBundle()
{
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    TArray<FName> BundleNames;
    BundleNames.Add(UAuraAssetManager::BaseGameBundle);
    
    AssetManager.LoadPrimaryAssetsWithBundles(
        TArray<FPrimaryAssetId>(),  // 空数组表示加载所有匹配的资产
        BundleNames,
        FStreamableDelegate::CreateLambda([]()
        {
            UE_LOG(LogAura, Log, TEXT("Base game bundle loaded"));
        })
    );
}
```

---

## 8. 打包流程

### 8.1 命令行打包

#### 步骤 1: Cook 资产

```bash
# Cook 所有资产（包括所有 Chunk）
UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=cook 
    -targetplatform=Windows 
    -cookall
```

#### 步骤 2: 打包基础包（Chunk 0）

```bash
# 只打包 Chunk 0
UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=AutomationTool 
    -Script="Engine/Build/BuildAutomation.cs" 
    -ExecuteBuild 
    -Project="ProjectPath.uproject" 
    -Target="ProjectName Windows Development" 
    -TargetPlatform=Win64 
    -Configuration=Development
    -ChunkList=0
```

#### 步骤 3: 打包扩展包（Chunk 1）

```bash
# 打包 Chunk 1（作为 DLC 或补丁）
UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=AutomationTool 
    -Script="Engine/Build/BuildAutomation.cs" 
    -ExecuteBuild 
    -Project="ProjectPath.uproject" 
    -Target="ProjectName Windows Development" 
    -TargetPlatform=Win64 
    -Configuration=Development
    -ChunkList=1
```

### 8.2 编辑器打包

#### 步骤 1: 配置打包设置

1. **File → Package Project → Windows → Windows (64-bit)**
2. **在打包设置中**:
   - 启用 "Generate Chunks"
   - 设置 Chunk 列表

#### 步骤 2: 分别打包

1. **基础包**:
   - 选择 "Chunk 0"
   - 打包到 `BasePackage` 目录

2. **扩展包**:
   - 选择 "Chunk 1"
   - 打包到 `ExpansionPackage` 目录

---

## 9. 分发策略

### 9.1 Steam 分发

#### 基础包（必需）
- 作为主游戏包
- 用户必须下载

#### 扩展包（可选）
- 作为 DLC 或可选下载
- 用户可以选择性下载

### 9.2 Epic Games Store 分发

#### 基础包
- 主游戏包
- 必需下载

#### 扩展包
- 作为可选内容
- 后台下载或按需下载

### 9.3 自定义分发

```cpp
// 实现自定义下载系统
class AURA_API AChunkDownloadManager : public AActor
{
    GENERATED_BODY()
    
public:
    // 检查 Chunk 是否需要下载
    UFUNCTION(BlueprintCallable)
    bool NeedsDownload(int32 ChunkID);
    
    // 下载 Chunk
    UFUNCTION(BlueprintCallable)
    void DownloadChunk(int32 ChunkID, FOnChunkDownloaded OnDownloaded);
    
    // 获取下载进度
    UFUNCTION(BlueprintCallable)
    float GetDownloadProgress(int32 ChunkID);
};
```

---

## 10. 测试和验证

### 10.1 测试检查清单

- [ ] 基础包可以独立运行
- [ ] 扩展包可以正确加载
- [ ] Chunk 加载/卸载正常
- [ ] 地图切换正常
- [ ] 数据资产访问正常
- [ ] 内存使用正常
- [ ] 性能表现正常

### 10.2 验证 Chunk 内容

```cpp
// 验证 Chunk 内容
void VerifyChunkContents()
{
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    // 获取所有已安装的 Chunk
    TArray<int32> InstalledChunks;
    AssetManager.GetChunkIDs(InstalledChunks);
    
    UE_LOG(LogAura, Log, TEXT("Installed Chunks: %d"), InstalledChunks.Num());
    
    for (int32 ChunkID : InstalledChunks)
    {
        // 获取 Chunk 中的资产
        TArray<FPrimaryAssetId> Assets;
        AssetManager.GetPrimaryAssetIdList(
            FPrimaryAssetType(),
            Assets,
            ChunkID
        );
        
        UE_LOG(LogAura, Log, TEXT("Chunk %d contains %d assets"), 
            ChunkID, Assets.Num());
    }
}
```

---

## 11. 优化建议

### 11.1 减少基础包大小

#### 优化策略

1. **移除未使用的资产**
   - 检查资产引用
   - 移除未使用的资源

2. **压缩资产**
   - 使用更高的压缩级别
   - 优化纹理大小

3. **延迟加载**
   - 将非关键资产移到扩展包
   - 使用软引用

### 11.2 智能加载

```cpp
// 预加载关键资产
void PreloadEssentialAssets()
{
    // 加载基础游戏 Bundle
    LoadBaseGameBundle();
    
    // 后台加载扩展内容（如果已安装）
    if (IsChunkInstalled(1))
    {
        LoadChunk(1, FStreamableDelegate());
    }
}
```

### 11.3 内存管理

```cpp
// 卸载不需要的 Chunk
void UnloadUnusedChunks()
{
    // 如果不在扩展地图中，卸载扩展包
    if (!IsInExpansionMap())
    {
        UnloadChunk(1);
    }
}
```

---

## 12. 常见问题

### Q1: Chunk 加载失败？

**A**: 检查：
1. Chunk 是否已正确打包
2. Chunk ID 是否正确
3. 资产路径是否正确

### Q2: 基础包无法运行？

**A**: 检查：
1. 所有必需资产是否在 Chunk 0
2. 数据资产引用是否正确
3. Gameplay Tags 是否已初始化

### Q3: 扩展包无法加载？

**A**: 检查：
1. Chunk 是否已安装
2. 加载代码是否正确
3. 资产路径是否正确

---

## 13. 完整示例

### 13.1 基础包配置示例

```ini
# DefaultGame.ini
[/Script/UnrealEd.ProjectPackagingSettings]
bGenerateChunks=True
MaxChunkSize=0

# 基础包地图
+MapsToCook=(FilePath="/Game/Maps/MainMenu")
+MapsToCook=(FilePath="/Game/Maps/LoadMenu")
+MapsToCook=(FilePath="/Game/Maps/BaseMap")
```

### 13.2 扩展包配置示例

```ini
# 扩展包地图（在单独的配置文件中）
+MapsToCook=(FilePath="/Game/Maps/Dungeons/DungeonMap_One")
+MapsToCook=(FilePath="/Game/Maps/Dungeons/DungeonMap_Two")
```

### 13.3 运行时加载示例

```cpp
// 在游戏开始时
void AAuraGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    
    // 加载基础包（Chunk 0）
    LoadChunk(0, FStreamableDelegate::CreateUObject(
        this,
        &AAuraGameModeBase::OnBaseChunkLoaded
    ));
}

void AAuraGameModeBase::OnBaseChunkLoaded()
{
    // 基础包加载完成
    UE_LOG(LogAura, Log, TEXT("Base chunk loaded"));
    
    // 检查扩展包是否可用
    if (IsChunkInstalled(1))
    {
        // 后台加载扩展包
        LoadChunk(1, FStreamableDelegate());
    }
}
```

---

## 14. 总结

通过将游戏包分为两个部分：

1. ✅ **减少首包大小** - 30-50% 减少
2. ✅ **快速启动** - 用户更快进入游戏
3. ✅ **按需下载** - 扩展内容按需加载
4. ✅ **DLC 支持** - 支持后续内容扩展
5. ✅ **更好的用户体验** - 减少等待时间

### 实施步骤总结

1. **配置 Chunking** - 启用 `bGenerateChunks=True`
2. **标记资产** - 为资产设置 Chunk ID
3. **实现加载逻辑** - 添加 Chunk 加载/卸载代码
4. **测试验证** - 确保基础包和扩展包正常工作
5. **打包分发** - 分别打包基础包和扩展包

---

## 相关文档

- [资产打包文档](../Systems/Asset_Packaging.md) - 打包系统基础
- [资产管理详细文档](../Systems/Asset_Management_Details.md) - 资产管理细节
- [Asset Manager 系统](../Systems/Asset_Manager_System.md) - Asset Manager 基础

