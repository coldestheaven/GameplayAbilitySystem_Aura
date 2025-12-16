# GameplayAbility 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [GameplayAbility 基础](#gameplayability-基础)
3. [能力生命周期](#能力生命周期)
4. [能力激活流程](#能力激活流程)
5. [项目中的能力类层次](#项目中的能力类层次)
6. [能力类型](#能力类型)
7. [能力状态管理](#能力状态管理)
8. [能力输入绑定](#能力输入绑定)
9. [能力消耗和冷却](#能力消耗和冷却)
10. [能力描述系统](#能力描述系统)
11. [创建新能力](#创建新能力)
12. [最佳实践](#最佳实践)
13. [常见问题](#常见问题)

---

## 系统概述

GameplayAbility 是 GAS 系统中用于实现技能和能力的核心组件。在 Aura 项目中，GameplayAbility 用于：

- **技能实现**: 实现各种主动和被动技能
- **伤害处理**: 处理伤害计算和应用
- **效果应用**: 应用各种游戏效果
- **输入绑定**: 将输入动作绑定到能力
- **状态管理**: 管理能力的状态（锁定、解锁、装备）

### 核心组件

- **UGameplayAbility**: GameplayAbility 基类
- **UAuraGameplayAbility**: 项目的基础能力类
- **UAuraDamageGameplayAbility**: 伤害能力基类
- **UAuraProjectileSpell**: 投射物法术
- **UAuraBeamSpell**: 光束法术
- **UAuraMeleeAttack**: 近战攻击
- **UAuraSummonAbility**: 召唤能力
- **UAuraPassiveAbility**: 被动能力

---

## GameplayAbility 基础

### 能力结构

GameplayAbility 包含以下主要部分：

1. **能力标签** (Ability Tags): 标识能力类型
2. **激活标签** (Activation Tags): 激活时授予的标签
3. **阻塞标签** (Block Tags): 阻塞激活的标签
4. **消耗** (Cost): 激活消耗（如法力）
5. **冷却** (Cooldown): 冷却时间
6. **触发事件** (Trigger Events): 触发能力的事件

### 能力规格 (Ability Spec)

`FGameplayAbilitySpec` 包含能力的运行时信息：

```cpp
struct FGameplayAbilitySpec
{
    TSubclassOf<UGameplayAbility> Ability;      // 能力类
    FGameplayAbilitySpecHandle Handle;          // 能力句柄
    int32 Level;                                // 能力等级
    FGameplayTagContainer DynamicAbilityTags;   // 动态标签
    FActiveGameplayEffectHandle CooldownHandle; // 冷却句柄
    // ...
};
```

---

## 能力生命周期

### 生命周期阶段

```
┌─────────────┐
│ 未激活      │
│ (Inactive)  │
└──────┬──────┘
       ↓
┌─────────────┐
│ 激活中      │
│ (Activating)│
└──────┬──────┘
       ↓
┌─────────────┐
│ 已激活      │
│ (Active)    │
└──────┬──────┘
       ↓
┌─────────────┐
│ 结束中      │
│ (Ending)    │
└──────┬──────┘
       ↓
┌─────────────┐
│ 未激活      │
│ (Inactive)  │
└─────────────┘
```

### 生命周期函数

#### 1. CanActivateAbility

检查能力是否可以激活：

```cpp
virtual bool CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags = nullptr,
    const FGameplayTagContainer* TargetTags = nullptr,
    OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
) const;
```

#### 2. ActivateAbility

激活能力：

```cpp
virtual void ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData = nullptr
);
```

#### 3. EndAbility

结束能力：

```cpp
virtual void EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility = true,
    bool bWasCancelled = false
);
```

---

## 能力激活流程

### 完整激活流程

```
输入检测
    ↓
AbilityInputTagPressed/Held
    ↓
查找对应 AbilitySpec
    ↓
检查激活条件
    - 状态是否为 Equipped
    - 是否有足够法力
    - 是否在冷却中
    - 其他条件
    ↓
TryActivateAbility()
    ↓
CanActivateAbility()
    ↓
检查 Cost
    ↓
ActivateAbility()
    ↓
执行能力逻辑
    ↓
EndAbility()
```

### 输入处理

#### InputTagPressed

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            AbilitySpecInputPressed(AbilitySpec);
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
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            AbilitySpecInputPressed(AbilitySpec);
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
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
        {
            AbilitySpecInputReleased(AbilitySpec);
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

## 项目中的能力类层次

### 类层次结构

```
UGameplayAbility (UE5 Base)
    ↓
UAuraGameplayAbility
    ├── UAuraDamageGameplayAbility
    │   ├── UAuraProjectileSpell
    │   ├── UAuraBeamSpell
    │   └── UAuraMeleeAttack
    ├── UAuraSummonAbility
    └── UAuraPassiveAbility
```

### UAuraGameplayAbility

项目的基础能力类：

```cpp
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category="Input")
    FGameplayTag StartupInputTag;

    virtual FString GetDescription(int32 Level);
    virtual FString GetNextLevelDescription(int32 Level);
    static FString GetLockedDescription(int32 Level);

protected:
    float GetManaCost(float InLevel = 1.f) const;
    float GetCooldown(float InLevel = 1.f) const;
};
```

**核心功能**:
- 输入标签绑定
- 能力描述生成
- 消耗和冷却查询

### UAuraDamageGameplayAbility

伤害能力基类：

```cpp
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void CauseDamage(AActor* TargetActor);

    UFUNCTION(BlueprintPure)
    FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(...) const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    FGameplayTag DamageType;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    FScalableFloat Damage;

    // Debuff 参数
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffChance = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffDamage = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffFrequency = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffDuration = 5.f;
};
```

**核心功能**:
- 伤害处理
- Debuff 参数配置
- 伤害参数生成

---

## 能力类型

### 1. 投射物法术 (UAuraProjectileSpell)

**特点**:
- 生成投射物 Actor
- 支持多投射物
- 支持追踪目标

**实现示例**:

```cpp
void UAuraProjectileSpell::ActivateAbility(...)
{
    // 生成投射物
    const bool bIsServer = HasAuthority(&ActivationInfo);
    if (!bIsServer) return;

    // 获取目标位置
    FVector ProjectileTargetLocation = GetProjectileTargetLocation();

    // 生成投射物
    AAuraProjectile* Projectile = GetWorld()->SpawnActor<AAuraProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation
    );

    // 设置伤害参数
    Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
}
```

### 2. 光束法术 (UAuraBeamSpell)

**特点**:
- 持续伤害
- 目标追踪
- 光束视觉效果

**实现示例**:

```cpp
void UAuraBeamSpell::ActivateAbility(...)
{
    // 查找目标
    FHitResult HitResult = GetTargetUnderMouse();

    // 创建光束
    if (BeamTargetActor)
    {
        // 应用持续伤害
        // 更新光束视觉效果
    }
}
```

### 3. 近战攻击 (UAuraMeleeAttack)

**特点**:
- 使用动画蒙太奇
- 近战范围检测
- 武器碰撞检测

**实现示例**:

```cpp
void UAuraMeleeAttack::ActivateAbility(...)
{
    // 播放动画蒙太奇
    const FTaggedMontage TaggedMontage = GetRandomTaggedMontageFromArray(AttackMontages);
    PlayMontage(TaggedMontage);
}
```

### 4. 召唤能力 (UAuraSummonAbility)

**特点**:
- 生成召唤物
- 管理召唤物数量
- 召唤物 AI 控制

**实现示例**:

```cpp
void UAuraSummonAbility::ActivateAbility(...)
{
    // 检查召唤物数量限制
    if (CurrentSummons >= MaxSummons) return;

    // 生成召唤物
    AActor* Summon = GetWorld()->SpawnActor<AActor>(
        SummonClass,
        SpawnLocation,
        SpawnRotation
    );

    CurrentSummons++;
}
```

### 5. 被动能力 (UAuraPassiveAbility)

**特点**:
- 自动激活
- 持续效果
- 不可手动触发

**实现示例**:

```cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    // 应用被动效果
    // 持续存在直到移除
}
```

---

## 能力状态管理

### 能力状态

能力有四种状态：

1. **Locked** (锁定): 未解锁，需要达到等级要求
2. **Eligible** (可解锁): 达到等级要求，可以使用法术点解锁
3. **Unlocked** (已解锁): 已解锁但未装备
4. **Equipped** (已装备): 已装备到输入槽位

### 状态转换

```
Locked → Eligible → Unlocked → Equipped
         (Level Up)  (Spend Point) (Equip)
```

### 状态查询

```cpp
FGameplayTag StatusTag = ASC->GetStatusFromAbilityTag(AbilityTag);
if (StatusTag == FAuraGameplayTags::Get().Abilities_Status_Equipped)
{
    // 能力已装备
}
```

---

## 能力输入绑定

### 输入标签

能力通过输入标签绑定到输入动作：

- `InputTag.LMB`: 鼠标左键
- `InputTag.RMB`: 鼠标右键
- `InputTag.1` - `InputTag.4`: 数字键 1-4

### 绑定流程

1. **设置 StartupInputTag**: 在能力蓝图中设置
2. **添加到 AbilitySpec**: 在 `AddCharacterAbilities` 中添加
3. **输入检测**: 通过 `AbilityInputTagPressed/Held/Released` 检测
4. **激活能力**: 调用 `TryActivateAbility`

### 槽位管理

```cpp
// 检查槽位是否为空
bool bIsEmpty = ASC->SlotIsEmpty(SlotTag);

// 获取槽位中的能力
FGameplayAbilitySpec* Spec = ASC->GetSpecWithSlot(SlotTag);

// 分配槽位给能力
ASC->AssignSlotToAbility(AbilitySpec, SlotTag);
```

---

## 能力消耗和冷却

### 消耗 (Cost)

能力消耗通过 Cost GameplayEffect 实现：

```cpp
// 获取消耗
float ManaCost = GetManaCost(GetAbilityLevel());

// 检查是否有足够消耗
if (ManaCost > CurrentMana)
{
    // 无法激活
    return;
}
```

### 冷却 (Cooldown)

能力冷却通过 Cooldown GameplayEffect 实现：

```cpp
// 获取冷却时间
float Cooldown = GetCooldown(GetAbilityLevel());

// 检查是否在冷却中
FGameplayTag CooldownTag = GetCooldownTag();
if (HasMatchingGameplayTag(CooldownTag))
{
    // 在冷却中
    return;
}
```

---

## 能力描述系统

### 描述生成

能力描述通过以下函数生成：

```cpp
// 获取当前等级描述
FString Description = GetDescription(GetAbilityLevel());

// 获取下一等级描述
FString NextLevelDescription = GetNextLevelDescription(GetAbilityLevel() + 1);

// 获取锁定描述
FString LockedDescription = GetLockedDescription(RequiredLevel);
```

### 描述格式

描述使用富文本格式：

```cpp
FString UAuraGameplayAbility::GetDescription(int32 Level)
{
    return FString::Printf(
        TEXT("<Default>%s, </><Level>%d</>"),
        L"Ability Name",
        Level
    );
}
```

---

## 创建新能力

### 步骤 1: 创建 C++ 类

```cpp
// MyAbility.h
UCLASS()
class AURA_API UMyAbility : public UAuraGameplayAbility
{
    GENERATED_BODY()

public:
    UMyAbility();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData = nullptr
    ) override;
};
```

```cpp
// MyAbility.cpp
void UMyAbility::ActivateAbility(...)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 实现能力逻辑
    // ...

    // 结束能力
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
```

### 步骤 2: 创建蓝图

1. 创建继承自 `MyAbility` 的蓝图
2. 配置能力属性
3. 设置输入标签
4. 配置 Cost 和 Cooldown

### 步骤 3: 添加到角色

在角色的 `StartupAbilities` 中添加能力类。

---

## 最佳实践

### 1. 能力设计

- **单一职责**: 每个能力只负责一个功能
- **可复用性**: 设计可复用的能力基类
- **清晰命名**: 使用清晰的命名

### 2. 性能考虑

- **避免 Tick**: 使用事件驱动而非 Tick
- **对象池**: 对于频繁创建的对象，使用对象池
- **网络优化**: 合理使用网络复制

### 3. 错误处理

- **空指针检查**: 检查所有指针
- **条件验证**: 验证激活条件
- **优雅降级**: 处理失败情况

---

## 常见问题

### 问题 1: 能力无法激活

**原因**: 条件不满足或配置错误

**解决方案**:
1. 检查能力状态是否为 Equipped
2. 检查是否有足够消耗
3. 检查是否在冷却中
4. 检查阻塞标签

### 问题 2: 能力未执行逻辑

**原因**: ActivateAbility 未正确实现

**解决方案**:
1. 检查是否调用了 Super::ActivateAbility
2. 检查能力逻辑是否正确实现
3. 检查是否调用了 EndAbility

### 问题 3: 输入未响应

**原因**: 输入绑定配置错误

**解决方案**:
1. 检查 StartupInputTag 是否正确设置
2. 检查输入系统是否正确配置
3. 检查能力是否已装备

---

## 总结

GameplayAbility 是 GAS 系统的核心组件，用于：

- ✅ **技能实现**: 实现各种主动和被动技能
- ✅ **伤害处理**: 处理伤害计算和应用
- ✅ **输入绑定**: 将输入动作绑定到能力
- ✅ **状态管理**: 管理能力的状态

通过合理使用 GameplayAbility，可以实现灵活、可扩展的技能系统。

---

## 相关文档

- [GameplayTags 系统文档](./GameplayTags_System.md) - GameplayTags 详细文档
- [GameplayEffect 系统文档](./GameplayEffect_System.md) - GameplayEffect 详细文档
- [属性系统文档](./Attribute_System.md) - 属性系统实现
- [伤害计算系统文档](./Damage_Calculation.md) - 伤害计算实现
- [如何添加新技能指南](../Guides/How_To_Add_New_Ability.md) - 添加新技能的完整指南

