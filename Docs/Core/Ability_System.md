# 技能系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [技能类层次结构](#技能类层次结构)
3. [技能生命周期](#技能生命周期)
4. [技能状态管理](#技能状态管理)
5. [技能类型详解](#技能类型详解)
6. [技能激活流程](#技能激活流程)
7. [输入系统集成](#输入系统集成)
8. [技能槽位系统](#技能槽位系统)
9. [技能升级系统](#技能升级系统)
10. [具体技能实现](#具体技能实现)
11. [技能配置](#技能配置)
12. [扩展指南](#扩展指南)

---

## 系统概述

Aura 项目的技能系统基于 Unreal Engine 5.7 的 Gameplay Ability System (GAS) 构建，提供了完整的技能管理、激活、升级和配置系统。

### 核心特性

- **多类型技能支持**: 投射物、光束、近战、召唤、被动技能
- **技能状态管理**: Locked → Eligible → Unlocked → Equipped
- **输入绑定系统**: 支持多个输入槽位（LMB, RMB, 1-4）
- **技能升级系统**: 通过法术点升级技能等级
- **技能描述系统**: 动态生成技能描述和下一级描述
- **成本系统**: 法力消耗和冷却时间管理

---

## 技能类层次结构

### 类继承树

```
UGameplayAbility (UE5 Base)
    ↓
UAuraGameplayAbility (基础技能类)
    ├── UAuraDamageGameplayAbility (伤害技能基类)
    │   ├── UAuraProjectileSpell (投射物技能)
    │   │   ├── UAuraFireBolt (火球术)
    │   │   └── UAuraFireBlast (火球爆炸)
    │   │   └── UAuraArcaneShards (奥术碎片)
    │   ├── UAuraBeamSpell (光束技能)
    │   │   └── UElectrocute (电击)
    │   └── UAuraMeleeAttack (近战攻击)
    ├── UAuraSummonAbility (召唤技能)
    └── UAuraPassiveAbility (被动技能)
```

### 核心基类说明

#### UAuraGameplayAbility

所有技能的基础类，提供：

- **StartupInputTag**: 启动时的输入标签
- **GetDescription()**: 获取技能描述
- **GetNextLevelDescription()**: 获取下一级描述
- **GetLockedDescription()**: 获取锁定描述
- **GetManaCost()**: 获取法力消耗
- **GetCooldown()**: 获取冷却时间

#### UAuraDamageGameplayAbility

伤害技能基类，提供：

- **CauseDamage()**: 造成伤害
- **MakeDamageEffectParamsFromClassDefaults()**: 制作伤害效果参数
- **GetDamageAtLevel()**: 获取当前等级伤害
- **伤害参数配置**: DamageType, Damage, Debuff 相关参数
- **范围伤害支持**: RadialDamage 相关参数
- **击退和死亡冲量**: Knockback 和 DeathImpulse 参数

---

## 技能生命周期

### 技能状态机

```
┌─────────┐
│ Locked  │ ← 未达到等级要求
└────┬────┘
     │ 达到等级要求
     ↓
┌─────────────┐
│  Eligible   │ ← 可以使用法术点解锁
└────┬────────┘
     │ 消耗法术点
     ↓
┌─────────────┐
│  Unlocked   │ ← 已解锁但未装备
└────┬────────┘
     │ 装备到槽位
     ↓
┌─────────────┐
│  Equipped   │ ← 已装备，可以使用
└─────────────┘
```

### 状态说明

#### Locked (锁定)
- **条件**: 玩家等级 < 技能等级要求
- **行为**: 技能不可见或显示为锁定状态
- **解锁**: 达到等级要求后自动变为 Eligible

#### Eligible (符合条件)
- **条件**: 达到等级要求但未消耗法术点
- **行为**: 可以使用法术点解锁
- **解锁**: 消耗法术点后变为 Unlocked

#### Unlocked (已解锁)
- **条件**: 已消耗法术点解锁
- **行为**: 可以装备到输入槽位
- **装备**: 装备到槽位后变为 Equipped

#### Equipped (已装备)
- **条件**: 已装备到输入槽位
- **行为**: 可以通过输入激活
- **激活**: 满足条件（法力、冷却）时可以激活

### 技能激活流程

```
1. 输入检测
   ↓
2. 查找对应技能 (通过 InputTag)
   ↓
3. 检查激活条件
   - 技能状态是否为 Equipped
   - 是否有足够法力
   - 是否在冷却中
   - 其他条件检查
   ↓
4. 激活技能
   - TryActivateAbility()
   - ActivateAbility()
   ↓
5. 执行技能逻辑
   ↓
6. 技能完成/取消
   - EndAbility()
```

---

## 技能状态管理

### 状态标签

技能状态通过 `DynamicAbilityTags` 管理：

- `Abilities.Status.Locked`: 锁定状态
- `Abilities.Status.Eligible`: 符合条件状态
- `Abilities.Status.Unlocked`: 已解锁状态
- `Abilities.Status.Equipped`: 已装备状态

### 状态查询

```cpp
// 获取技能状态
FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);

// 获取技能状态（从 Spec）
static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
```

### 状态更新

#### 自动状态更新

当玩家升级时，系统会自动检查并更新技能状态：

```cpp
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
    // 检查每个技能
    // 如果达到等级要求且未解锁，设置为 Eligible
}
```

#### 手动状态更新

- **解锁技能**: `ServerSpendSpellPoint()` - 消耗法术点解锁
- **装备技能**: `ServerEquipAbility()` - 装备到槽位
- **升级技能**: `ServerSpendSpellPoint()` - 消耗法术点升级

---

## 技能类型详解

### 1. 投射物技能 (UAuraProjectileSpell)

#### 特点
- 生成投射物 Actor
- 支持多个投射物
- 支持追踪目标（Homing）
- 碰撞时造成伤害

#### 核心方法

```cpp
// 生成投射物
void SpawnProjectile(
    const FVector& ProjectileTargetLocation,
    const FGameplayTag& SocketTag,
    bool bOverridePitch = false,
    float PitchOverride = 0.f
);
```

#### 实现示例：FireBolt

```cpp
// 生成多个火球
void UAuraFireBolt::SpawnProjectiles(
    const FVector& ProjectileTargetLocation,
    const FGameplayTag& SocketTag,
    bool bOverridePitch,
    float PitchOverride,
    AActor* HomingTarget
)
{
    // 1. 获取发射位置
    const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(...);
    
    // 2. 计算多个投射物的旋转（扇形分布）
    TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(...);
    
    // 3. 生成每个投射物
    for (const FRotator& Rot : Rotations)
    {
        // 创建投射物
        AAuraProjectile* Projectile = SpawnActorDeferred<AAuraProjectile>(...);
        
        // 设置伤害参数
        Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 设置追踪目标
        if (HomingTarget)
        {
            Projectile->ProjectileMovement->HomingTargetComponent = ...;
        }
        
        // 完成生成
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

#### 配置参数

- **ProjectileClass**: 投射物类
- **NumProjectiles**: 投射物数量
- **ProjectileSpread**: 投射物散布角度（FireBolt）
- **HomingAccelerationMin/Max**: 追踪加速度范围（FireBolt）

### 2. 光束技能 (UAuraBeamSpell)

#### 特点
- 持续伤害
- 支持目标链（Chain）
- 目标死亡时自动切换目标

#### 核心方法

```cpp
// 存储鼠标数据
void StoreMouseDataInfo(const FHitResult& HitResult);

// 存储所有者变量
void StoreOwnerVariables();

// 追踪第一个目标
void TraceFirstTarget(const FVector& BeamTargetLocation);

// 存储额外目标
void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);
```

#### 实现示例：Electrocute

- 发射闪电光束
- 可以链式传播到多个目标
- 持续造成闪电伤害
- 有几率造成眩晕（Stun）

#### 配置参数

- **MaxNumShockTargets**: 最大连锁目标数
- **MouseHitLocation**: 鼠标点击位置
- **MouseHitActor**: 鼠标点击的 Actor

### 3. 近战攻击 (UAuraMeleeAttack)

#### 特点
- 使用动画蒙太奇
- 近战范围检测
- 武器碰撞检测

#### 实现
- 继承自 `UAuraDamageGameplayAbility`
- 通过动画通知触发伤害检测
- 使用 CombatInterface 获取攻击动画

### 4. 召唤技能 (UAuraSummonAbility)

#### 特点
- 生成召唤物
- 管理召唤物数量
- 支持多个召唤物类型

#### 核心方法

```cpp
// 获取生成位置
UFUNCTION(BlueprintCallable)
TArray<FVector> GetSpawnLocations();

// 获取随机召唤物类
UFUNCTION(BlueprintPure, Category="Summoning")
TSubclassOf<APawn> GetRandomMinionClass();
```

#### 配置参数

- **NumMinions**: 召唤物数量
- **MinionClasses**: 召唤物类数组
- **MinSpawnDistance**: 最小生成距离
- **MaxSpawnDistance**: 最大生成距离
- **SpawnSpread**: 生成散布角度

### 5. 被动技能 (UAuraPassiveAbility)

#### 特点
- 自动激活
- 持续效果
- 不可手动触发
- 可以停用

#### 激活流程

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
}
```

#### 停用机制

```cpp
void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
    if (AbilityTags.HasTagExact(AbilityTag))
    {
        EndAbility(..., true, true);
    }
}
```

#### 被动技能示例

- **HaloOfProtection**: 光环保护
- **LifeSiphon**: 生命汲取
- **ManaSiphon**: 法力汲取

---

## 技能激活流程

### 完整激活流程

```
┌─────────────────┐
│ 玩家输入        │
│ (LMB/RMB/1-4)   │
└────────┬────────┘
         ↓
┌─────────────────────────┐
│ AuraInputComponent      │
│ 检测输入                │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ AbilityInputTagPressed/ │
│ Held/Released            │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ AuraAbilitySystemComponent│
│ 查找对应技能             │
│ (通过 InputTag)          │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ 检查激活条件             │
│ - 状态是否为 Equipped   │
│ - 是否有足够法力        │
│ - 是否在冷却中          │
│ - 其他条件              │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ TryActivateAbility()     │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ Ability.ActivateAbility()│
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ 执行技能逻辑             │
│ - 生成投射物            │
│ - 发射光束              │
│ - 造成伤害              │
│ - 应用效果              │
└────────┬────────────────┘
         ↓
┌─────────────────────────┐
│ EndAbility()             │
│ - 技能完成              │
│ - 技能取消              │
└─────────────────────────┘
```

### 输入处理

#### InputTagPressed

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    // 查找对应技能
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            // 通知输入按下
            AbilitySpecInputPressed(AbilitySpec);
            
            // 如果技能已激活，触发复制事件
            if (AbilitySpec.IsActive())
            {
                InvokeReplicatedEvent(...);
            }
        }
    }
}
```

#### InputTagHeld

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
    // 查找对应技能
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
    // 查找对应技能
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && 
            AbilitySpec.IsActive())
        {
            // 通知输入释放
            AbilitySpecInputReleased(AbilitySpec);
            
            // 触发复制事件
            InvokeReplicatedEvent(...);
        }
    }
}
```

---

## 输入系统集成

### 输入标签

系统支持以下输入标签：

- **InputTag.LMB**: 鼠标左键
- **InputTag.RMB**: 鼠标右键
- **InputTag.1**: 数字键 1
- **InputTag.2**: 数字键 2
- **InputTag.3**: 数字键 3
- **InputTag.4**: 数字键 4
- **InputTag.Passive.1**: 被动技能 1
- **InputTag.Passive.2**: 被动技能 2

### 输入绑定

技能在添加到角色时自动绑定输入标签：

```cpp
void UAuraAbilitySystemComponent::AddCharacterAbilities(...)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
    {
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
        
        if (const UAuraGameplayAbility* AuraAbility = ...)
        {
            // 添加输入标签
            AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
            
            // 设置为已装备状态
            AbilitySpec.DynamicAbilityTags.AddTag(
                FAuraGameplayTags::Get().Abilities_Status_Equipped
            );
            
            GiveAbility(AbilitySpec);
        }
    }
}
```

---

## 技能槽位系统

### 槽位管理

每个技能可以分配到一个输入槽位，一个槽位只能有一个技能。

### 槽位操作

#### 检查槽位是否为空

```cpp
bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilityHasSlot(AbilitySpec, Slot))
        {
            return false;
        }
    }
    return true;
}
```

#### 获取槽位中的技能

```cpp
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
        {
            return &AbilitySpec;
        }
    }
    return nullptr;
}
```

#### 分配槽位

```cpp
void UAuraAbilitySystemComponent::AssignSlotToAbility(
    FGameplayAbilitySpec& Spec, 
    const FGameplayTag& Slot
)
{
    // 清除旧槽位
    ClearSlot(&Spec);
    
    // 添加新槽位
    Spec.DynamicAbilityTags.AddTag(Slot);
}
```

#### 清除槽位

```cpp
void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
    const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
    Spec->DynamicAbilityTags.RemoveTag(Slot);
}
```

### 装备技能流程

```cpp
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(
    const FGameplayTag& AbilityTag, 
    const FGameplayTag& Slot
)
{
    // 1. 获取技能 Spec
    FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag);
    
    // 2. 检查状态是否有效
    if (Status == Equipped || Status == Unlocked)
    {
        // 3. 如果槽位已有技能，清除旧技能
        if (!SlotIsEmpty(Slot))
        {
            FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
            
            // 如果是被动技能，停用
            if (IsPassiveAbility(*SpecWithSlot))
            {
                MulticastActivatePassiveEffect(..., false);
                DeactivatePassiveAbility.Broadcast(...);
            }
            
            ClearSlot(SpecWithSlot);
        }
        
        // 4. 如果技能未激活，激活被动技能
        if (!AbilityHasAnySlot(*AbilitySpec))
        {
            if (IsPassiveAbility(*AbilitySpec))
            {
                TryActivateAbility(AbilitySpec->Handle);
                MulticastActivatePassiveEffect(AbilityTag, true);
            }
            
            // 更新状态为 Equipped
            AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
            AbilitySpec->DynamicAbilityTags.AddTag(Abilities_Status_Equipped);
        }
        
        // 5. 分配槽位
        AssignSlotToAbility(*AbilitySpec, Slot);
    }
}
```

---

## 技能升级系统

### 升级流程

```
1. 玩家升级获得法术点
   ↓
2. 打开技能菜单
   ↓
3. 选择技能
   ↓
4. 消耗法术点
   ↓
5. 升级技能或解锁新技能
```

### 升级实现

```cpp
void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(
    const FGameplayTag& AbilityTag
)
{
    FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag);
    
    // 消耗法术点
    IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
    
    const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
    FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
    
    // 如果状态是 Eligible，解锁技能
    if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
    {
        AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
        AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
        Status = GameplayTags.Abilities_Status_Unlocked;
    }
    // 如果状态是 Equipped 或 Unlocked，升级技能
    else if (Status == Equipped || Status == Unlocked)
    {
        AbilitySpec->Level += 1;
    }
    
    // 通知客户端更新
    ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
}
```

### 自动解锁

当玩家升级时，系统会自动检查并解锁符合条件的技能：

```cpp
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
    UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(...);
    
    for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
    {
        // 检查等级要求
        if (Level < Info.LevelRequirement) continue;
        
        // 如果技能不存在，创建并设置为 Eligible
        if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
        {
            FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
            AbilitySpec.DynamicAbilityTags.AddTag(
                FAuraGameplayTags::Get().Abilities_Status_Eligible
            );
            GiveAbility(AbilitySpec);
        }
    }
}
```

---

## 具体技能实现

### FireBolt (火球术)

#### 特点
- 投射物技能
- 支持多个火球（根据等级）
- 支持追踪目标
- 造成火焰伤害
- 有几率造成燃烧（Burn）

#### 实现细节

```cpp
// 生成多个火球
void UAuraFireBolt::SpawnProjectiles(...)
{
    // 1. 计算有效投射物数量（不超过等级）
    const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
    
    // 2. 计算扇形分布的旋转
    TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
        Forward, 
        FVector::UpVector, 
        ProjectileSpread, 
        EffectiveNumProjectiles
    );
    
    // 3. 生成每个火球
    for (const FRotator& Rot : Rotations)
    {
        // 创建投射物
        AAuraProjectile* Projectile = SpawnActorDeferred<AAuraProjectile>(...);
        
        // 设置伤害参数
        Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 设置追踪
        if (HomingTarget)
        {
            Projectile->ProjectileMovement->HomingTargetComponent = ...;
            Projectile->ProjectileMovement->HomingAccelerationMagnitude = 
                FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
        }
    }
}
```

### FireBlast (火球爆炸)

#### 特点
- 生成多个火球向四周发射
- 火球返回并爆炸
- 造成范围火焰伤害
- 有几率造成燃烧

#### 实现细节

```cpp
// 生成火球
TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
    // 1. 计算均匀分布的旋转（360度）
    TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
        Forward, 
        FVector::UpVector, 
        360.f, 
        NumFireBalls
    );
    
    // 2. 生成每个火球
    for (const FRotator& Rotator : Rotators)
    {
        AAuraFireBall* FireBall = SpawnActorDeferred<AAuraFireBall>(...);
        
        // 设置伤害参数
        FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
        FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 设置返回目标
        FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
    }
}
```

### Electrocute (电击)

#### 特点
- 光束技能
- 持续伤害
- 支持连锁传播（根据等级）
- 造成闪电伤害
- 有几率造成眩晕（Stun）

#### 实现细节

- 使用 `UAuraBeamSpell` 基类
- 通过蓝图实现光束视觉效果
- 自动处理目标死亡和切换
- 支持连锁到多个目标

### ArcaneShards (奥术碎片)

#### 特点
- 投射物技能
- 生成多个奥术碎片
- 造成奥术伤害
- 有几率造成奥术 Debuff

---

## 技能配置

### 技能信息数据资产 (UAbilityInfo)

每个技能需要在 `AbilityInfo` 数据资产中配置：

```cpp
struct FAuraAbilityInfo
{
    FGameplayTag AbilityTag;          // 技能标签
    FGameplayTag InputTag;           // 输入标签（运行时）
    FGameplayTag StatusTag;          // 状态标签（运行时）
    FGameplayTag CooldownTag;        // 冷却标签
    FGameplayTag AbilityType;        // 技能类型（Offensive/Passive）
    UTexture2D* Icon;                // 图标
    UMaterialInterface* BackgroundMaterial; // 背景材质
    int32 LevelRequirement;          // 等级要求
    TSubclassOf<UGameplayAbility> Ability; // 技能类
};
```

### GameplayEffect 配置

每个技能需要配置以下 GameplayEffect：

#### 1. Cost GameplayEffect (法力消耗)

- **类型**: Instant
- **修改器**: 减少 Mana 属性
- **值**: 可配置，支持按等级缩放

#### 2. Cooldown GameplayEffect (冷却时间)

- **类型**: HasDuration
- **持续时间**: 可配置，支持按等级缩放
- **标签**: 使用技能的 CooldownTag

#### 3. Damage GameplayEffect (伤害效果)

- **类型**: Instant
- **执行计算**: ExecCalc_Damage
- **修改器**: 修改 IncomingDamage 属性
- **SetByCaller**: 使用 DamageType 标签设置伤害值

### 技能描述系统

技能描述使用格式化字符串：

```cpp
FString UAuraFireBolt::GetDescription(int32 Level)
{
    return FString::Printf(TEXT(
        "<Title>FIRE BOLT</>\n\n"
        "<Small>Level: </><Level>%d</>\n"
        "<Small>ManaCost: </><ManaCost>%.1f</>\n"
        "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
        "<Default>Launches %d bolts of fire, "
        "exploding on impact and dealing: </>"
        "<Damage>%d</><Default> fire damage with"
        " a chance to burn</>"),
        Level, ManaCost, Cooldown, NumProjectiles, ScaledDamage
    );
}
```

#### 描述标签

- `<Title>...</>`: 标题
- `<Small>...</>`: 小文本
- `<Level>...</>`: 等级值
- `<ManaCost>...</>`: 法力消耗
- `<Cooldown>...</>`: 冷却时间
- `<Damage>...</>`: 伤害值
- `<Default>...</>`: 默认文本

---

## 扩展指南

### 创建新技能

#### 步骤 1: 创建技能类

```cpp
// MyNewAbility.h
UCLASS()
class AURA_API UMyNewAbility : public UAuraProjectileSpell
{
    GENERATED_BODY()
    
public:
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    virtual void ActivateAbility(...) override;
};
```

#### 步骤 2: 实现技能逻辑

```cpp
// MyNewAbility.cpp
void UMyNewAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 实现技能逻辑
    SpawnProjectile(TargetLocation, SocketTag);
    
    EndAbility(...);
}

FString UMyNewAbility::GetDescription(int32 Level)
{
    // 返回技能描述
}
```

#### 步骤 3: 配置技能信息

1. 打开 `AbilityInfo` 数据资产
2. 添加新的 `FAuraAbilityInfo` 条目
3. 设置所有必要字段

#### 步骤 4: 创建 GameplayEffect

1. 创建 Cost GameplayEffect
2. 创建 Cooldown GameplayEffect
3. 创建 Damage GameplayEffect（如果需要）

#### 步骤 5: 添加到角色

在角色的 `StartupAbilities` 数组中添加新技能类。

### 创建新技能类型

如果需要创建全新的技能类型：

1. 继承 `UAuraGameplayAbility` 或 `UAuraDamageGameplayAbility`
2. 实现 `ActivateAbility()` 方法
3. 添加必要的配置参数
4. 实现技能描述方法

### 技能调试

#### 查看技能状态

```cpp
// 在代码中
FGameplayTag Status = ASC->GetStatusFromAbilityTag(AbilityTag);
UE_LOG(LogTemp, Warning, TEXT("Ability Status: %s"), *Status.ToString());
```

#### 查看技能等级

```cpp
FGameplayAbilitySpec* Spec = ASC->GetSpecFromAbilityTag(AbilityTag);
if (Spec)
{
    int32 Level = Spec->Level;
}
```

#### 测试技能激活

- 使用控制台命令
- 在蓝图中直接调用
- 使用调试工具

---

## 最佳实践

### 1. 技能设计

- **单一职责**: 每个技能只做一件事
- **可配置**: 使用数据资产而非硬编码
- **可扩展**: 使用基类提供通用功能

### 2. 性能优化

- **对象池**: 对频繁创建的投射物使用对象池
- **延迟生成**: 使用 `SpawnActorDeferred` 设置参数
- **网络优化**: 只在服务器生成投射物

### 3. 网络同步

- **服务器权威**: 技能激活在服务器执行
- **客户端预测**: 使用 GAS 的预测系统
- **RPC 使用**: 合理使用 Server/Client/Multicast RPC

### 4. 错误处理

- **空指针检查**: 检查所有指针
- **有效性检查**: 检查输入参数
- **日志记录**: 记录重要事件

---

## 总结

Aura 项目的技能系统提供了：

- ✅ 完整的技能生命周期管理
- ✅ 多种技能类型支持
- ✅ 灵活的输入绑定系统
- ✅ 技能升级和状态管理
- ✅ 可扩展的架构设计
- ✅ 数据驱动的配置系统

通过这个系统，开发者可以轻松创建、配置和管理各种类型的技能，同时保持代码的清晰和可维护性。

