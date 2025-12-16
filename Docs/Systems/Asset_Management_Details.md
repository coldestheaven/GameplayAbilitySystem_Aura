# 资产管理详细文档

## 概述

本文档详细说明 Aura 项目中资产管理的各个方面，包括 Asset Manager、数据资产、软引用、资产加载和访问机制等。

---

## 1. Asset Manager 系统

### 1.1 UAuraAssetManager

自定义的 Asset Manager，继承自 UE5 的 `UAssetManager`。

#### 类定义

```cpp
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
    GENERATED_BODY()
    
public:
    // 获取单例
    static UAuraAssetManager& Get();
    
protected:
    // 初始加载
    virtual void StartInitialLoading() override;
};
```

#### 核心职责

1. **全局初始化入口**
   - 游戏启动时的第一个初始化点
   - 确保关键系统在游戏开始前初始化

2. **Gameplay Tags 初始化**
   - 初始化所有 Native Gameplay Tags
   - 确保 Tags 在游戏逻辑使用前可用

3. **GAS 全局数据初始化**
   - 初始化 Ability System Globals
   - 启用 Target Data 系统（必需）

#### 实现细节

```cpp
UAuraAssetManager& UAuraAssetManager::Get()
{
    check(GEngine);
    
    // 从 GEngine 获取 AssetManager
    UAuraAssetManager* AuraAssetManager = 
        Cast<UAuraAssetManager>(GEngine->AssetManager);
    return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
    // 1. 调用父类初始化
    Super::StartInitialLoading();
    
    // 2. 初始化 Gameplay Tags（必须在最早时机）
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // 3. 初始化 GAS 全局数据（必需，用于 Target Data）
    UAbilitySystemGlobals::Get().InitGlobalData();
}
```

#### 初始化时机

```
游戏启动
    ↓
Engine 初始化
    ↓
AssetManager 创建（单例）
    ↓
StartInitialLoading() 调用
    ↓
初始化 Gameplay Tags
    ↓
初始化 GAS Globals
    ↓
其他系统初始化
```

---

## 2. 数据资产系统

### 2.1 数据资产类型

项目中使用以下数据资产：

#### 2.1.1 UCharacterClassInfo

**用途**: 存储角色职业配置信息

**位置**: `Source/Aura/AbilitySystem/Data/CharacterClassInfo.h`

**结构**:
```cpp
UCLASS()
class UCharacterClassInfo : public UDataAsset
{
    // 职业信息映射
    UPROPERTY(EditDefaultsOnly)
    TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
    
    // 通用属性（所有职业共享）
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;
    
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> SecondaryAttributes;
    
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> SecondaryAttributes_Infinite;
    
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> VitalAttributes;
    
    // 通用能力
    UPROPERTY(EditDefaultsOnly)
    TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;
    
    // 伤害计算系数（曲线表）
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UCurveTable> DamageCalculationCoefficients;
};
```

**存储内容**:
- 每个职业的主属性 GameplayEffect
- 每个职业的初始能力列表
- 每个职业的经验值奖励（按等级）
- 所有职业共享的通用属性和能力
- 伤害计算系数曲线表

#### 2.1.2 UAbilityInfo

**用途**: 存储所有能力的元数据

**位置**: `Source/Aura/AbilitySystem/Data/AbilityInfo.h`

**结构**:
```cpp
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
    // 能力标签
    FGameplayTag AbilityTag;
    
    // 输入标签
    FGameplayTag InputTag;
    
    // 状态标签
    FGameplayTag StatusTag;
    
    // 冷却标签
    FGameplayTag CooldownTag;
    
    // 能力类型
    FGameplayTag AbilityType;
    
    // UI 信息
    TObjectPtr<const UTexture2D> Icon;
    TObjectPtr<const UMaterialInterface> BackgroundMaterial;
    
    // 解锁要求
    int32 LevelRequirement;
    
    // 能力类
    TSubclassOf<UAuraGameplayAbility> Ability;
};

UCLASS()
class UAbilityInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FAuraAbilityInfo> AbilityInformation;
};
```

#### 2.1.3 UAttributeInfo

**用途**: 存储属性的 UI 显示信息

**位置**: `Source/Aura/AbilitySystem/Data/AttributeInfo.h`

**结构**:
```cpp
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
    // 属性标签
    FGameplayTag AttributeTag;
    
    // 显示名称
    FText AttributeName;
    
    // 描述
    FText AttributeDescription;
};

UCLASS()
class UAttributeInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FAuraAttributeInfo> AttributeInformation;
};
```

#### 2.1.4 ULevelUpInfo

**用途**: 存储等级信息

**位置**: `Source/Aura/AbilitySystem/Data/LevelUpInfo.h`

**结构**:
```cpp
USTRUCT(BlueprintType)
struct FLevelUpInfo
{
    // 升级所需经验值
    int32 LevelUpRequirement;
    
    // 属性点奖励
    int32 AttributePointReward;
    
    // 法术点奖励
    int32 SpellPointReward;
};

UCLASS()
class ULevelUpInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FLevelUpInfo> LevelUpInformation;
};
```

#### 2.1.5 ULootTiers

**用途**: 存储战利品等级信息

**位置**: `Source/Aura/AbilitySystem/Data/LootTiers.h`

---

### 2.2 数据资产访问机制

#### 2.2.1 通过 GameMode 访问

**主要方式**: 数据资产存储在 `AAuraGameModeBase` 中

```cpp
// GameMode 中存储数据资产引用
class AAuraGameModeBase : public AGameModeBase
{
    UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
    TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
    
    UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
    TObjectPtr<UAbilityInfo> AbilityInfo;
    
    UPROPERTY(EditDefaultsOnly, Category = "Attribute Info")
    TObjectPtr<UAttributeInfo> AttributeInfo;
};
```

#### 2.2.2 通过静态库函数访问

**工具类**: `UAuraAbilitySystemLibrary`

```cpp
// 获取 CharacterClassInfo
UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(
    const UObject* WorldContextObject
)
{
    // 从 GameMode 获取
    const AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    
    if (AuraGameMode)
    {
        return AuraGameMode->CharacterClassInfo;
    }
    
    return nullptr;
}

// 获取 AbilityInfo
UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(
    const UObject* WorldContextObject
)
{
    const AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    
    if (AuraGameMode)
    {
        return AuraGameMode->AbilityInfo;
    }
    
    return nullptr;
}

// 获取 AttributeInfo
UAttributeInfo* UAuraAbilitySystemLibrary::GetAttributeInfo(
    const UObject* WorldContextObject
)
{
    const AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    
    if (AuraGameMode)
    {
        return AuraGameMode->AttributeInfo;
    }
    
    return nullptr;
}
```

#### 2.2.3 使用示例

```cpp
// 在技能中获取能力信息
void UAuraGameplayAbility::SomeFunction()
{
    UAbilityInfo* AbilityInfo = 
        UAuraAbilitySystemLibrary::GetAbilityInfo(this);
    
    if (AbilityInfo)
    {
        FAuraAbilityInfo Info = 
            AbilityInfo->FindAbilityInfoForTag(AbilityTag);
        
        // 使用信息
        // ...
    }
}

// 初始化角色属性
void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(
    const UObject* WorldContextObject,
    ECharacterClass CharacterClass,
    float Level,
    UAbilitySystemComponent* ASC
)
{
    // 获取 CharacterClassInfo
    UCharacterClassInfo* CharacterClassInfo = 
        GetCharacterClassInfo(WorldContextObject);
    
    // 获取职业默认信息
    FCharacterClassDefaultInfo ClassDefaultInfo = 
        CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
    
    // 应用主属性
    FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = 
        ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, ...);
    ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());
    
    // 应用次属性
    FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = 
        ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, ...);
    ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());
    
    // 应用生命值属性
    FGameplayEffectSpecHandle VitalAttributesSpecHandle = 
        ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, ...);
    ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}
```

---

## 3. 软引用系统

### 3.1 软引用类型

项目中使用 `TSoftObjectPtr` 来引用地图资产，避免硬引用导致的加载问题。

#### 3.1.1 地图软引用

**位置**: `AAuraGameModeBase` 和 `AMapEntrance`

```cpp
// GameMode 中存储地图软引用
class AAuraGameModeBase : public AGameModeBase
{
    // 默认地图
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Settings")
    TSoftObjectPtr<UWorld> DefaultMap;
    
    // 地图映射（地图名称 -> 地图软引用）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Settings")
    TMap<FString, TSoftObjectPtr<UWorld>> Maps;
};

// MapEntrance 中存储目标地图软引用
class AMapEntrance : public ACheckpoint
{
    // 目标地图
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UWorld> DestinationMap;
    
    // 目标 PlayerStart Tag
    UPROPERTY(EditAnywhere)
    FName DestinationPlayerStartTag;
};
```

### 3.2 软引用使用

#### 3.2.1 获取资产名称

```cpp
// 从软引用获取资产名称
FString MapAssetName = DestinationMap.ToSoftObjectPath().GetAssetName();

// 在存档中使用
LoadScreenSaveGame->MapAssetName = 
    AuraGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();
```

#### 3.2.2 加载地图

```cpp
// 在 MapEntrance 中切换地图
void AMapEntrance::OnSphereOverlap(...)
{
    if (APawn* OverlappingPawn = Cast<APawn>(OtherActor))
    {
        if (OverlappingPawn->IsPlayerControlled())
        {
            // 保存当前地图状态
            AAuraGameModeBase* AuraGM = 
                Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
            
            if (AuraGM)
            {
                // 保存世界状态
                AuraGM->SaveWorldState(
                    GetWorld(), 
                    DestinationMap.ToSoftObjectPath().GetAssetName()
                );
            }
            
            // 切换地图
            UGameplayStatics::OpenLevelBySoftObjectPtr(
                this, 
                DestinationMap, 
                true,  // bAbsolute
                DestinationPlayerStartTag.ToString()
            );
        }
    }
}
```

#### 3.2.3 查找地图

```cpp
// 根据资产名称查找地图
TSoftObjectPtr<UWorld> AAuraGameModeBase::GetMapByName(
    const FString& MapAssetName
) const
{
    for (const auto& Map : Maps)
    {
        if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
        {
            return Map.Value;
        }
    }
    
    return TSoftObjectPtr<UWorld>();
}
```

---

## 4. 资产加载策略

### 4.1 加载时机

#### 4.1.1 启动时加载

**Asset Manager 初始化**:
- Gameplay Tags（必需，游戏逻辑依赖）
- GAS Globals（必需，Target Data 依赖）

#### 4.1.2 运行时加载

**数据资产**:
- 按需加载（通过 GameMode 引用）
- 首次访问时加载
- 常驻内存（GameMode 生命周期内）

**地图资产**:
- 使用软引用延迟加载
- 切换地图时加载
- 使用 `OpenLevelBySoftObjectPtr` 异步加载

### 4.2 加载方式

#### 4.2.1 同步加载

**数据资产**: 通过 GameMode 引用直接访问

```cpp
// 直接访问，已加载
UCharacterClassInfo* CharacterClassInfo = 
    AuraGameMode->CharacterClassInfo;
```

#### 4.2.2 异步加载

**地图资产**: 使用软引用异步加载

```cpp
// 异步加载地图
UGameplayStatics::OpenLevelBySoftObjectPtr(
    this,
    DestinationMap,  // 软引用
    true,
    PlayerStartTag.ToString()
);
```

---

## 5. 资产配置

### 5.1 项目设置配置

#### 5.1.1 Asset Manager 配置

1. **打开项目设置**
   - Edit → Project Settings

2. **设置 Asset Manager**
   - Game → Asset Manager
   - **Asset Manager Class**: 选择 `AuraAssetManager`

3. **验证配置**
   - 确保 Asset Manager 类正确设置
   - 检查 Default Classes 配置

### 5.2 GameMode 配置

#### 5.2.1 数据资产配置

在 GameMode 蓝图中设置数据资产引用：

```
AAuraGameModeBase (Blueprint)
├── CharacterClassInfo: DA_CharacterClassInfo
├── AbilityInfo: DA_AbilityInfo
├── AttributeInfo: DA_AttributeInfo
└── DefaultMap: /Game/Maps/MainMap
```

#### 5.2.2 地图配置

```
Maps:
  "MainMap": /Game/Maps/MainMap
  "Dungeon1": /Game/Maps/Dungeon1
  "Dungeon2": /Game/Maps/Dungeon2
```

### 5.3 数据资产创建

#### 5.3.1 创建步骤

1. **创建数据资产**
   - 右键 Content Browser
   - Miscellaneous → Data Asset
   - 选择对应类（如 `CharacterClassInfo`）

2. **配置数据**
   - 打开数据资产
   - 填写各个字段
   - 设置数组和映射

3. **在 GameMode 中引用**
   - 打开 GameMode 蓝图
   - 设置数据资产引用

#### 5.3.2 配置示例

**CharacterClassInfo 配置**:
```
CharacterClassInformation:
  Elementalist:
    PrimaryAttributes: GE_PrimaryAttributes_Elementalist
    StartupAbilities:
      - BP_GA_FireBolt
      - BP_GA_Electrocute
    XPReward:
      Level 1: 10
      Level 2: 15
      Level 3: 20
      ...
  
  Warrior:
    PrimaryAttributes: GE_PrimaryAttributes_Warrior
    StartupAbilities:
      - BP_GA_MeleeAttack
    XPReward:
      Level 1: 10
      ...

Common Class Defaults:
  PrimaryAttributes_SetByCaller: GE_PrimaryAttributes_SetByCaller
  SecondaryAttributes: GE_SecondaryAttributes
  SecondaryAttributes_Infinite: GE_SecondaryAttributes_Infinite
  VitalAttributes: GE_VitalAttributes
  CommonAbilities:
    - BP_GA_CommonAbility1
    - BP_GA_CommonAbility2
  
  DamageCalculationCoefficients: CT_Damage
```

---

## 6. 资产访问模式

### 6.1 访问模式总结

| 资产类型 | 存储位置 | 访问方式 | 加载时机 |
|---------|---------|---------|---------|
| Gameplay Tags | AssetManager | `FAuraGameplayTags::Get()` | 启动时 |
| CharacterClassInfo | GameMode | `GetCharacterClassInfo()` | 运行时 |
| AbilityInfo | GameMode | `GetAbilityInfo()` | 运行时 |
| AttributeInfo | GameMode | `GetAttributeInfo()` | 运行时 |
| LevelUpInfo | PlayerState | `PlayerState->LevelUpInfo` | 运行时 |
| 地图 | GameMode/MapEntrance | `TSoftObjectPtr<UWorld>` | 切换时 |

### 6.2 访问最佳实践

#### 6.2.1 缓存访问

```cpp
// ✅ 缓存数据资产引用
class UAuraGameplayAbility
{
private:
    mutable UCharacterClassInfo* CachedCharacterClassInfo = nullptr;
    
public:
    UCharacterClassInfo* GetCharacterClassInfo() const
    {
        if (!CachedCharacterClassInfo)
        {
            CachedCharacterClassInfo = 
                UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
        }
        return CachedCharacterClassInfo;
    }
};
```

#### 6.2.2 空指针检查

```cpp
// ✅ 始终检查空指针
UCharacterClassInfo* CharacterClassInfo = 
    UAuraAbilitySystemLibrary::GetCharacterClassInfo(WorldContextObject);

if (CharacterClassInfo == nullptr)
{
    UE_LOG(LogAura, Error, TEXT("CharacterClassInfo is null!"));
    return;
}

// 使用 CharacterClassInfo
```

#### 6.2.3 使用 WorldContextObject

```cpp
// ✅ 使用 WorldContextObject 获取 World
UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject)
{
    // 从 WorldContextObject 获取 World
    UWorld* World = GEngine->GetWorldFromContextObject(
        WorldContextObject, 
        EGetWorldErrorMode::LogAndReturnNull
    );
    
    if (!World)
    {
        return nullptr;
    }
    
    // 获取 GameMode
    AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(World->GetAuthGameMode());
    
    return AuraGameMode ? AuraGameMode->CharacterClassInfo : nullptr;
}
```

---

## 7. 性能考虑

### 7.1 内存管理

#### 7.1.1 数据资产常驻

- 数据资产存储在 GameMode 中
- GameMode 生命周期内常驻内存
- 适合频繁访问的数据

#### 7.1.2 软引用延迟加载

- 地图使用软引用，避免启动时加载
- 只在需要时加载
- 减少初始加载时间

### 7.2 访问优化

#### 7.2.1 避免频繁查找

```cpp
// ❌ 每次查找
for (int32 i = 0; i < 100; ++i)
{
    UCharacterClassInfo* Info = GetCharacterClassInfo(this);
    // ...
}

// ✅ 缓存引用
UCharacterClassInfo* Info = GetCharacterClassInfo(this);
for (int32 i = 0; i < 100; ++i)
{
    // 使用缓存的 Info
}
```

#### 7.2.2 批量操作

```cpp
// ✅ 批量获取数据
void InitializeAllAbilities()
{
    UAbilityInfo* AbilityInfo = GetAbilityInfo(this);
    
    // 一次性获取所有能力信息
    for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
    {
        // 处理每个能力
    }
}
```

---

## 8. 扩展指南

### 8.1 添加新数据资产

#### 步骤 1: 创建数据资产类

```cpp
UCLASS()
class UMyDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly)
    // 添加数据字段
};
```

#### 步骤 2: 在 GameMode 中添加引用

```cpp
class AAuraGameModeBase : public AGameModeBase
{
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UMyDataAsset> MyDataAsset;
};
```

#### 步骤 3: 添加访问函数

```cpp
UMyDataAsset* UAuraAbilitySystemLibrary::GetMyDataAsset(
    const UObject* WorldContextObject
)
{
    const AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    
    return AuraGameMode ? AuraGameMode->MyDataAsset : nullptr;
}
```

### 8.2 添加 Primary Asset 支持

如果需要使用 UE5 的 Primary Asset 系统：

```cpp
class UAuraAssetManager : public UAssetManager
{
public:
    // 定义 Primary Asset 类型
    static const FPrimaryAssetType CharacterClassInfoType;
    static const FPrimaryAssetType AbilityInfoType;
    
    // 加载 Primary Asset
    void LoadPrimaryAssets();
};
```

---

## 9. 总结

Aura 项目的资产管理系统提供了：

1. ✅ **统一的初始化入口** - AssetManager 管理全局初始化
2. ✅ **数据驱动配置** - 数据资产提供灵活的配置方式
3. ✅ **软引用支持** - 地图使用软引用延迟加载
4. ✅ **便捷的访问接口** - 静态库函数提供统一访问
5. ✅ **类型安全** - C++ 类型确保数据正确性
6. ✅ **编辑器集成** - 完整的编辑器支持

通过这个系统，可以高效地管理和访问游戏资产，同时保持代码的清晰和可维护性。

---

## 相关文档

- [Asset Manager 系统](./Asset_Manager_System.md) - Asset Manager 基础文档
- [数据资产系统](./Data_Assets_System.md) - 数据资产详细文档
- [Gameplay 框架](../Gameplay/Gameplay_Framework.md) - Gameplay 框架文档

