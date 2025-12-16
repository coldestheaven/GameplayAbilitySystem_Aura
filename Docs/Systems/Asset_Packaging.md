# 资产打包文档

## 概述

本文档详细说明 Aura 项目中资产打包（Asset Packaging）的配置、策略和最佳实践。资产打包是 Unreal Engine 中管理游戏资源分发和加载的重要系统。

---

## 1. 打包系统概述

### 1.1 什么是资产打包

资产打包是将游戏资产组织、压缩和分发的过程，包括：
- **Primary Assets**: 主要资产类型定义
- **Asset Bundles**: 资产捆绑和分组
- **Chunking**: 资产分块（用于 DLC、流式下载等）
- **Compression**: 资产压缩
- **Cook**: 资产烹饪（转换为运行时格式）

### 1.2 打包流程

```
资产准备
    ↓
Primary Asset 定义
    ↓
Asset Bundle 配置
    ↓
Cook（烹饪）
    ↓
Chunking（分块）
    ↓
Compression（压缩）
    ↓
Package（打包）
    ↓
分发
```

---

## 2. 项目打包配置

### 2.1 DefaultGame.ini 配置

项目中的打包配置位于 `Config/DefaultGame.ini`：

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
Build=IfProjectHasCode
BuildConfiguration=PPBC_Development
UsePakFile=True
bUseIoStore=True
bUseZenStore=False
bMakeBinaryConfig=False
bGenerateChunks=False
bGenerateNoChunks=False
bChunkHardReferencesOnly=False
bForceOneChunkPerFile=False
MaxChunkSize=0
bBuildHttpChunkInstallData=False
bCompressed=True
PackageCompressionFormat=Oodle
PackageCompressionMethod=Kraken
PackageCompressionLevel_DebugDevelopment=4
PackageCompressionLevel_TestShipping=5
PackageCompressionLevel_Distribution=7
```

### 2.2 关键配置说明

#### 2.2.1 Pak 文件

```ini
UsePakFile=True
```

**说明**: 使用 Pak 文件打包资产
- **优点**: 减少文件数量，提高加载速度
- **缺点**: 需要解压，增加内存使用

#### 2.2.2 IoStore

```ini
bUseIoStore=True
```

**说明**: 使用 IoStore 系统（UE5 新特性）
- **优点**: 更快的加载速度，更好的压缩
- **要求**: UE5.0+

#### 2.2.3 压缩

```ini
bCompressed=True
PackageCompressionFormat=Oodle
PackageCompressionMethod=Kraken
```

**说明**: 使用 Oodle Kraken 压缩
- **压缩级别**:
  - Debug/Development: 4（快速）
  - Test/Shipping: 5（平衡）
  - Distribution: 7（最大压缩）

---

## 3. Primary Assets 系统

### 3.1 什么是 Primary Assets

Primary Assets 是 UE5 中用于标识和管理主要游戏资产的系统。每个 Primary Asset 有：
- **Type**: 资产类型（如 CharacterClassInfo, AbilityInfo）
- **Name**: 资产名称
- **Chunk ID**: 所属的 Chunk

### 3.2 当前项目状态

**注意**: Aura 项目目前**未使用** Primary Assets 系统，数据资产通过 GameMode 直接引用。

### 3.3 如何添加 Primary Assets 支持

如果需要添加 Primary Assets 支持：

#### 步骤 1: 定义 Primary Asset 类型

```cpp
// 在 AuraAssetManager.h 中
class UAuraAssetManager : public UAssetManager
{
public:
    // 定义 Primary Asset 类型
    static const FPrimaryAssetType CharacterClassInfoType;
    static const FPrimaryAssetType AbilityInfoType;
    static const FPrimaryAssetType AttributeInfoType;
    
    // 获取 Primary Asset
    static UCharacterClassInfo* GetCharacterClassInfo(const FPrimaryAssetId& AssetId);
    
protected:
    virtual void StartInitialLoading() override;
    
    // 扫描和注册 Primary Assets
    virtual void StartInitialLoading() override;
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

void UAuraAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // 初始化 Gameplay Tags
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // 初始化 GAS Globals
    UAbilitySystemGlobals::Get().InitGlobalData();
    
    // 扫描 Primary Assets
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
}
```

#### 步骤 3: 标记数据资产

在数据资产类中添加 Primary Asset 标记：

```cpp
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
};
```

#### 步骤 4: 使用 Primary Asset 加载

```cpp
// 异步加载
void LoadCharacterClassInfo()
{
    FPrimaryAssetId AssetId(
        UAuraAssetManager::CharacterClassInfoType,
        TEXT("DA_CharacterClassInfo")
    );
    
    TArray<FName> Bundles;
    FStreamableDelegate Delegate;
    Delegate.BindLambda([]()
    {
        // 加载完成后的处理
    });
    
    UAuraAssetManager::Get().LoadPrimaryAsset(
        AssetId,
        Bundles,
        Delegate
    );
}
```

---

## 4. Asset Bundles

### 4.1 什么是 Asset Bundles

Asset Bundles 用于将相关资产分组，实现按需加载。

### 4.2 Bundle 类型

可以定义不同的 Bundle 类型：

```cpp
// 在 AuraAssetManager.h 中
class UAuraAssetManager : public UAssetManager
{
public:
    // Bundle 类型定义
    static const FName CharacterClassBundle;
    static const FName AbilityBundle;
    static const FName MapBundle;
};
```

### 4.3 使用示例

```cpp
// 加载特定 Bundle
void LoadCharacterClassBundle()
{
    FPrimaryAssetId AssetId(
        UAuraAssetManager::CharacterClassInfoType,
        TEXT("DA_CharacterClassInfo")
    );
    
    TArray<FName> Bundles;
    Bundles.Add(UAuraAssetManager::CharacterClassBundle);
    
    UAuraAssetManager::Get().LoadPrimaryAsset(
        AssetId,
        Bundles,
        FStreamableDelegate()
    );
}
```

---

## 5. Chunking（分块）

### 5.1 什么是 Chunking

Chunking 将资产分成多个块（Chunk），用于：
- **DLC 内容**: 将 DLC 内容放在单独的 Chunk
- **流式下载**: 按需下载特定 Chunk
- **平台优化**: 不同平台使用不同的 Chunk

### 5.2 当前配置

```ini
bGenerateChunks=False
bGenerateNoChunks=False
bChunkHardReferencesOnly=False
bForceOneChunkPerFile=False
MaxChunkSize=0
```

**说明**: 当前项目**未启用** Chunking。

### 5.3 如何启用 Chunking

#### 步骤 1: 启用 Chunk 生成

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
bGenerateChunks=True
MaxChunkSize=2097152  # 2MB per chunk
```

#### 步骤 2: 在代码中指定 Chunk ID

```cpp
// 在数据资产中
UCLASS()
class UCharacterClassInfo : public UDataAsset
{
    GENERATED_BODY()
    
public:
    virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override
    {
        Super::GetAssetRegistryTags(OutTags);
        
        // 指定 Chunk ID
        OutTags.Add(FAssetRegistryTag(
            "ChunkID",
            TEXT("0"),  // 基础游戏内容
            FAssetRegistryTag::TT_Numerical
        ));
    }
};
```

#### 步骤 3: 使用 Chunk 加载

```cpp
// 加载特定 Chunk
void LoadChunk(int32 ChunkID)
{
    TArray<int32> ChunkIDs;
    ChunkIDs.Add(ChunkID);
    
    UAuraAssetManager::Get().LoadChunks(
        ChunkIDs,
        FStreamableDelegate()
    );
}
```

---

## 6. 压缩配置

### 6.1 压缩格式

项目使用 **Oodle Kraken** 压缩：

```ini
PackageCompressionFormat=Oodle
PackageCompressionMethod=Kraken
```

### 6.2 压缩级别

```ini
PackageCompressionLevel_DebugDevelopment=4
PackageCompressionLevel_TestShipping=5
PackageCompressionLevel_Distribution=7
```

**说明**:
- **级别 1-3**: 快速压缩，文件较大
- **级别 4-5**: 平衡压缩，推荐用于开发
- **级别 6-7**: 最大压缩，文件最小，但压缩时间较长

### 6.3 压缩选项

```ini
PackageCompressionMinBytesSaved=1024
PackageCompressionMinPercentSaved=5
```

**说明**: 只有节省超过 1024 字节或 5% 的文件才会被压缩。

---

## 7. 打包流程

### 7.1 打包步骤

#### 步骤 1: 准备资产

1. 确保所有资产已正确配置
2. 检查数据资产引用
3. 验证软引用路径

#### 步骤 2: Cook 资产

```bash
# 使用 UnrealEditor-Cmd
UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=cook 
    -targetplatform=Windows 
    -cookall
```

#### 步骤 3: 打包项目

```bash
# 打包项目
UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=AutomationTool 
    -Script="Engine/Build/BuildAutomation.cs" 
    -ExecuteBuild 
    -Project="ProjectPath.uproject" 
    -Target="ProjectName Windows Development" 
    -TargetPlatform=Win64 
    -Configuration=Development
```

### 7.2 编辑器打包

1. **打开项目**
2. **File → Package Project → ...**
3. **选择平台**（Windows, Android, iOS 等）
4. **选择配置**（Development, Shipping 等）
5. **选择输出目录**
6. **开始打包**

---

## 8. 数据资产的打包考虑

### 8.1 当前实现

**数据资产存储方式**:
- 存储在 GameMode 中（硬引用）
- 启动时自动加载
- 常驻内存

**优点**:
- 简单直接
- 无需额外配置
- 访问快速

**缺点**:
- 无法按需加载
- 无法分块
- 启动时加载所有数据

### 8.2 优化建议

#### 8.2.1 使用软引用

```cpp
// 在 GameMode 中使用软引用
class AAuraGameModeBase : public AGameModeBase
{
    // 改为软引用
    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UCharacterClassInfo> CharacterClassInfo;
    
    // 异步加载
    void LoadCharacterClassInfo()
    {
        if (CharacterClassInfo.IsNull())
        {
            return;
        }
        
        FStreamableManager& StreamableManager = 
            UAuraAssetManager::Get().GetStreamableManager();
        
        StreamableManager.RequestAsyncLoad(
            CharacterClassInfo.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(
                this,
                &AAuraGameModeBase::OnCharacterClassInfoLoaded
            )
        );
    }
    
    void OnCharacterClassInfoLoaded()
    {
        UCharacterClassInfo* LoadedInfo = 
            CharacterClassInfo.LoadSynchronous();
        // 使用加载的数据
    }
};
```

#### 8.2.2 使用 Primary Assets

将数据资产转换为 Primary Assets，实现：
- 按需加载
- 分块管理
- 更好的内存控制

---

## 9. 地图资产的打包

### 9.1 当前实现

**地图存储方式**:
- 使用 `TSoftObjectPtr<UWorld>` 软引用
- 切换地图时异步加载

**优点**:
- 延迟加载
- 减少初始加载时间
- 支持流式加载

### 9.2 地图分块

如果需要将地图分成多个 Chunk：

```cpp
// 在 MapEntrance 中指定 Chunk
class AMapEntrance : public ACheckpoint
{
    UPROPERTY(EditAnywhere)
    int32 MapChunkID = 1;  // DLC 内容使用不同的 Chunk ID
    
    void LoadMapChunk()
    {
        TArray<int32> ChunkIDs;
        ChunkIDs.Add(MapChunkID);
        
        UAuraAssetManager::Get().LoadChunks(
            ChunkIDs,
            FStreamableDelegate::CreateUObject(
                this,
                &AMapEntrance::OnChunkLoaded
            )
        );
    }
};
```

---

## 10. 打包最佳实践

### 10.1 资产组织

#### 10.1.1 目录结构

```
Content/
├── DataAssets/          # 数据资产
│   ├── CharacterClassInfo/
│   ├── AbilityInfo/
│   └── AttributeInfo/
├── Maps/                # 地图
│   ├── MainMap/
│   └── Dungeons/
├── Blueprints/          # 蓝图
│   ├── Abilities/
│   └── Characters/
└── ...
```

#### 10.1.2 命名规范

- **数据资产**: `DA_` 前缀（如 `DA_CharacterClassInfo`）
- **地图**: 描述性名称（如 `MainMap`, `Dungeon1`）
- **蓝图**: `BP_` 前缀（如 `BP_GA_FireBolt`）

### 10.2 性能优化

#### 10.2.1 减少硬引用

- 使用软引用替代硬引用
- 减少启动时加载的资产

#### 10.2.2 资产压缩

- 使用适当的压缩级别
- 平衡文件大小和加载时间

#### 10.2.3 按需加载

- 使用 Primary Assets 或软引用
- 实现按需加载机制

### 10.3 内存管理

#### 10.3.1 卸载未使用的资产

```cpp
// 卸载不再需要的资产
void UnloadUnusedAssets()
{
    UAuraAssetManager::Get().UnloadPrimaryAssets(
        TArray<FPrimaryAssetId>()
    );
}
```

#### 10.3.2 监控内存使用

```cpp
// 获取已加载的资产信息
void LogLoadedAssets()
{
    TArray<FPrimaryAssetId> LoadedAssets;
    UAuraAssetManager::Get().GetPrimaryAssetIdList(
        UAuraAssetManager::CharacterClassInfoType,
        LoadedAssets
    );
    
    UE_LOG(LogAura, Log, TEXT("Loaded %d CharacterClassInfo assets"), 
        LoadedAssets.Num());
}
```

---

## 11. 打包检查清单

### 11.1 打包前检查

- [ ] 所有数据资产已正确配置
- [ ] 软引用路径正确
- [ ] 地图引用正确
- [ ] 没有缺失的资产引用
- [ ] 压缩设置合适
- [ ] Chunk 配置正确（如果使用）

### 11.2 打包后验证

- [ ] 打包成功完成
- [ ] Pak 文件大小合理
- [ ] 游戏可以正常启动
- [ ] 所有资产可以正常加载
- [ ] 地图切换正常
- [ ] 性能表现正常

---

## 12. 常见问题

### Q1: 打包后资产缺失？

**A**: 检查：
1. 资产是否被正确引用
2. 软引用路径是否正确
3. 资产是否在 Cook 列表中

### Q2: 打包文件过大？

**A**: 
1. 检查压缩设置
2. 使用更高的压缩级别
3. 移除未使用的资产
4. 考虑使用 Chunking

### Q3: 加载时间过长？

**A**:
1. 减少启动时加载的资产
2. 使用软引用延迟加载
3. 优化资产大小
4. 使用 IoStore

---

## 13. 总结

Aura 项目的资产打包系统：

1. ✅ **基础打包配置** - Pak 文件、IoStore、压缩
2. ✅ **软引用支持** - 地图使用软引用延迟加载
3. ✅ **数据资产管理** - 通过 GameMode 引用
4. ⚠️ **未使用 Primary Assets** - 可以添加以支持更高级功能
5. ⚠️ **未启用 Chunking** - 可以添加以支持 DLC 和流式下载

### 未来优化方向

1. **添加 Primary Assets 支持**
   - 更好的资产管理
   - 按需加载
   - 分块支持

2. **启用 Chunking**
   - DLC 内容支持
   - 流式下载
   - 平台优化

3. **优化加载策略**
   - 预加载关键资产
   - 后台加载非关键资产
   - 智能卸载机制

---

## 相关文档

- [资产管理详细文档](./Asset_Management_Details.md) - 资产管理详细说明
- [Asset Manager 系统](./Asset_Manager_System.md) - Asset Manager 基础文档
- [数据资产系统](./Data_Assets_System.md) - 数据资产系统文档

