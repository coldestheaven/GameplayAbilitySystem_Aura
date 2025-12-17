# 被动技能系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [被动技能基础](#被动技能基础)
3. [被动技能特点](#被动技能特点)
4. [被动技能架构](#被动技能架构)
5. [被动技能实现](#被动技能实现)
6. [视觉效果系统](#视觉效果系统)
7. [被动技能激活流程](#被动技能激活流程)
8. [被动技能停用机制](#被动技能停用机制)
9. [创建被动技能](#创建被动技能)
10. [项目中的被动技能示例](#项目中的被动技能示例)
11. [最佳实践](#最佳实践)
12. [常见问题](#常见问题)

---

## 系统概述

被动技能（Passive Ability）是 GAS 系统中一种特殊的能力类型，与主动技能不同，被动技能：

- **自动激活**: 在角色初始化时自动激活
- **持续效果**: 持续生效，直到被停用
- **不可手动触发**: 不能通过输入触发
- **无消耗**: 通常不需要消耗资源（如法力）

在 Aura 项目中，被动技能用于实现：

- **持续增益效果**: 如光环保护、属性提升
- **自动触发效果**: 如生命汲取、法力汲取
- **被动防御**: 如自动格挡、伤害减免

### 核心组件

- **UAuraPassiveAbility**: 被动技能基类
- **UPassiveNiagaraComponent**: 被动技能视觉效果组件
- **UAuraAbilitySystemComponent**: 能力系统组件（管理被动技能）
- **FActivatePassiveEffect**: 被动效果激活委托
- **FDeactivatePassiveAbility**: 被动技能停用委托

---

## 被动技能基础

### 什么是被动技能

被动技能是一种自动激活并持续生效的能力，不需要玩家手动触发。被动技能通常：

1. **自动激活**: 在角色初始化时自动激活
2. **持续生效**: 持续存在直到被停用
3. **无输入绑定**: 不绑定到任何输入动作
4. **通过 GameplayEffect 实现效果**: 效果通过 GameplayEffect 配置

### 被动技能 vs 主动技能

| 特性 | 被动技能 | 主动技能 |
|------|----------|----------|
| **激活方式** | 自动激活 | 手动触发 |
| **输入绑定** | 无 | 有（LMB、RMB、1-4） |
| **持续时间** | 持续 | 瞬时或短暂 |
| **消耗** | 无 | 有（法力、冷却） |
| **状态管理** | 装备/未装备 | 锁定/可解锁/已解锁/已装备 |
| **视觉效果** | 持续特效 | 瞬时特效 |

---

## 被动技能特点

### 1. 自动激活

被动技能在角色初始化时自动激活，无需玩家操作：

```cpp
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
    const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities
)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
    {
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
        AbilitySpec.DynamicAbilityTags.AddTag(
            FAuraGameplayTags::Get().Abilities_Status_Equipped
        );
        GiveAbilityAndActivateOnce(AbilitySpec);  // 自动激活
    }
}
```

### 2. 持续效果

被动技能持续生效，直到被停用：

```cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 绑定停用委托
    if (UAuraAbilitySystemComponent* AuraASC = ...)
    {
        AuraASC->DeactivatePassiveAbility.AddUObject(
            this,
            &UAuraPassiveAbility::ReceiveDeactivate
        );
    }
    
    // 技能不会自动结束，持续生效
}
```

### 3. 不可手动触发

被动技能不绑定输入标签，无法通过输入触发：

```cpp
// 被动技能没有 StartupInputTag
// 不会响应输入事件
```

### 4. 通过 GameplayEffect 实现效果

被动技能的效果通过 GameplayEffect 配置：

```cpp
// 在被动技能蓝图中
// 1. 创建 GameplayEffect
// 2. 配置持续效果（Duration Policy: Infinite）
// 3. 配置增益效果（Modifiers）
// 4. 在技能激活时应用 Effect
```

---

## 被动技能架构

### 类层次结构

```
UGameplayAbility
    └── UAuraGameplayAbility
        └── UAuraPassiveAbility
```

### 核心类

#### UAuraPassiveAbility

被动技能基类，继承自 `UAuraGameplayAbility`：

```cpp
UCLASS()
class AURA_API UAuraPassiveAbility : public UAuraGameplayAbility
{
    GENERATED_BODY()
    
public:
    virtual void ActivateAbility(...) override;
    void ReceiveDeactivate(const FGameplayTag& AbilityTag);
};
```

**关键方法**:
- `ActivateAbility()`: 激活被动技能
- `ReceiveDeactivate()`: 接收停用通知

#### UPassiveNiagaraComponent

被动技能视觉效果组件，继承自 `UNiagaraComponent`：

```cpp
UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
    GENERATED_BODY()
    
public:
    UPassiveNiagaraComponent();
    
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag PassiveSpellTag;
    
protected:
    virtual void BeginPlay() override;
    void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);
    void ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC);
};
```

**关键属性**:
- `PassiveSpellTag`: 被动技能标签，用于匹配激活事件

**关键方法**:
- `OnPassiveActivate()`: 响应被动效果激活事件
- `ActivateIfEquipped()`: 如果已装备则激活

---

## 被动技能实现

### UAuraPassiveAbility 实现

#### 头文件

```cpp
// AuraPassiveAbility.h
UCLASS()
class AURA_API UAuraPassiveAbility : public UAuraGameplayAbility
{
    GENERATED_BODY()
    
public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;
    
    void ReceiveDeactivate(const FGameplayTag& AbilityTag);
};
```

#### 实现文件

```cpp
// AuraPassiveAbility.cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 绑定停用委托
    if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
            GetAvatarActorFromActorInfo()
        )
    ))
    {
        AuraASC->DeactivatePassiveAbility.AddUObject(
            this,
            &UAuraPassiveAbility::ReceiveDeactivate
        );
    }
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
    if (AbilityTags.HasTagExact(AbilityTag))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}
```

### UPassiveNiagaraComponent 实现

#### 头文件

```cpp
// PassiveNiagaraComponent.h
UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
    GENERATED_BODY()
    
public:
    UPassiveNiagaraComponent();
    
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag PassiveSpellTag;
    
protected:
    virtual void BeginPlay() override;
    void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);
    void ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC);
};
```

#### 实现文件

```cpp
// PassiveNiagaraComponent.cpp
UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
    bAutoActivate = false;  // 不自动激活
}

void UPassiveNiagaraComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())
    ))
    {
        // 绑定激活事件
        AuraASC->ActivatePassiveEffect.AddUObject(
            this,
            &UPassiveNiagaraComponent::OnPassiveActivate
        );
        ActivateIfEquipped(AuraASC);
    }
    else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
    {
        // 如果 ASC 还未注册，等待注册
        CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC)
        {
            if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(
                UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())
            ))
            {
                AuraASC->ActivatePassiveEffect.AddUObject(
                    this,
                    &UPassiveNiagaraComponent::OnPassiveActivate
                );
                ActivateIfEquipped(AuraASC);
            }
        });
    }
}

void UPassiveNiagaraComponent::ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC)
{
    const bool bStartupAbilitiesGiven = AuraASC->bStartupAbilitiesGiven;
    if (bStartupAbilitiesGiven)
    {
        if (AuraASC->GetStatusFromAbilityTag(PassiveSpellTag) == 
            FAuraGameplayTags::Get().Abilities_Status_Equipped)
        {
            Activate();  // 如果已装备，激活视觉效果
        }
    }
}

void UPassiveNiagaraComponent::OnPassiveActivate(
    const FGameplayTag& AbilityTag,
    bool bActivate
)
{
    if (AbilityTag.MatchesTagExact(PassiveSpellTag))
    {
        if (bActivate && !IsActive())
        {
            Activate();  // 激活视觉效果
        }
        else
        {
            Deactivate();  // 停用视觉效果
        }
    }
}
```

---

## 视觉效果系统

### PassiveNiagaraComponent 工作原理

1. **组件初始化**: 在角色构造时创建组件
2. **绑定事件**: 在 `BeginPlay` 时绑定 `ActivatePassiveEffect` 委托
3. **检查装备状态**: 如果技能已装备，自动激活视觉效果
4. **响应激活事件**: 当技能激活/停用时，响应事件并更新视觉效果

### 在角色中添加视觉效果组件

```cpp
// AuraCharacterBase.h
UPROPERTY(VisibleAnywhere)
TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

UPROPERTY(VisibleAnywhere)
TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

UPROPERTY(VisibleAnywhere)
TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;
```

```cpp
// AuraCharacterBase.cpp
HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "HaloOfProtectionComponent"
);
HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
HaloOfProtectionNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_HaloOfProtection;

LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "LifeSiphonNiagaraComponent"
);
LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
LifeSiphonNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_LifeSiphon;

ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "ManaSiphonNiagaraComponent"
);
ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
ManaSiphonNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_ManaSiphon;
```

---

## 被动技能激活流程

### 1. 角色初始化

```
角色创建
    ↓
InitAbilityActorInfo()
    ↓
AddCharacterAbilities()
    ↓
AddCharacterPassiveAbilities()
```

### 2. 添加被动技能

```cpp
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
    const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities
)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
    {
        // 1. 创建 AbilitySpec
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
        
        // 2. 设置状态为已装备
        AbilitySpec.DynamicAbilityTags.AddTag(
            FAuraGameplayTags::Get().Abilities_Status_Equipped
        );
        
        // 3. 给予能力并立即激活一次
        GiveAbilityAndActivateOnce(AbilitySpec);
    }
}
```

### 3. 技能激活

```cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 获取 ASC
    if (UAuraAbilitySystemComponent* AuraASC = ...)
    {
        // 2. 绑定停用委托
        AuraASC->DeactivatePassiveAbility.AddUObject(
            this,
            &UAuraPassiveAbility::ReceiveDeactivate
        );
        
        // 3. 广播激活事件（触发视觉效果）
        AuraASC->MulticastActivatePassiveEffect(AbilityTag, true);
    }
    
    // 4. 应用 GameplayEffect（在蓝图中配置）
    // ...
    
    // 技能不会自动结束，持续生效
}
```

### 4. 视觉效果激活

```
ActivatePassiveEffect 委托广播
    ↓
PassiveNiagaraComponent::OnPassiveActivate()
    ↓
检查 AbilityTag 是否匹配
    ↓
激活/停用 Niagara 组件
```

---

## 被动技能停用机制

### 停用流程

被动技能可以通过以下方式停用：

1. **装备其他技能**: 当装备其他被动技能到同一槽位时
2. **手动停用**: 通过 `DeactivatePassiveAbility` 委托
3. **技能移除**: 当技能从角色中移除时

### 停用实现

```cpp
// 在 AuraAbilitySystemComponent 中
void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
    if (Spec)
    {
        const FGameplayTag Slot = GetSlotFromAbilityTag(GetAbilityTagFromSpec(*Spec));
        Spec->DynamicAbilityTags.RemoveTag(Slot);
        
        // 如果是被动技能，停用效果
        if (IsPassiveAbility(*Spec))
        {
            MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*Spec), false);
            DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*Spec));
        }
    }
}
```

### 接收停用通知

```cpp
void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
    if (AbilityTags.HasTagExact(AbilityTag))
    {
        // 结束技能
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}
```

---

## 创建被动技能

### 步骤 1: 创建 GameplayTag

#### 1.1 在头文件中添加 Tag

```cpp
// AuraGameplayTags.h
struct FAuraGameplayTags
{
    FGameplayTag Abilities_Passive_MyPassiveAbility;
};
```

#### 1.2 在实现文件中注册 Tag

```cpp
// AuraGameplayTags.cpp
GameplayTags.Abilities_Passive_MyPassiveAbility = 
    UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Abilities.Passive.MyPassiveAbility"),
        FString("My Passive Ability Tag")
    );
```

### 步骤 2: 创建被动技能蓝图

1. 在内容浏览器中，右键 → `Gameplay` → `Gameplay Ability`
2. 选择父类为 `AuraPassiveAbility`
3. 命名为 `BP_MyPassiveAbility`

### 步骤 3: 配置技能属性

在技能蓝图中：

- **Ability Tags**: 添加 `Abilities.Passive.MyPassiveAbility`
- **Ability Type**: 设置为 `Abilities.Type.Passive`
- **Activation Policy**: `OnSpawn` 或 `OnGiven`
- **Net Execution Policy**: `Server Only`

### 步骤 4: 创建 GameplayEffect

1. 创建 GameplayEffect 蓝图
2. 配置持续效果：
   - **Duration Policy**: `Infinite`
   - **Period**: 0（或设置周期执行）
3. 配置增益效果：
   - 添加 Modifiers（如增加护甲、抗性等）

### 步骤 5: 在技能中应用 Effect

在被动技能蓝图的 `ActivateAbility` 中：

```
Event ActivateAbility
├── Apply GameplayEffect to Self
│   └── GameplayEffect: GE_MyPassiveEffect
└── (技能持续生效，不会自动结束)
```

### 步骤 6: 添加视觉效果组件（可选）

#### 6.1 在角色中添加组件

```cpp
// AuraCharacterBase.h
UPROPERTY(VisibleAnywhere)
TObjectPtr<UPassiveNiagaraComponent> MyPassiveNiagaraComponent;
```

```cpp
// AuraCharacterBase.cpp
MyPassiveNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "MyPassiveComponent"
);
MyPassiveNiagaraComponent->SetupAttachment(EffectAttachComponent);
MyPassiveNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_MyPassiveAbility;
```

#### 6.2 配置 Niagara 特效

在编辑器中：
1. 选择 `MyPassiveNiagaraComponent`
2. 设置 `Asset` 为 Niagara 特效资产
3. 设置 `Passive Spell Tag` 为 `Abilities.Passive.MyPassiveAbility`

### 步骤 7: 添加到角色

在角色蓝图中：
1. 找到 `Startup Passive Abilities` 数组
2. 添加 `BP_MyPassiveAbility`

---

## 项目中的被动技能示例

### 1. HaloOfProtection (光环保护)

**功能**: 提供持续的保护增益效果

**实现**:
- 继承自 `UAuraPassiveAbility`
- 通过 GameplayEffect 增加护甲和抗性
- 使用 `PassiveNiagaraComponent` 显示光环特效

**标签**: `Abilities.Passive.HaloOfProtection`

### 2. LifeSiphon (生命汲取)

**功能**: 在造成伤害时自动恢复生命值

**实现**:
- 继承自 `UAuraPassiveAbility`
- 监听伤害事件
- 根据伤害值恢复生命
- 使用 `PassiveNiagaraComponent` 显示汲取特效

**标签**: `Abilities.Passive.LifeSiphon`

### 3. ManaSiphon (法力汲取)

**功能**: 在造成伤害时自动恢复法力值

**实现**:
- 继承自 `UAuraPassiveAbility`
- 监听伤害事件
- 根据伤害值恢复法力
- 使用 `PassiveNiagaraComponent` 显示汲取特效

**标签**: `Abilities.Passive.ManaSiphon`

---

## 最佳实践

### 1. 技能设计

- **单一职责**: 每个被动技能只实现一个功能
- **效果清晰**: 效果应该明确且易于理解
- **平衡性**: 考虑游戏平衡，避免过强或过弱

### 2. 性能优化

- **避免频繁更新**: 使用事件驱动而非每帧更新
- **对象池**: 对于频繁创建的效果，使用对象池
- **LOD 系统**: 根据距离使用不同细节级别的视觉效果

### 3. 视觉效果

- **清晰可见**: 视觉效果应该清晰可见，但不遮挡视线
- **性能友好**: 使用高效的粒子系统
- **状态反馈**: 视觉效果应该反映技能状态

### 4. GameplayEffect 配置

- **持续时间**: 使用 `Infinite` 持续时间
- **周期执行**: 如果需要定期触发，设置 `Period`
- **标签管理**: 正确使用标签来管理效果

### 5. 网络同步

- **服务器权威**: 被动技能应该在服务器上激活
- **客户端同步**: 视觉效果在客户端同步
- **委托广播**: 使用 `MulticastActivatePassiveEffect` 广播激活事件

---

## 常见问题

### 问题 1: 被动技能未激活

**原因**: 技能未添加到 `StartupPassiveAbilities` 数组

**解决方案**:
1. 检查技能是否添加到角色的 `StartupPassiveAbilities` 数组
2. 检查 `AddCharacterPassiveAbilities` 是否被调用
3. 检查技能是否在服务器上激活

### 问题 2: 视觉效果未显示

**原因**: `PassiveNiagaraComponent` 未正确配置

**解决方案**:
1. 检查组件是否添加到角色
2. 检查 `PassiveSpellTag` 是否匹配技能标签
3. 检查 `ActivatePassiveEffect` 委托是否绑定
4. 检查技能状态是否为 `Equipped`

### 问题 3: 效果未生效

**原因**: GameplayEffect 未正确应用

**解决方案**:
1. 检查 GameplayEffect 是否在技能激活时应用
2. 检查 GameplayEffect 的配置是否正确
3. 检查 Modifiers 是否配置正确
4. 检查标签要求是否满足

### 问题 4: 技能无法停用

**原因**: 停用委托未正确绑定或广播

**解决方案**:
1. 检查 `DeactivatePassiveAbility` 委托是否绑定
2. 检查停用事件是否被广播
3. 检查 `ReceiveDeactivate` 是否正确实现

### 问题 5: 多个被动技能冲突

**原因**: 技能效果相互冲突

**解决方案**:
1. 检查 GameplayEffect 的标签要求
2. 使用不同的标签来区分效果
3. 考虑使用堆叠系统

---

## 完整示例

### 示例: 创建护盾被动技能

#### 1. 添加 Tag

```cpp
// AuraGameplayTags.h
FGameplayTag Abilities_Passive_Shield;

// AuraGameplayTags.cpp
GameplayTags.Abilities_Passive_Shield = 
    UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Abilities.Passive.Shield"),
        FString("Shield Passive Ability")
    );
```

#### 2. 创建技能蓝图

1. 创建 `BP_ShieldPassiveAbility`，继承自 `AuraPassiveAbility`
2. 配置 `Ability Tags`: `Abilities.Passive.Shield`
3. 配置 `Ability Type`: `Abilities.Type.Passive`

#### 3. 创建 GameplayEffect

1. 创建 `GE_ShieldPassive`
2. 配置 `Duration Policy`: `Infinite`
3. 添加 Modifier:
   - Attribute: `Shield`
   - Modifier Op: `Additive`
   - Magnitude: `50.0`

#### 4. 在技能中应用 Effect

在 `BP_ShieldPassiveAbility` 的 `ActivateAbility` 中：
- Apply `GE_ShieldPassive` to Self

#### 5. 添加视觉效果

```cpp
// AuraCharacterBase.h
UPROPERTY(VisibleAnywhere)
TObjectPtr<UPassiveNiagaraComponent> ShieldNiagaraComponent;

// AuraCharacterBase.cpp
ShieldNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "ShieldComponent"
);
ShieldNiagaraComponent->SetupAttachment(EffectAttachComponent);
ShieldNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_Shield;
```

#### 6. 添加到角色

在角色蓝图的 `Startup Passive Abilities` 中添加 `BP_ShieldPassiveAbility`

---

## 总结

被动技能系统是 GAS 中重要的组成部分：

- ✅ **自动激活**: 在角色初始化时自动激活
- ✅ **持续效果**: 持续生效直到被停用
- ✅ **视觉效果**: 通过 `PassiveNiagaraComponent` 显示特效
- ✅ **灵活配置**: 通过 GameplayEffect 配置效果
- ✅ **易于扩展**: 可以轻松创建新的被动技能

通过合理使用被动技能系统，可以实现丰富的持续效果和增益。

---

## 相关文档

- [GameplayAbility 系统文档](./GameplayAbility_System.md) - GameplayAbility 详细文档
- [GameplayEffect 系统文档](./GameplayEffect_System.md) - GameplayEffect 详细文档
- [GameplayTags 系统文档](./GameplayTags_System.md) - GameplayTags 详细文档
- [如何添加新技能指南](../Guides/How_To_Add_New_Ability.md) - 添加新技能的完整指南
- [HaloOfProtection 文档](../Abilities/HaloOfProtection.md) - 光环保护技能示例
- [LifeSiphon 文档](../Abilities/LifeSiphon.md) - 生命汲取技能示例
- [ManaSiphon 文档](../Abilities/ManaSiphon.md) - 法力汲取技能示例

