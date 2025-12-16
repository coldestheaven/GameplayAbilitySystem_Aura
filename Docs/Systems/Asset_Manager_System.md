# Asset Manager 系统文档

## 概述

Asset Manager 是 Unreal Engine 的资源管理系统，Aura 项目使用自定义的 `UAuraAssetManager` 来初始化游戏全局数据，包括 Gameplay Tags 和 GAS 全局数据。

## 核心组件

### UAuraAssetManager

自定义资源管理器，继承自 `UAssetManager`。

#### 类层次结构

```
UAssetManager (UE5 Base)
    ↓
UAuraAssetManager
```

#### 核心功能

1. **Gameplay Tags 初始化**
   - 在游戏启动时初始化所有 Native Gameplay Tags
   - 确保 Tags 在游戏开始前就可用

2. **GAS 全局数据初始化**
   - 初始化 Ability System Globals
   - 启用 Target Data 系统

#### 关键方法

**获取单例**:
```cpp
static UAuraAssetManager& Get();
```

**初始化加载**:
```cpp
virtual void StartInitialLoading() override;
```

#### 实现

```cpp
UAuraAssetManager& UAuraAssetManager::Get()
{
    check(GEngine);
    
    UAuraAssetManager* AuraAssetManager = 
        Cast<UAuraAssetManager>(GEngine->AssetManager);
    return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // 初始化 Gameplay Tags
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // 初始化 GAS 全局数据（必需，用于 Target Data）
    UAbilitySystemGlobals::Get().InitGlobalData();
}
```

---

## 初始化流程

### 1. 游戏启动

```
游戏启动
    ↓
Engine 初始化
    ↓
AssetManager 创建
    ↓
StartInitialLoading 调用
    ↓
初始化 Gameplay Tags
    ↓
初始化 GAS Globals
```

### 2. Gameplay Tags 初始化

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // 初始化所有 Tags
    // Attributes
    GameplayTags.Attributes_Primary_Strength = 
        UGameplayTagsManager::Get().AddNativeGameplayTag(...);
    
    // Abilities
    GameplayTags.Abilities_Attack = 
        UGameplayTagsManager::Get().AddNativeGameplayTag(...);
    
    // ... 更多 Tags ...
}
```

### 3. GAS Globals 初始化

```cpp
UAbilitySystemGlobals::Get().InitGlobalData();
```

这个调用是必需的，用于：
- 初始化 Target Data 系统
- 设置 GAS 全局配置
- 启用网络同步功能

---

## 配置

### 在项目设置中配置

1. **打开项目设置**
   - Edit → Project Settings

2. **设置 Asset Manager**
   - Game → Asset Manager
   - 选择 `AuraAssetManager` 类

3. **验证配置**
   - 确保 Asset Manager 类正确设置
   - 检查 Default Classes 配置

---

## 使用场景

### 访问 Asset Manager

```cpp
// 获取 Asset Manager 单例
UAuraAssetManager& AssetManager = UAuraAssetManager::Get();

// 使用 Asset Manager 的功能
// ...
```

### 初始化检查

在需要确保初始化完成的代码中：

```cpp
void SomeFunction()
{
    // 确保 Asset Manager 已初始化
    UAuraAssetManager& AssetManager = UAuraAssetManager::Get();
    
    // 现在可以安全使用 Gameplay Tags
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    // ...
}
```

---

## 相关文档

- [核心文档](../Core/Architecture.md) - 系统架构
- [Gameplay 框架](../Gameplay/Gameplay_Framework.md) - 游戏框架

---

## 总结

Asset Manager 系统提供了：

1. ✅ **全局初始化** - 游戏启动时的初始化点
2. ✅ **Gameplay Tags** - 确保 Tags 在游戏开始前可用
3. ✅ **GAS 支持** - 初始化 GAS 全局数据
4. ✅ **单例访问** - 全局访问 Asset Manager
5. ✅ **扩展性** - 可以添加更多初始化逻辑

通过这个系统，可以确保游戏在启动时正确初始化所有必要的全局数据。

