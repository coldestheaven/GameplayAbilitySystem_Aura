# 输入系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [Enhanced Input 集成](#enhanced-input-集成)
3. [输入配置](#输入配置)
4. [输入组件](#输入组件)
5. [输入绑定流程](#输入绑定流程)
6. [输入标签系统](#输入标签系统)
7. [技能输入处理](#技能输入处理)
8. [配置指南](#配置指南)
9. [使用示例](#使用示例)

---

## 系统概述

Aura 项目使用 Unreal Engine 5.7 的 Enhanced Input 系统，提供了灵活的输入绑定和技能激活机制。

### 核心组件

- **UAuraInputComponent**: 自定义输入组件，扩展 EnhancedInputComponent
- **UAuraInputConfig**: 输入配置数据资产
- **Enhanced Input Actions**: UE5 的输入动作

### 系统特点

- ✅ Enhanced Input 集成
- ✅ GameplayTag 绑定
- ✅ 技能输入自动绑定
- ✅ 支持按下、按住、释放事件
- ✅ 数据驱动配置

---

## Enhanced Input 集成

### Enhanced Input 系统

Enhanced Input 是 UE5 的新输入系统，提供：

- **Input Actions**: 输入动作（按键、鼠标等）
- **Input Mapping Context**: 输入映射上下文
- **Modifiers**: 输入修饰符
- **Triggers**: 输入触发器

### 输入组件

```cpp
class UAuraInputComponent : public UEnhancedInputComponent
```

继承自 `UEnhancedInputComponent`，提供 Enhanced Input 的所有功能。

---

## 输入配置

### UAuraInputConfig

输入配置数据资产，存储输入动作和 GameplayTag 的映射。

#### 数据结构

```cpp
struct FAuraInputAction
{
    const UInputAction* InputAction;  // Enhanced Input Action
    FGameplayTag InputTag;           // 对应的 GameplayTag
};

class UAuraInputConfig : public UDataAsset
{
    TArray<FAuraInputAction> AbilityInputActions;  // 输入动作数组
};
```

#### 查找输入动作

```cpp
const UInputAction* FindAbilityInputActionForTag(
    const FGameplayTag& InputTag, 
    bool bLogNotFound = false
) const;
```

**功能**: 根据 InputTag 查找对应的 InputAction

---

## 输入组件

### UAuraInputComponent

自定义输入组件，提供技能输入绑定功能。

#### 核心方法

```cpp
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void BindAbilityActions(
    const UAuraInputConfig* InputConfig,
    UserClass* Object,
    PressedFuncType PressedFunc,
    ReleasedFuncType ReleasedFunc,
    HeldFuncType HeldFunc
);
```

**功能**: 绑定所有技能输入动作

**参数**:
- `InputConfig`: 输入配置数据资产
- `Object`: 拥有者对象
- `PressedFunc`: 按下回调函数
- `ReleasedFunc`: 释放回调函数
- `HeldFunc`: 按住回调函数

### 绑定实现

```cpp
template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UAuraInputComponent::BindAbilityActions(...)
{
    check(InputConfig);
    
    // 遍历所有输入动作
    for (const FAuraInputAction& Action : InputConfig->AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            // 绑定按下事件
            if (PressedFunc)
            {
                BindAction(
                    Action.InputAction,
                    ETriggerEvent::Started,
                    Object,
                    PressedFunc,
                    Action.InputTag
                );
            }
            
            // 绑定释放事件
            if (ReleasedFunc)
            {
                BindAction(
                    Action.InputAction,
                    ETriggerEvent::Completed,
                    Object,
                    ReleasedFunc,
                    Action.InputTag
                );
            }
            
            // 绑定按住事件
            if (HeldFunc)
            {
                BindAction(
                    Action.InputAction,
                    ETriggerEvent::Triggered,
                    Object,
                    HeldFunc,
                    Action.InputTag
                );
            }
        }
    }
}
```

---

## 输入绑定流程

### 完整流程

```
1. 创建 InputConfig 数据资产
   ↓
2. 配置 InputActions 和 InputTags
   ↓
3. 在 PlayerController 中绑定
   ↓
4. 输入触发 → Enhanced Input
   ↓
5. 调用回调函数（Pressed/Held/Released）
   ↓
6. 传递 InputTag 到 AbilitySystemComponent
   ↓
7. 查找对应技能并激活
```

### 在 PlayerController 中绑定

```cpp
void AAuraPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // 获取输入组件
    UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
    
    // 获取输入配置
    const UAuraInputConfig* InputConfig = GetDefault<UAuraInputConfig>();
    
    // 绑定技能输入
    AuraInputComponent->BindAbilityActions(
        InputConfig,
        this,
        &ThisClass::InputTagPressed,
        &ThisClass::InputTagReleased,
        &ThisClass::InputTagHeld
    );
}
```

### 输入处理函数

```cpp
// 输入按下
void AAuraPlayerController::InputTagPressed(FGameplayTag InputTag)
{
    if (UAuraAbilitySystemComponent* ASC = GetAuraASC())
    {
        ASC->AbilityInputTagPressed(InputTag);
    }
}

// 输入按住
void AAuraPlayerController::InputTagHeld(FGameplayTag InputTag)
{
    if (UAuraAbilitySystemComponent* ASC = GetAuraASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}

// 输入释放
void AAuraPlayerController::InputTagReleased(FGameplayTag InputTag)
{
    if (UAuraAbilitySystemComponent* ASC = GetAuraASC())
    {
        ASC->AbilityInputTagReleased(InputTag);
    }
}
```

---

## 输入标签系统

### 支持的输入标签

- **InputTag.LMB**: 鼠标左键
- **InputTag.RMB**: 鼠标右键
- **InputTag.1**: 数字键 1
- **InputTag.2**: 数字键 2
- **InputTag.3**: 数字键 3
- **InputTag.4**: 数字键 4
- **InputTag.Passive.1**: 被动技能 1
- **InputTag.Passive.2**: 被动技能 2

### 标签初始化

在 `AuraGameplayTags.cpp` 中初始化：

```cpp
GameplayTags.InputTag_LMB = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("InputTag.LMB"));
// ... 其他标签
```

---

## 技能输入处理

### AbilitySystemComponent 处理

#### InputTagPressed

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;
    
    FScopedAbilityListLock ActiveScopeLoc(*this);
    
    // 查找所有可激活的技能
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        // 检查是否匹配 InputTag
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            // 通知输入按下
            AbilitySpecInputPressed(AbilitySpec);
            
            // 如果技能已激活，触发复制事件
            if (AbilitySpec.IsActive())
            {
                InvokeReplicatedEvent(
                    EAbilityGenericReplicatedEvent::InputPressed,
                    AbilitySpec.Handle,
                    AbilitySpec.ActivationInfo.GetActivationPredictionKey()
                );
            }
        }
    }
}
```

#### InputTagHeld

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;
    
    FScopedAbilityListLock ActiveScopeLoc(*this);
    
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            // 通知输入按住
            AbilitySpecInputPressed(AbilitySpec);
            
            // 如果技能未激活，尝试激活
            if (!AbilitySpec.IsActive())
            {
                TryActivateAbility(AbilitySpec.Handle);
            }
        }
    }
}
```

#### InputTagReleased

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;
    
    FScopedAbilityListLock ActiveScopeLoc(*this);
    
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && 
            AbilitySpec.IsActive())
        {
            // 通知输入释放
            AbilitySpecInputReleased(AbilitySpec);
            
            // 触发复制事件
            InvokeReplicatedEvent(
                EAbilityGenericReplicatedEvent::InputReleased,
                AbilitySpec.Handle,
                AbilitySpec.ActivationInfo.GetActivationPredictionKey()
            );
        }
    }
}
```

---

## 配置指南

### 步骤 1: 创建 Input Actions

1. 在编辑器中创建 Input Actions
2. 配置触发条件（按下、按住、释放）
3. 设置输入键位（鼠标、键盘等）

### 步骤 2: 创建 InputConfig 数据资产

1. 创建 `UAuraInputConfig` 数据资产
2. 在 `AbilityInputActions` 数组中添加条目
3. 为每个条目设置：
   - `InputAction`: 对应的 Input Action
   - `InputTag`: 对应的 GameplayTag

### 步骤 3: 配置 Input Mapping Context

1. 创建 Input Mapping Context
2. 将 Input Actions 映射到物理输入
3. 在 PlayerController 中设置 Context

### 步骤 4: 在 PlayerController 中绑定

```cpp
void AAuraPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // 获取输入组件
    UAuraInputComponent* AuraInputComponent = 
        CastChecked<UAuraInputComponent>(InputComponent);
    
    // 获取输入配置
    const UAuraInputConfig* InputConfig = GetDefault<UAuraInputConfig>();
    
    // 绑定技能输入
    AuraInputComponent->BindAbilityActions(
        InputConfig,
        this,
        &ThisClass::InputTagPressed,
        &ThisClass::InputTagReleased,
        &ThisClass::InputTagHeld
    );
}
```

---

## 使用示例

### 自定义输入处理

```cpp
// 在 PlayerController 中
void AAuraPlayerController::InputTagPressed(FGameplayTag InputTag)
{
    // 自定义处理逻辑
    if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
    {
        // 处理鼠标左键按下
    }
    
    // 传递给 AbilitySystemComponent
    if (UAuraAbilitySystemComponent* ASC = GetAuraASC())
    {
        ASC->AbilityInputTagPressed(InputTag);
    }
}
```

### 添加新输入

#### 步骤 1: 添加 InputTag

```cpp
// AuraGameplayTags.h
FGameplayTag InputTag_5;

// AuraGameplayTags.cpp
GameplayTags.InputTag_5 = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("InputTag.5"));
```

#### 步骤 2: 创建 Input Action

在编辑器中创建新的 Input Action（例如：数字键 5）

#### 步骤 3: 添加到 InputConfig

在 `InputConfig` 数据资产中添加新的 `FAuraInputAction` 条目

#### 步骤 4: 绑定技能

技能会自动绑定到对应的 InputTag

---

## 最佳实践

### 1. 输入设计

- **清晰的映射**: 保持输入动作和标签的清晰映射
- **一致性**: 使用统一的命名规范
- **文档化**: 记录所有输入映射

### 2. 性能优化

- **批量绑定**: 使用 `BindAbilityActions` 批量绑定
- **避免重复绑定**: 检查是否已绑定
- **及时清理**: 在适当时机清理绑定

### 3. 错误处理

- **空指针检查**: 检查 InputConfig 和 InputAction
- **标签验证**: 验证 InputTag 是否有效
- **日志记录**: 记录输入错误

---

## 总结

输入系统提供了灵活的输入绑定机制：

- ✅ **Enhanced Input 集成**: 使用 UE5 的新输入系统
- ✅ **数据驱动**: 通过数据资产配置输入
- ✅ **自动绑定**: 自动绑定所有技能输入
- ✅ **事件支持**: 支持按下、按住、释放事件
- ✅ **Tag 系统**: 通过 GameplayTag 管理输入

通过这个系统，开发者可以轻松配置和管理游戏输入。

