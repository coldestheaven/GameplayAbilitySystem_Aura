# Gameplay 框架文档

## 目录

1. [系统概述](#系统概述)
2. [核心组件](#核心组件)
3. [游戏模式 (Game Mode)](#游戏模式-game-mode)
4. [游戏实例 (Game Instance)](#游戏实例-game-instance)
5. [存档系统 (Save System)](#存档系统-save-system)
6. [资源管理器 (Asset Manager)](#资源管理器-asset-manager)
7. [存档接口 (Save Interface)](#存档接口-save-interface)
8. [世界状态管理](#世界状态管理)
9. [地图切换系统](#地图切换系统)
10. [玩家进度管理](#玩家进度管理)

---

## 系统概述

Gameplay 框架是 Aura 项目的核心游戏逻辑系统，负责管理游戏状态、存档系统、地图切换、世界状态保存等核心功能。

### 核心特性

- ✅ **存档系统**: 完整的存档和加载功能
- ✅ **世界状态管理**: 保存和恢复世界中的 Actor 状态
- ✅ **地图切换**: 支持多地图切换和玩家起始点管理
- ✅ **进度管理**: 保存和加载玩家进度、能力、属性
- ✅ **资源管理**: 统一管理游戏资源

---

## 核心组件

### 组件层次结构

```
UGameInstance (UE5 Base)
    ↓
UAuraGameInstance (游戏实例)
    - 存储会话数据
    - 管理存档槽信息

AGameModeBase (UE5 Base)
    ↓
AAuraGameModeBase (游戏模式)
    - 存档管理
    - 世界状态保存/加载
    - 地图切换

USaveGame (UE5 Base)
    ↓
ULoadScreenSaveGame (存档数据)
    - 玩家数据
    - 能力数据
    - 地图状态

UAssetManager (UE5 Base)
    ↓
UAuraAssetManager (资源管理器)
    - 初始化 Gameplay Tags
    - 初始化 GAS 全局数据
```

---

## 游戏模式 (Game Mode)

### AAuraGameModeBase

游戏模式是游戏逻辑的核心，负责管理存档、世界状态、地图切换等功能。

#### 核心功能

1. **存档管理**
   - 保存和加载存档槽数据
   - 删除存档槽
   - 检索游戏内存档数据

2. **世界状态管理**
   - 保存世界中所有可存档 Actor 的状态
   - 加载世界状态

3. **地图切换**
   - 切换到指定地图
   - 管理玩家起始点

4. **玩家死亡处理**
   - 处理玩家死亡后的重生

#### 数据资产

```cpp
// 职业信息数据资产
UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

// 能力信息数据资产
UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
TObjectPtr<UAbilityInfo> AbilityInfo;

// 战利品等级数据资产
UPROPERTY(EditDefaultsOnly, Category = "Loot Tiers")
TObjectPtr<ULootTiers> LootTiers;
```

#### 地图配置

```cpp
// 默认地图
UPROPERTY(EditDefaultsOnly)
FString DefaultMapName;

UPROPERTY(EditDefaultsOnly)
TSoftObjectPtr<UWorld> DefaultMap;

UPROPERTY(EditDefaultsOnly)
FName DefaultPlayerStartTag;

// 地图映射表
UPROPERTY(EditDefaultsOnly)
TMap<FString, TSoftObjectPtr<UWorld>> Maps;
```

### 存档管理 API

#### 保存存档槽数据

```cpp
void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
```

**功能**: 保存存档槽数据到文件

**流程**:
1. 检查存档是否存在，如果存在则删除
2. 创建新的存档对象
3. 从 LoadSlot 复制数据
4. 保存到文件

#### 获取存档槽数据

```cpp
ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
```

**功能**: 获取指定存档槽的数据

**返回**: 
- 如果存档存在，返回加载的存档对象
- 如果不存在，返回新创建的存档对象

#### 删除存档槽

```cpp
static void DeleteSlot(const FString& SlotName, int32 SlotIndex);
```

**功能**: 删除指定的存档槽

#### 检索游戏内存档数据

```cpp
ULoadScreenSaveGame* RetrieveInGameSaveData();
```

**功能**: 从 GameInstance 获取当前游戏的存档数据

### 世界状态管理 API

#### 保存世界状态

```cpp
void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;
```

**功能**: 保存世界中所有实现 `ISaveInterface` 的 Actor 状态

**流程**:
1. 获取世界名称
2. 从存档中获取或创建地图数据
3. 遍历所有 Actor
4. 对实现 `ISaveInterface` 的 Actor：
   - 保存 Actor 名称和变换
   - 序列化 Actor 的 `SaveGame` 标记的变量
   - 添加到保存列表
5. 更新存档并保存

**序列化机制**:
- 使用 `FObjectAndNameAsStringProxyArchive` 进行序列化
- 只序列化标记了 `SaveGame` 的变量
- 数据存储为二进制字节数组

#### 加载世界状态

```cpp
void LoadWorldState(UWorld* World) const;
```

**功能**: 从存档加载世界状态

**流程**:
1. 获取世界名称
2. 从存档加载数据
3. 遍历所有 Actor
4. 对实现 `ISaveInterface` 的 Actor：
   - 查找对应的保存数据
   - 如果 `ShouldLoadTransform()` 返回 true，恢复变换
   - 反序列化变量
   - 调用 `LoadActor()` 进行后处理

### 地图切换 API

#### 切换到地图

```cpp
void TravelToMap(UMVVM_LoadSlot* Slot);
```

**功能**: 切换到指定地图

**流程**:
1. 从 Slot 获取地图名称和存档信息
2. 从 Maps 映射表获取地图引用
3. 使用 `OpenLevelBySoftObjectPtr` 打开地图

#### 获取地图名称

```cpp
FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;
```

**功能**: 从地图资源名称获取显示名称

### 玩家起始点选择

```cpp
virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
```

**功能**: 选择玩家起始点

**流程**:
1. 从 GameInstance 获取 `PlayerStartTag`
2. 查找所有 PlayerStart
3. 返回匹配 Tag 的 PlayerStart
4. 如果没有匹配，返回第一个 PlayerStart

### 玩家死亡处理

```cpp
void PlayerDied(ACharacter* DeadCharacter);
```

**功能**: 处理玩家死亡

**流程**:
1. 获取当前存档数据
2. 获取存档中的地图资源名称
3. 重新加载地图（重生）

---

## 游戏实例 (Game Instance)

### UAuraGameInstance

游戏实例存储会话级别的数据，在游戏运行期间持续存在。

#### 核心数据

```cpp
// 玩家起始点标签
UPROPERTY()
FName PlayerStartTag = FName();

// 存档槽名称
UPROPERTY()
FString LoadSlotName = FString();

// 存档槽索引
UPROPERTY()
int32 LoadSlotIndex = 0;
```

#### 使用场景

- **地图切换**: 存储目标地图的 PlayerStartTag
- **存档管理**: 存储当前使用的存档槽信息
- **会话数据**: 存储需要在游戏运行期间保持的数据

---

## 存档系统 (Save System)

### ULoadScreenSaveGame

存档系统使用 `ULoadScreenSaveGame` 类存储所有游戏数据。

#### 存档槽状态

```cpp
enum ESaveSlotStatus
{
    Vacant,      // 空槽位
    EnterName,   // 需要输入名称
    Taken        // 已占用
};
```

#### 存档数据结构

##### 基础信息

```cpp
FString SlotName;                    // 存档槽名称
int32 SlotIndex;                     // 存档槽索引
FString PlayerName;                  // 玩家名称
FString MapName;                     // 地图名称
FString MapAssetName;                // 地图资源名称
FName PlayerStartTag;                // 玩家起始点标签
ESaveSlotStatus SaveSlotStatus;      // 存档槽状态
bool bFirstTimeLoadIn;               // 是否首次加载
```

##### 玩家数据

```cpp
int32 PlayerLevel;                   // 玩家等级
int32 XP;                            // 经验值
int32 SpellPoints;                   // 法术点
int32 AttributePoints;               // 属性点
float Strength;                      // 力量
float Intelligence;                  // 智力
float Resilience;                    // 韧性
float Vigor;                         // 活力
```

##### 能力数据

```cpp
TArray<FSavedAbility> SavedAbilities; // 保存的能力列表
```

**FSavedAbility 结构**:
```cpp
struct FSavedAbility
{
    TSubclassOf<UGameplayAbility> GameplayAbility;  // 能力类
    FGameplayTag AbilityTag;                        // 能力标签
    FGameplayTag AbilityStatus;                     // 能力状态
    FGameplayTag AbilitySlot;                       // 能力槽位
    FGameplayTag AbilityType;                       // 能力类型
    int32 AbilityLevel;                             // 能力等级
};
```

##### 地图状态数据

```cpp
TArray<FSavedMap> SavedMaps;         // 保存的地图列表
```

**FSavedMap 结构**:
```cpp
struct FSavedMap
{
    FString MapAssetName;            // 地图资源名称
    TArray<FSavedActor> SavedActors; // 保存的 Actor 列表
};
```

**FSavedActor 结构**:
```cpp
struct FSavedActor
{
    FName ActorName;                 // Actor 名称
    FTransform Transform;            // 变换
    TArray<uint8> Bytes;             // 序列化的变量数据
};
```

### 存档操作

#### 保存游戏进度

```cpp
void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);
```

**功能**: 保存游戏内进度数据

**流程**:
1. 从 GameInstance 获取存档槽信息
2. 更新 GameInstance 的 PlayerStartTag
3. 保存存档对象到文件

#### 加载游戏进度

在 `AAuraCharacter::LoadProgress()` 中实现：

**流程**:
1. 获取存档数据
2. 如果是首次加载：
   - 初始化默认属性
   - 添加角色能力
3. 如果不是首次加载：
   - 从存档加载能力
   - 恢复玩家状态（等级、经验、点数）
   - 恢复属性值

### 存档接口方法

#### 获取保存的地图

```cpp
FSavedMap GetSavedMapWithMapName(const FString& InMapName);
```

**功能**: 根据地图名称获取保存的地图数据

#### 检查地图是否存在

```cpp
bool HasMap(const FString& InMapName);
```

**功能**: 检查存档中是否包含指定地图

---

## 资源管理器 (Asset Manager)

### UAuraAssetManager

资源管理器负责初始化游戏资源和全局数据。

#### 核心功能

```cpp
static UAuraAssetManager& Get();
```

**功能**: 获取资源管理器单例

#### 初始化流程

```cpp
virtual void StartInitialLoading() override;
```

**功能**: 启动初始加载

**流程**:
1. 调用父类初始化
2. 初始化原生 Gameplay Tags
3. 初始化 GAS 全局数据（用于 Target Data）

---

## 存档接口 (Save Interface)

### ISaveInterface

存档接口定义了 Actor 需要实现的存档相关方法。

#### 接口方法

##### 是否加载变换

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
bool ShouldLoadTransform();
```

**功能**: 决定是否从存档加载 Actor 的变换

**返回**: 
- `true`: 加载变换
- `false`: 不加载变换（使用默认位置）

##### 加载 Actor

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
void LoadActor();
```

**功能**: Actor 加载完成后的后处理

**用途**:
- 恢复 Actor 的特殊状态
- 重新初始化组件
- 更新视觉效果

### 实现要求

要实现存档功能，Actor 需要：

1. **实现接口**: 实现 `ISaveInterface`
2. **标记变量**: 需要保存的变量使用 `UPROPERTY(SaveGame)` 标记
3. **实现方法**: 实现 `ShouldLoadTransform()` 和 `LoadActor()`

### 序列化机制

#### 保存流程

```cpp
// 1. 创建内存写入器
FMemoryWriter MemoryWriter(SavedActor.Bytes);

// 2. 创建代理归档
FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
Archive.ArIsSaveGame = true;

// 3. 序列化 Actor
Actor->Serialize(Archive);
```

#### 加载流程

```cpp
// 1. 创建内存读取器
FMemoryReader MemoryReader(SavedActor.Bytes);

// 2. 创建代理归档
FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
Archive.ArIsSaveGame = true;

// 3. 反序列化 Actor
Actor->Serialize(Archive);
```

---

## 世界状态管理

### 保存世界状态

世界状态保存系统会保存所有实现 `ISaveInterface` 的 Actor 的状态。

#### 保存时机

- 地图切换前
- 玩家保存游戏时
- 检查点触发时

#### 保存内容

- **Actor 名称**: 用于匹配
- **变换**: 位置、旋转、缩放
- **标记变量**: 所有 `SaveGame` 标记的变量

#### 保存流程

```
1. 获取世界名称
   ↓
2. 从存档获取或创建地图数据
   ↓
3. 遍历所有 Actor
   ↓
4. 检查是否实现 ISaveInterface
   ↓
5. 序列化 Actor 数据
   ↓
6. 添加到保存列表
   ↓
7. 更新存档并保存
```

### 加载世界状态

世界状态加载系统会在世界加载时恢复所有保存的 Actor 状态。

#### 加载时机

- 地图加载完成后
- 玩家加载游戏时

#### 加载流程

```
1. 获取世界名称
   ↓
2. 从存档加载数据
   ↓
3. 遍历所有 Actor
   ↓
4. 检查是否实现 ISaveInterface
   ↓
5. 查找对应的保存数据
   ↓
6. 恢复变换（如果需要）
   ↓
7. 反序列化变量
   ↓
8. 调用 LoadActor() 后处理
```

---

## 地图切换系统

### 地图配置

地图通过 `Maps` 映射表配置：

```cpp
TMap<FString, TSoftObjectPtr<UWorld>> Maps;
```

- **Key**: 地图显示名称
- **Value**: 地图资源的软引用

### 切换流程

```
1. 从 LoadSlot 获取地图名称
   ↓
2. 从 Maps 查找地图引用
   ↓
3. 保存当前世界状态
   ↓
4. 使用 OpenLevelBySoftObjectPtr 打开地图
   ↓
5. 新地图加载后恢复世界状态
```

### 玩家起始点管理

- **PlayerStartTag**: 存储在 GameInstance 中
- **选择逻辑**: 在 `ChoosePlayerStart_Implementation` 中实现
- **匹配机制**: 通过 Tag 匹配对应的 PlayerStart

---

## 玩家进度管理

### 保存进度

#### 保存时机

- 检查点触发时
- 玩家手动保存时
- 地图切换前

#### 保存内容

- **玩家状态**: 等级、经验、属性点、法术点
- **属性值**: 主属性值
- **能力数据**: 所有能力的状态和等级
- **世界状态**: 所有可存档 Actor 的状态
- **检查点信息**: PlayerStartTag

### 加载进度

#### 首次加载

- 初始化默认属性
- 添加初始能力
- 设置 `bFirstTimeLoadIn = false`

#### 非首次加载

- 从存档恢复能力
- 恢复玩家状态
- 恢复属性值
- 加载世界状态

### 进度数据结构

```cpp
// 玩家基础数据
PlayerLevel, XP, SpellPoints, AttributePoints

// 主属性
Strength, Intelligence, Resilience, Vigor

// 能力数据
SavedAbilities[] {
    GameplayAbility,
    AbilityTag,
    AbilityStatus,
    AbilitySlot,
    AbilityType,
    AbilityLevel
}

// 地图状态
SavedMaps[] {
    MapAssetName,
    SavedActors[] {
        ActorName,
        Transform,
        Bytes (序列化数据)
    }
}
```

---

## 使用示例

### 实现可存档的 Actor

```cpp
// MyActor.h
UCLASS()
class AURA_API AMyActor : public AActor, public ISaveInterface
{
    GENERATED_BODY()
    
public:
    // 需要保存的变量
    UPROPERTY(SaveGame)
    int32 MySaveData = 0;
    
    // 实现接口方法
    virtual bool ShouldLoadTransform_Implementation() override { return true; }
    virtual void LoadActor_Implementation() override;
};
```

### 保存游戏进度

```cpp
// 在角色中
void AAuraCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
    AAuraGameModeBase* GameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        ULoadScreenSaveGame* SaveData = GameMode->RetrieveInGameSaveData();
        
        // 保存玩家数据
        SaveData->PlayerLevel = PlayerState->GetPlayerLevel();
        SaveData->XP = PlayerState->GetXP();
        
        // 保存能力
        // ...
        
        // 保存世界状态
        GameMode->SaveWorldState(GetWorld());
        
        // 保存存档
        GameMode->SaveInGameProgressData(SaveData);
    }
}
```

### 加载游戏进度

```cpp
// 在角色中
void AAuraCharacter::LoadProgress()
{
    AAuraGameModeBase* GameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        ULoadScreenSaveGame* SaveData = GameMode->RetrieveInGameSaveData();
        
        if (SaveData->bFirstTimeLoadIn)
        {
            // 首次加载：初始化
            InitializeDefaultAttributes();
            AddCharacterAbilities();
        }
        else
        {
            // 非首次加载：恢复数据
            // 恢复能力
            AbilitySystemComponent->AddCharacterAbilitiesFromSaveData(SaveData);
            
            // 恢复状态
            PlayerState->SetLevel(SaveData->PlayerLevel);
            PlayerState->SetXP(SaveData->XP);
            
            // 恢复属性
            // ...
        }
        
        // 加载世界状态
        GameMode->LoadWorldState(GetWorld());
    }
}
```

---

## 配置指南

### 配置游戏模式

1. **创建游戏模式蓝图**
   - 继承自 `AAuraGameModeBase`
   - 设置数据资产（CharacterClassInfo, AbilityInfo, LootTiers）

2. **配置地图**
   - 设置 `DefaultMapName` 和 `DefaultMap`
   - 设置 `DefaultPlayerStartTag`
   - 在 `Maps` 映射表中添加所有地图

3. **配置存档类**
   - 设置 `LoadScreenSaveGameClass`

### 配置存档槽

存档槽通过 `MVVM_LoadSlot` 管理：

- **SlotName**: 存档槽名称（例如 "SaveSlot1"）
- **SlotIndex**: 存档槽索引（0, 1, 2...）
- **PlayerName**: 玩家名称
- **MapName**: 地图名称

---

## 最佳实践

### 1. 存档设计

- **最小化数据**: 只保存必要的数据
- **版本控制**: 考虑存档版本兼容性
- **错误处理**: 处理存档加载失败的情况

### 2. 世界状态管理

- **选择性保存**: 只保存需要持久化的 Actor
- **性能考虑**: 大量 Actor 的序列化可能影响性能
- **内存管理**: 及时清理不需要的存档数据

### 3. 地图切换

- **状态保存**: 切换前保存世界状态
- **加载优化**: 使用异步加载优化体验
- **错误恢复**: 处理地图加载失败的情况

---

## 总结

Gameplay 框架提供了完整的游戏逻辑管理功能：

- ✅ **完整的存档系统**: 支持多存档槽、世界状态保存
- ✅ **灵活的地图切换**: 支持多地图和玩家起始点管理
- ✅ **可扩展的存档接口**: 易于实现可存档的 Actor
- ✅ **资源管理**: 统一的资源初始化和管理

通过这个框架，开发者可以轻松实现复杂的游戏逻辑和存档功能。

