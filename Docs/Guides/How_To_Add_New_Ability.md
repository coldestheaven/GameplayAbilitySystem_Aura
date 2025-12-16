# 如何添加新技能 - 完整步骤指南

## 目录

1. [概述](#概述)
2. [步骤概览](#步骤概览)
3. [详细步骤](#详细步骤)
4. [技能类型选择](#技能类型选择)
5. [完整示例](#完整示例)
6. [常见问题](#常见问题)

---

## 概述

本文档详细说明如何在 Aura 项目中添加一个新技能。添加技能需要多个步骤，包括创建技能类、添加 GameplayTag、配置数据资产等。

### 前置要求

- 熟悉 C++ 和 Unreal Engine
- 了解 Gameplay Ability System (GAS)
- 了解项目结构

---

## 步骤概览

添加新技能的完整流程：

```
1. 确定技能类型和基类
   ↓
2. 创建技能 C++ 类
   ↓
3. 添加 GameplayTag
   ↓
4. 创建技能蓝图
   ↓
5. 创建 GameplayEffect（Cost 和 Cooldown）
   ↓
6. 在 AbilityInfo 数据资产中添加技能信息
   ↓
7. 配置技能参数
   ↓
8. 添加到角色的 StartupAbilities（可选）
   ↓
9. 测试技能
```

---

## 详细步骤

### 步骤 1: 确定技能类型和基类

根据技能功能选择合适的基类：

#### 技能基类选择

##### UAuraGameplayAbility（基础技能类）

**适用场景**: 通用技能，不造成伤害或需要自定义实现

**特点**:
- 最基础的技能类
- 需要自己实现所有逻辑
- 适合特殊技能

**使用场景**: 治疗技能、增益技能、特殊效果技能

##### UAuraDamageGameplayAbility（伤害技能基类）

**适用场景**: 所有造成伤害的技能

**提供的功能**:
- `CauseDamage()`: 造成伤害
- `MakeDamageEffectParamsFromClassDefaults()`: 创建伤害参数
- `GetDamageAtLevel()`: 获取当前等级伤害

**可配置参数**:
- `Damage`: 伤害值（Scalable Float，支持按等级缩放）
- `DamageType`: 伤害类型（Fire, Lightning, Arcane, Physical）
- `DamageEffectClass`: 伤害 GameplayEffect 类
- `DebuffChance`: Debuff 触发几率
- `DebuffDamage`: Debuff 伤害值
- `DebuffDuration`: Debuff 持续时间
- `DebuffFrequency`: Debuff 触发频率
- `DeathImpulseMagnitude`: 死亡冲量大小
- `KnockbackForceMagnitude`: 击退力度
- `KnockbackChance`: 击退几率
- `bIsRadialDamage`: 是否为范围伤害
- `RadialDamageInnerRadius`: 范围伤害内半径
- `RadialDamageOuterRadius`: 范围伤害外半径

**继承类**:
- **UAuraProjectileSpell**: 投射物技能
- **UAuraBeamSpell**: 光束技能

##### UAuraProjectileSpell（投射物技能）

**适用场景**: 发射投射物的技能（火球、冰箭、奥术飞弹等）

**提供的功能**:
- `SpawnProjectile()`: 生成单个投射物
- 自动处理投射物生成和伤害参数传递

**可配置参数**:
- `ProjectileClass`: 投射物类（必须继承自 `AAuraProjectile`）
- `NumProjectiles`: 投射物数量（默认 5）

**扩展示例**: FireBolt（支持多个投射物、追踪、扇形分布）

##### UAuraBeamSpell（光束技能）

**适用场景**: 持续光束技能（闪电链、激光等）

**提供的功能**:
- `StoreMouseDataInfo()`: 存储鼠标数据
- `StoreOwnerVariables()`: 存储所有者变量
- `TraceFirstTarget()`: 追踪第一个目标
- `StoreAdditionalTargets()`: 存储额外目标（连锁）

**可配置参数**:
- `MaxNumShockTargets`: 最大连锁目标数

**扩展示例**: Electrocute（闪电链，支持多目标连锁）

##### UAuraMeleeAttack（近战攻击）

**适用场景**: 近战攻击技能

**特点**:
- 简单直接的伤害技能
- 通常配合动画蒙太奇使用

##### UAuraSummonAbility（召唤技能）

**适用场景**: 召唤生物的技能

**提供的功能**:
- `GetSpawnLocations()`: 获取生成位置
- `GetRandomMinionClass()`: 获取随机召唤物类

**可配置参数**:
- `NumMinions`: 召唤数量
- `MinionClasses`: 召唤物类数组
- `MinSpawnDistance`: 最小生成距离
- `MaxSpawnDistance`: 最大生成距离
- `SpawnSpread`: 生成散布角度

##### UAuraPassiveAbility（被动技能）

**适用场景**: 被动效果技能

**特点**:
- 自动激活
- 持续生效
- 不需要输入
- 提供 `ReceiveDeactivate()` 方法处理停用

**扩展示例**: HaloOfProtection, LifeSiphon, ManaSiphon

#### 选择指南

**选择投射物技能，如果**:
- 技能发射物理投射物
- 需要投射物碰撞检测
- 需要投射物视觉效果

**选择光束技能，如果**:
- 技能是持续光束
- 需要目标链（Chain）
- 需要持续伤害

**选择召唤技能，如果**:
- 技能召唤生物
- 需要多个召唤物
- 需要生成位置控制

**选择被动技能，如果**:
- 技能自动生效
- 不需要玩家输入
- 持续提供效果

**选择基础技能类，如果**:
- 技能有特殊需求
- 不适用任何现有基类
- 需要完全自定义实现

---

### 步骤 2: 创建技能 C++ 类

#### 2.1 创建头文件

在 `Source/Aura/Public/AbilitySystem/Abilities/` 目录下创建头文件。

**示例：创建冰霜箭技能**

```cpp
// AuraIceArrow.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraIceArrow.generated.h"

/**
 * 冰霜箭技能
 * 发射冰霜箭，造成冰霜伤害并可能减速敌人
 */
UCLASS()
class AURA_API UAuraIceArrow : public UAuraProjectileSpell
{
    GENERATED_BODY()
    
public:
    UAuraIceArrow();
    
    // 重写描述方法
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    // 技能特定参数
    UPROPERTY(EditDefaultsOnly, Category = "IceArrow")
    float SlowPercentage = 0.3f;  // 减速百分比
    
    UPROPERTY(EditDefaultsOnly, Category = "IceArrow")
    float SlowDuration = 3.f;  // 减速持续时间
};
```

#### 2.2 创建实现文件

在 `Source/Aura/Private/AbilitySystem/Abilities/` 目录下创建实现文件。

```cpp
// AuraIceArrow.cpp
#include "AbilitySystem/Abilities/AuraIceArrow.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"

UAuraIceArrow::UAuraIceArrow()
{
    // 设置技能标签
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    AbilityTags.AddTag(Tags.Abilities_Ice_IceArrow);
    
    // 设置输入标签（例如：鼠标右键）
    StartupInputTag = Tags.InputTag_RMB;
    
    // 设置激活所需标签（可选）
    // ActivationRequiredTags.AddTag(...);
    
    // 设置激活阻止标签（可选）
    // ActivationBlockedTags.AddTag(...);
}

FString UAuraIceArrow::GetDescription(int32 Level)
{
    // 获取当前等级的伤害值
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    
    // 获取法力消耗（使用绝对值，因为 Cost 是负数）
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    
    // 获取冷却时间
    const float Cooldown = GetCooldown(Level);
    
    // 格式化描述字符串
    // 注意：使用 TEXT() 宏包装字符串以支持 Unicode
    // 使用格式化标签来设置文本样式
    return FString::Printf(
        TEXT(
            // 标题
            "<Title>ICE ARROW</>\n\n"
            
            // 等级
            "<Small>Level: </><Level>%d</>\n"
            // 法力消耗
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            // 冷却时间
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            
            // 描述
            "<Default>Launches an ice arrow, "
            "dealing </>"
            // 伤害值
            "<Damage>%d</>"
            "<Default> ice damage with a chance to slow enemies.</>\n\n"
            
            // 特殊效果
            "<Small>Slow Effect: </><Default>%.0f%% for %.1f seconds</>"
        ),
        // 参数值
        Level,
        ManaCost,
        Cooldown,
        ScaledDamage,
        SlowPercentage * 100.f,
        SlowDuration
    );
}

FString UAuraIceArrow::GetNextLevelDescription(int32 Level)
{
    // 获取下一级和当前级的伤害值
    const int32 NextLevelDamage = Damage.GetValueAtLevel(Level + 1);
    const int32 CurrentLevelDamage = Damage.GetValueAtLevel(Level);
    const int32 DamageIncrease = NextLevelDamage - CurrentLevelDamage;
    
    // 获取下一级的法力消耗和冷却时间
    const float NextManaCost = FMath::Abs(GetManaCost(Level + 1));
    const float NextCooldown = GetCooldown(Level + 1);
    
    return FString::Printf(
        TEXT(
            "<Title>NEXT LEVEL: </>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Damage: </><Damage>%d</><Default> (+%d)</>"
        ),
        Level + 1,
        NextManaCost,
        NextCooldown,
        NextLevelDamage,
        DamageIncrease
    );
}
```

#### 2.3 描述字符串格式化标签

技能描述使用特殊的格式化标签来设置文本样式：

- `<Title>...</>`: 标题文本（大号、加粗）
- `<Small>...</>`: 小文本（用于标签）
- `<Level>...</>`: 等级值（特殊颜色）
- `<ManaCost>...</>`: 法力消耗值（特殊颜色）
- `<Cooldown>...</>`: 冷却时间值（特殊颜色）
- `<Damage>...</>`: 伤害值（特殊颜色，通常为红色）
- `<Default>...</>`: 默认文本（普通颜色）

#### 2.4 实现 ActivateAbility（如果需要自定义激活逻辑）

对于投射物技能，通常不需要重写 `ActivateAbility`，因为基类已经处理了。但如果需要自定义逻辑：

```cpp
void UAuraIceArrow::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 自定义激活逻辑
    // 例如：播放动画、生成特效等
    
    // 对于投射物技能，通常在蓝图中调用 SpawnProjectile
}
```

#### 2.5 编译项目

1. 在 Visual Studio 中打开项目
2. 右键点击项目 → "Build"
3. 确保编译成功，没有错误
4. 如果有错误，检查：
   - 头文件包含是否正确
   - 类名和命名空间是否正确
   - GameplayTag 是否已定义

#### 2.3 编译项目

在 Visual Studio 中编译项目，确保代码编译通过。

---

### 步骤 3: 添加 GameplayTag

#### 3.1 在 AuraGameplayTags.h 中添加 Tag

```cpp
// AuraGameplayTags.h
struct FAuraGameplayTags
{
    // ... 现有 Tags ...
    
    // 添加新技能的 Tag
    FGameplayTag Abilities_Ice_IceArrow;
    
    // 添加冷却 Tag（如果需要）
    FGameplayTag Cooldown_Ice_IceArrow;
};
```

#### 3.2 在 AuraGameplayTags.cpp 中初始化 Tag

```cpp
// AuraGameplayTags.cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有初始化 ...
    
    // 初始化技能 Tag
    GameplayTags.Abilities_Ice_IceArrow = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Abilities.Ice.IceArrow"),
            FString("Ice Arrow ability")
        );
    
    // 初始化冷却 Tag
    GameplayTags.Cooldown_Ice_IceArrow = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Cooldown.Ice.IceArrow"),
            FString("Ice Arrow cooldown")
        );
}
```

#### 3.3 添加伤害类型 Tag（如果是新伤害类型）

如果技能使用新的伤害类型（例如冰霜伤害），需要添加：

```cpp
// 在 AuraGameplayTags.h 中
FGameplayTag Damage_Ice;
FGameplayTag Attributes_Resistance_Ice;

// 在 AuraGameplayTags.cpp 中初始化
GameplayTags.Damage_Ice = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Damage.Ice"), FString("Ice damage"));

GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Attributes.Resistance.Ice"), FString("Ice resistance"));

// 添加到映射
GameplayTags.DamageTypesToResistances.Add(
    GameplayTags.Damage_Ice,
    GameplayTags.Attributes_Resistance_Ice
);
```

---

### 步骤 4: 创建技能蓝图

#### 4.1 创建蓝图类

1. 在编辑器中，右键点击 `AuraIceArrow` C++ 类
2. 选择 "Create Blueprint class based on AuraIceArrow"
3. 命名为 `BP_GA_IceArrow`

#### 4.2 配置技能属性

在蓝图中设置：

- **Ability Tags**: `Abilities.Ice.IceArrow`
- **Activation Owned Tags**: 根据需要添加
- **Activation Blocked Tags**: 根据需要添加
- **Activation Required Tags**: 根据需要添加

#### 4.3 配置技能参数

根据技能类型配置：

- **ProjectileClass**: 投射物类（如果是投射物技能）
- **Damage**: 伤害值（Scalable Float）
- **ManaCost**: 法力消耗
- **Cooldown**: 冷却时间
- **SlowPercentage**: 减速百分比（技能特定参数）

---

### 步骤 5: 创建 GameplayEffect

每个技能需要创建三个 GameplayEffect：Cost（消耗）、Cooldown（冷却）和 Damage（伤害，如果是伤害技能）。

#### 5.1 创建法力消耗 GameplayEffect

**目的**: 消耗角色的法力值

**详细步骤**:

1. **创建 GameplayEffect**
   - 在编辑器中创建新的 GameplayEffect
   - 命名为 `GE_IceArrow_Cost`
   - 建议放在 `Content/Blueprints/AbilitySystem/Effects/Cost/` 目录

2. **配置基本属性**
   - **Duration Policy**: `Instant`（立即生效）
   - **Period**: 0（不周期性触发）

3. **添加修改器（Modifier）**
   - 点击 "Add Modifier" 按钮
   - **Attribute**: 选择 `Mana`（从 `AuraAttributeSet`）
   - **Modifier Op**: `Subtract`（减法，消耗法力）
   - **Magnitude Calculation Type**: `SetByCaller`（由调用者设置值）
   - **SetByCaller Magnitude**: `Data.ManaCost`（这是 GAS 的标准标签）

4. **配置标签（可选）**
   - **Asset Tags**: 可以添加标签用于识别
   - **Granted Tags**: 通常不需要

**重要提示**:
- `Data.ManaCost` 是 GAS 内置的 SetByCaller 标签
- 技能会自动从 Cost GameplayEffect 中读取法力消耗值
- 法力消耗值在技能蓝图中通过 `ManaCost` 参数配置

#### 5.2 创建冷却 GameplayEffect

**目的**: 设置技能的冷却时间，防止技能被频繁使用

**详细步骤**:

1. **创建 GameplayEffect**
   - 命名为 `GE_IceArrow_Cooldown`
   - 建议放在 `Content/Blueprints/AbilitySystem/Effects/Cooldown/` 目录

2. **配置基本属性**
   - **Duration Policy**: `HasDuration`（有持续时间）
   - **Duration Magnitude**: 
     - **Calculation Type**: `ScalableFloat`（支持按等级缩放）
     - 或使用固定值（例如 5.0）
   - **Period**: 0（不周期性触发）

3. **配置标签**
   - **Granted Tags**: 添加 `Cooldown.Ice.IceArrow`
     - 这个标签会阻止技能激活
     - 标签会在冷却时间结束后自动移除

4. **配置标签需求（Ongoing Tag Requirements）**
   - **Require Tags**: 留空
   - **Ignore Tags**: 添加 `Cooldown.Ice.IceArrow`
     - 这确保在冷却期间，技能无法激活

**冷却时间配置选项**:

**选项 1: 固定冷却时间**
```
Duration Magnitude Calculation Type: ScalableFloat
Value: 5.0 (固定 5 秒)
```

**选项 2: 按等级缩放的冷却时间**
```
Duration Magnitude Calculation Type: ScalableFloat
在曲线表中配置不同等级的冷却时间
```

#### 5.3 创建伤害 GameplayEffect（如果是伤害技能）

**目的**: 对目标造成伤害

**详细步骤**:

1. **创建 GameplayEffect**
   - 命名为 `GE_IceArrow_Damage`
   - 建议放在 `Content/Blueprints/AbilitySystem/Effects/Damage/` 目录

2. **配置基本属性**
   - **Duration Policy**: `Instant`（立即生效）
   - **Period**: 0

3. **配置执行计算（Execution Calculation）**
   - **Execution Calculation**: 选择 `ExecCalc_Damage`
     - 这是项目的自定义伤害计算器
     - 处理护甲、暴击、抗性等计算

4. **添加修改器**
   - **Attribute**: `IncomingDamage`（从 `AuraAttributeSet`）
   - **Modifier Op**: `Additive`（加法）
   - **Magnitude Calculation Type**: `SetByCaller`（由调用者设置）
   - **SetByCaller Magnitude**: `Damage.Ice`（伤害类型标签）
     - 注意：这里使用伤害类型标签，不是 `Data.Damage`

5. **配置标签（可选）**
   - **Asset Tags**: 可以添加伤害类型标签

**重要提示**:
- 伤害值通过 `SetByCaller` 在技能激活时设置
- 使用伤害类型标签（如 `Damage.Ice`）而不是通用标签
- 伤害计算由 `ExecCalc_Damage` 处理，包括护甲、暴击等

#### 5.4 在技能蓝图中关联 GameplayEffect

1. **打开技能蓝图** (`BP_GA_IceArrow`)

2. **设置 Cost**
   - 在 "Ability Cost" 部分
   - 添加 `GE_IceArrow_Cost`
   - 技能激活时会自动应用此效果

3. **设置 Cooldown**
   - 在 "Cooldown" 部分
   - 添加 `GE_IceArrow_Cooldown`
   - 技能激活后会应用此效果

4. **设置伤害效果**（在技能逻辑中）
   - 伤害效果不在蓝图的 Cost/Cooldown 中设置
   - 而是在技能激活逻辑中通过 `MakeDamageEffectParamsFromClassDefaults()` 创建
   - 投射物会自动使用 `DamageEffectClass` 中配置的效果

#### 5.5 GameplayEffect 配置检查清单

**Cost GameplayEffect**:
- ✅ Duration Policy = Instant
- ✅ Modifier: Mana, Subtract, SetByCaller(Data.ManaCost)

**Cooldown GameplayEffect**:
- ✅ Duration Policy = HasDuration
- ✅ Duration Magnitude 已设置
- ✅ Granted Tags 包含 CooldownTag
- ✅ Ignore Tags 包含 CooldownTag

**Damage GameplayEffect**:
- ✅ Duration Policy = Instant
- ✅ Execution Calculation = ExecCalc_Damage
- ✅ Modifier: IncomingDamage, Additive, SetByCaller(DamageType)

---

### 步骤 6: 在 AbilityInfo 数据资产中添加技能信息

AbilityInfo 数据资产存储了所有技能的信息，用于 UI 显示和技能管理。

#### 6.1 打开 AbilityInfo 数据资产

1. 在编辑器中找到 `DA_AbilityInfo` 数据资产
2. 通常在 `Content/Data/` 目录下
3. 双击打开

#### 6.2 添加技能信息条目

在 `AbilityInformation` 数组中点击 "Add" 按钮，添加新条目并配置：

##### 必需字段

- **Ability Tag**: `Abilities.Ice.IceArrow`
  - 必须与技能类中设置的 Tag 完全匹配
  - 用于识别和查找技能

- **Input Tag**: `InputTag.RMB`（根据技能输入设置）
  - 决定技能绑定到哪个输入
  - 可选值：
    - `InputTag.LMB`: 鼠标左键
    - `InputTag.RMB`: 鼠标右键
    - `InputTag.1` 到 `InputTag.4`: 数字键 1-4
    - `InputTag.Passive.1` 或 `InputTag.Passive.2`: 被动技能

- **Status Tag**: `Abilities.Status.Locked`（初始状态）
  - 技能初始状态，通常为 `Locked`
  - 状态会在游戏运行时动态更新
  - 可能的状态：
    - `Abilities.Status.Locked`: 锁定（未解锁）
    - `Abilities.Status.Eligible`: 符合条件（可以解锁）
    - `Abilities.Status.Unlocked`: 已解锁（未装备）
    - `Abilities.Status.Equipped`: 已装备

- **Cooldown Tag**: `Cooldown.Ice.IceArrow`
  - 必须与 Cooldown GameplayEffect 中的 Granted Tag 匹配
  - 用于检测技能是否在冷却中

- **Ability Type**: `Abilities.Type.Offensive`（或 Passive）
  - 技能类型，影响技能菜单中的分类
  - 可选值：
    - `Abilities.Type.Offensive`: 攻击技能
    - `Abilities.Type.Passive`: 被动技能
    - `Abilities.Type.None`: 无类型

- **Ability**: `BP_GA_IceArrow`（技能蓝图类）
  - 技能蓝图类的引用
  - 必须设置为创建的技能蓝图

##### 可选字段

- **Icon**: 技能图标纹理
  - 在技能菜单中显示的图标
  - 建议尺寸：256x256 或 512x512
  - 格式：PNG 或 TGA

- **Background Material**: 背景材质
  - 技能图标的背景材质
  - 用于区分不同类型的技能

- **Level Requirement**: 解锁等级（例如 5）
  - 技能解锁所需的玩家等级
  - 玩家达到此等级后，技能状态变为 `Eligible`

#### 6.3 验证配置

确保以下内容正确：

- ✅ Ability Tag 与技能类中的 Tag 匹配
- ✅ Input Tag 已正确设置
- ✅ Cooldown Tag 与 Cooldown GameplayEffect 匹配
- ✅ Ability Type 正确
- ✅ Ability 引用指向正确的蓝图类
- ✅ Level Requirement 合理（通常 1-50）

#### 6.4 保存数据资产

1. 点击 "Save" 按钮保存
2. 确保没有编译错误
3. 如果修改了 C++ 代码，需要重新编译项目

---

### 步骤 7: 配置技能参数

在技能蓝图中配置所有技能参数，这些参数决定了技能的行为和效果。

#### 7.1 打开技能蓝图

1. 打开创建的技能蓝图（`BP_GA_IceArrow`）
2. 在 "Details" 面板中可以看到所有可配置的参数

#### 7.2 配置基础参数（所有技能）

##### Input 参数

- **Startup Input Tag**: `InputTag.RMB`
  - 技能的输入标签
  - 必须与 AbilityInfo 中的 Input Tag 匹配

##### Ability Tags

- **Ability Tags**: 添加 `Abilities.Ice.IceArrow`
  - 技能的主要标签
  - 用于识别技能

- **Activation Owned Tags**: 根据需要添加
  - 技能激活时拥有的标签
  - 可用于阻止其他技能激活

- **Activation Blocked Tags**: 根据需要添加
  - 阻止技能激活的标签
  - 例如：眩晕、沉默等状态标签

- **Activation Required Tags**: 根据需要添加
  - 技能激活所需的标签
  - 例如：需要特定状态才能使用

#### 7.3 配置投射物技能参数

如果技能继承自 `UAuraProjectileSpell`：

##### Projectile 参数

- **Projectile Class**: 选择投射物类
  - 必须继承自 `AAuraProjectile`
  - 例如：`BP_Projectile_FireBolt`、`BP_Projectile_IceArrow`
  - 投射物类需要单独创建（见步骤 7.5）

- **Num Projectiles**: 投射物数量
  - 默认值：5
  - 实际数量受技能等级限制：`FMath::Min(NumProjectiles, AbilityLevel)`

##### 投射物散布（FireBolt 示例）

如果技能支持多个投射物和散布：

- **Projectile Spread**: 投射物散布角度（度）
  - 例如：90.0（90度扇形）
  - 投射物会在此角度内均匀分布

- **Max Num Projectiles**: 最大投射物数量
  - 限制投射物数量上限

- **Homing Acceleration Min**: 追踪加速度最小值
- **Homing Acceleration Max**: 追踪加速度最大值
- **Launch Homing Projectiles**: 是否启用追踪
  - 如果启用，投射物会追踪目标

#### 7.4 配置伤害技能参数

如果技能继承自 `UAuraDamageGameplayAbility`：

##### Damage 参数

- **Damage**: 伤害值（Scalable Float）
  - **Calculation Type**: `ScalableFloat`
  - 在曲线表中配置不同等级的伤害值
  - 例如：
    - Level 1: 50
    - Level 2: 75
    - Level 3: 100
    - ...

- **Damage Type**: 伤害类型标签
  - 例如：`Damage.Ice`、`Damage.Fire`、`Damage.Lightning`
  - 必须与 GameplayTag 中定义的伤害类型匹配

- **Damage Effect Class**: 伤害 GameplayEffect 类
  - 设置为 `GE_IceArrow_Damage`（步骤 5.3 中创建的）

##### Debuff 参数

- **Debuff Chance**: Debuff 触发几率（0-100）
  - 例如：30.0（30% 几率）

- **Debuff Damage**: Debuff 伤害值
  - 每次 Debuff 触发造成的伤害

- **Debuff Duration**: Debuff 持续时间（秒）
  - Debuff 持续的总时间

- **Debuff Frequency**: Debuff 触发频率（秒）
  - 每多少秒触发一次 Debuff 伤害
  - 例如：1.0（每秒触发一次）

##### 击退和死亡冲量

- **Death Impulse Magnitude**: 死亡冲量大小
  - 敌人死亡时受到的冲量
  - 用于物理效果

- **Knockback Force Magnitude**: 击退力度
  - 击退敌人的力度

- **Knockback Chance**: 击退几率（0-100）
  - 例如：20.0（20% 几率击退）

##### 范围伤害

- **Is Radial Damage**: 是否为范围伤害
  - 如果启用，伤害会在范围内传播

- **Radial Damage Inner Radius**: 范围伤害内半径
  - 内半径内的目标受到满额伤害

- **Radial Damage Outer Radius**: 范围伤害外半径
  - 外半径外的目标不受伤害
  - 内外半径之间的目标受到衰减伤害

#### 7.5 创建投射物类（如果需要）

如果技能使用投射物，需要创建投射物类：

##### 步骤 1: 创建投射物 C++ 类（可选）

如果不需要自定义逻辑，可以直接使用 `AAuraProjectile`。

##### 步骤 2: 创建投射物蓝图

1. 基于 `BP_Projectile`（或 `AAuraProjectile`）创建蓝图
2. 命名为 `BP_Projectile_IceArrow`

##### 步骤 3: 配置投射物

- **Sphere Component**: 碰撞球体
  - **Collision Object Type**: `Projectile`
  - **Collision Enabled**: `QueryOnly`
  - **Collision Responses**: 
    - `WorldDynamic`: Overlap
    - `WorldStatic`: Overlap
    - `Pawn`: Overlap

- **Projectile Movement**: 投射物移动组件
  - **Initial Speed**: 初始速度（例如 550）
  - **Max Speed**: 最大速度（例如 550）
  - **Projectile Gravity Scale**: 重力缩放（0 = 无重力）

- **Visual Effects**: 视觉效果
  - **Impact Effect**: 碰撞时的 Niagara 效果
  - **Impact Sound**: 碰撞音效
  - **Looping Sound**: 飞行时的循环音效

- **Life Span**: 生命周期（秒）
  - 投射物存在的最长时间
  - 例如：15.0（15秒后自动销毁）

#### 7.6 配置技能激活逻辑（在蓝图中）

对于投射物技能，通常需要在蓝图中实现激活逻辑：

##### 示例：投射物技能激活

1. **重写 ActivateAbility 事件**
   - 在蓝图中添加 "Event ActivateAbility" 节点

2. **获取目标位置**
   - 使用 "Get Hit Result Under Cursor" 或类似节点
   - 获取鼠标指向的位置

3. **调用 SpawnProjectile**
   - 调用 `SpawnProjectile` 函数
   - 传入目标位置和 Socket Tag（例如 `CombatSocket.RightHand`）

4. **播放动画（可选）**
   - 播放技能动画蒙太奇
   - 使用 "Play Montage" 节点

5. **结束技能**
   - 调用 "EndAbility" 节点

##### 示例：多投射物技能激活（FireBolt）

```cpp
// 在蓝图中或 C++ 中
void UAuraFireBolt::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 获取目标位置（从鼠标或目标）
    FVector TargetLocation = GetTargetLocation();
    
    // 生成多个投射物
    SpawnProjectiles(
        TargetLocation,
        FAuraGameplayTags::Get().CombatSocket_RightHand,
        false,  // bOverridePitch
        0.f,    // PitchOverride
        nullptr // HomingTarget
    );
    
    // 结束技能
    EndAbility(...);
}
```

#### 7.7 参数配置检查清单

**基础参数**:
- ✅ Startup Input Tag 已设置
- ✅ Ability Tags 已添加
- ✅ Activation Tags 已配置（如需要）

**投射物参数**:
- ✅ Projectile Class 已选择
- ✅ Num Projectiles 已设置
- ✅ Projectile Spread 已设置（如需要）

**伤害参数**:
- ✅ Damage 曲线表已配置
- ✅ Damage Type 已设置
- ✅ Damage Effect Class 已选择
- ✅ Debuff 参数已配置（如需要）

**投射物类**:
- ✅ 投射物蓝图已创建
- ✅ 碰撞已配置
- ✅ 移动参数已设置
- ✅ 视觉效果已配置

---

### 步骤 8: 添加到角色的 StartupAbilities（可选）

如果希望技能在角色创建时自动获得，需要将其添加到角色的初始技能列表中。

#### 8.1 在角色蓝图中添加

**适用场景**: 所有角色都获得此技能

**步骤**:

1. 打开角色蓝图（例如 `BP_AuraCharacter`）
2. 在 "Details" 面板中找到 "Startup Abilities" 数组
3. 点击 "Add" 按钮
4. 选择 `BP_GA_IceArrow`
5. 保存蓝图

**注意**: 
- 添加到 `Startup Abilities` 的技能会在角色创建时自动获得
- 技能会自动设置为 `Equipped` 状态
- 技能会自动绑定到 `StartupInputTag` 指定的输入

#### 8.2 在 CharacterClassInfo 中配置

**适用场景**: 特定职业获得此技能

**步骤**:

1. **打开 CharacterClassInfo 数据资产**
   - 找到 `DA_CharacterClassInfo` 数据资产
   - 通常在 `Content/Data/` 目录下

2. **选择职业**
   - 在 `CharacterClassInformation` 映射中找到目标职业
   - 例如：`Elementalist`、`Warrior`、`Ranger`

3. **添加到 StartupAbilities**
   - 在对应职业的 `StartupAbilities` 数组中添加技能
   - 点击 "Add" 按钮
   - 选择 `BP_GA_IceArrow`

4. **添加到 CommonAbilities（可选）**
   - 如果希望所有职业都获得，添加到 `CommonAbilities` 数组
   - `CommonAbilities` 中的技能会给予所有职业

**职业特定配置示例**:

```
Elementalist:
  StartupAbilities:
    - BP_GA_FireBolt
    - BP_GA_IceArrow  ← 添加这里
    - BP_GA_Electrocute

Warrior:
  StartupAbilities:
    - BP_GA_MeleeAttack
    - (不添加 IceArrow)

Ranger:
  StartupAbilities:
    - BP_GA_MeleeAttack
    - BP_GA_IceArrow  ← 或者添加这里
```

#### 8.3 技能获得时机

技能在以下时机获得：

1. **首次创建角色时**
   - 从 `StartupAbilities` 或 `CharacterClassInfo` 中获得
   - 技能状态为 `Equipped`

2. **加载存档时**
   - 从存档数据中恢复技能
   - 保持之前的技能状态和等级

3. **升级解锁时**
   - 当玩家达到 `LevelRequirement` 时
   - 技能状态变为 `Eligible`（可以解锁）

#### 8.4 不添加到 StartupAbilities 的情况

如果技能不添加到 `StartupAbilities`：

- 技能初始状态为 `Locked`
- 玩家需要达到 `LevelRequirement` 才能解锁
- 玩家需要使用法术点解锁技能
- 技能解锁后状态变为 `Unlocked`
- 玩家需要手动装备技能

**推荐做法**:
- **初始技能**: 添加到 `StartupAbilities`（例如：基础攻击）
- **高级技能**: 不添加到 `StartupAbilities`，让玩家解锁（例如：高级法术）

---

### 步骤 9: 测试技能

测试是确保技能正常工作的重要步骤。

#### 9.1 编译和运行

1. **编译项目**
   - 在 Visual Studio 中编译
   - 确保没有编译错误
   - 如果有错误，检查：
     - C++ 代码语法
     - 头文件包含
     - GameplayTag 定义

2. **打开编辑器**
   - 启动 Unreal Editor
   - 等待编译完成

3. **运行游戏**
   - 点击 "Play" 按钮
   - 或使用快捷键（PIE）

#### 9.2 测试检查清单

##### 基础功能测试

- ✅ **技能可以激活**
  - 按下对应的输入键
  - 技能应该正常激活
  - 如果没有激活，检查：
    - Input Tag 是否正确绑定
    - 技能是否已装备
    - 是否有阻止标签

- ✅ **法力消耗正确**
  - 激活技能后，法力值应该减少
  - 检查减少的量是否与配置一致
  - 如果法力不足，技能应该无法激活

- ✅ **冷却时间正确**
  - 激活技能后，技能应该进入冷却
  - 冷却期间无法再次激活
  - 冷却时间结束后可以再次使用
  - 检查冷却时间是否与配置一致

##### 伤害测试（如果是伤害技能）

- ✅ **伤害计算正确**
  - 对敌人使用技能
  - 检查造成的伤害值
  - 验证伤害计算（考虑护甲、抗性等）
  - 检查暴击是否正常工作

- ✅ **Debuff 应用**
  - 如果配置了 Debuff，检查是否触发
  - 验证 Debuff 几率是否正确
  - 检查 Debuff 伤害和持续时间

- ✅ **击退效果**
  - 如果配置了击退，检查是否生效
  - 验证击退力度和方向

##### 视觉效果测试

- ✅ **投射物显示**
  - 投射物是否正确生成
  - 投射物移动是否正常
  - 投射物碰撞是否正确

- ✅ **碰撞效果**
  - 碰撞时的特效是否正确
  - 碰撞音效是否播放

- ✅ **动画播放**
  - 技能动画是否播放
  - 动画是否流畅

##### UI 测试

- ✅ **技能描述显示**
  - 在技能菜单中查看技能
  - 检查描述是否正确显示
  - 验证等级、伤害、消耗等信息

- ✅ **技能图标显示**
  - 图标是否正确显示
  - 背景材质是否正确

- ✅ **技能状态显示**
  - 锁定状态显示正确
  - 解锁状态显示正确
  - 装备状态显示正确

##### 网络测试（多人游戏）

- ✅ **服务器同步**
  - 在多人游戏中测试
  - 确保服务器和客户端同步
  - 检查投射物在客户端是否正确显示

- ✅ **伤害同步**
  - 确保伤害在服务器计算
  - 客户端正确显示伤害数字

#### 9.3 调试技巧

##### 使用日志

在技能代码中添加日志：

```cpp
UE_LOG(LogTemp, Warning, TEXT("Ice Arrow activated at level %d"), GetAbilityLevel());
UE_LOG(LogTemp, Warning, TEXT("Damage: %.2f"), Damage.GetValueAtLevel(GetAbilityLevel()));
```

##### 使用断点

在 Visual Studio 中设置断点：
- 在 `ActivateAbility` 中设置断点
- 在 `SpawnProjectile` 中设置断点
- 检查变量值是否正确

##### 使用蓝图调试

在技能蓝图中：
- 使用 "Print String" 节点输出调试信息
- 使用 "Break Hit Result" 检查碰撞信息
- 使用 "Draw Debug" 节点可视化信息

##### 检查 GameplayTag

在编辑器中：
- 打开 "GameplayTag Manager"
- 搜索技能的 Tag
- 确保 Tag 已正确注册

#### 9.4 常见问题排查

**技能无法激活**:
1. 检查 Input Tag 是否绑定
2. 检查技能是否已装备
3. 检查是否有阻止标签（眩晕、沉默等）
4. 检查法力是否充足
5. 检查技能是否在冷却中

**技能没有伤害**:
1. 检查 `DamageEffectClass` 是否设置
2. 检查 `Damage` 值是否配置
3. 检查 `DamageType` 是否正确
4. 检查 `SetByCaller` 标签是否匹配
5. 检查目标是否有 AbilitySystemComponent

**投射物不显示**:
1. 检查 `ProjectileClass` 是否设置
2. 检查投射物蓝图是否正确
3. 检查碰撞设置
4. 检查视觉效果资源

**UI 不显示技能**:
1. 检查 AbilityInfo 是否配置
2. 检查 Ability Tag 是否匹配
3. 检查技能是否已解锁
4. 检查 UI Widget 是否正确绑定

---

## 技能类型选择

### 投射物技能 (UAuraProjectileSpell)

**适用场景**: 发射投射物的技能（火球、冰箭等）

**需要配置**:
- `ProjectileClass`: 投射物类
- `NumProjectiles`: 投射物数量
- `ProjectileSpread`: 散布角度

**示例**: FireBolt, IceArrow

### 光束技能 (UAuraBeamSpell)

**适用场景**: 持续光束技能（闪电链等）

**需要配置**:
- `MaxNumShockTargets`: 最大连锁目标数

**示例**: Electrocute

### 召唤技能 (UAuraSummonAbility)

**适用场景**: 召唤生物的技能

**需要配置**:
- `NumMinions`: 召唤数量
- `MinionClasses`: 召唤物类数组
- `MinSpawnDistance`: 最小生成距离
- `MaxSpawnDistance`: 最大生成距离

**示例**: SummonAbility

### 被动技能 (UAuraPassiveAbility)

**适用场景**: 被动效果技能

**特点**:
- 自动激活
- 持续生效
- 不需要输入

**示例**: HaloOfProtection, LifeSiphon

---

## 完整示例

### 示例：创建冰霜箭技能（Ice Arrow）

这是一个完整的示例，展示如何创建一个投射物技能。

#### 1. 创建 C++ 类

**AuraIceArrow.h**:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraIceArrow.generated.h"

/**
 * 冰霜箭技能
 * 发射冰霜箭投射物，造成冰霜伤害，有几率减速敌人
 */
UCLASS()
class AURA_API UAuraIceArrow : public UAuraProjectileSpell
{
    GENERATED_BODY()
    
public:
    UAuraIceArrow();
    
    // 重写描述方法
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    // 技能特定参数
    UPROPERTY(EditDefaultsOnly, Category = "IceArrow")
    float SlowPercentage = 0.3f;  // 减速百分比（30%）
    
    UPROPERTY(EditDefaultsOnly, Category = "IceArrow")
    float SlowDuration = 3.f;  // 减速持续时间（3秒）
};
```

**AuraIceArrow.cpp**:
```cpp
#include "AbilitySystem/Abilities/AuraIceArrow.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"

UAuraIceArrow::UAuraIceArrow()
{
    // 获取 GameplayTags 单例
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    
    // 设置技能标签
    AbilityTags.AddTag(Tags.Abilities_Ice_IceArrow);
    
    // 设置输入标签（鼠标右键）
    StartupInputTag = Tags.InputTag_RMB;
}

FString UAuraIceArrow::GetDescription(int32 Level)
{
    // 获取当前等级的伤害值
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    
    // 获取法力消耗（使用绝对值）
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    
    // 获取冷却时间
    const float Cooldown = GetCooldown(Level);
    
    // 格式化描述字符串
    return FString::Printf(
        TEXT(
            "<Title>ICE ARROW</>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Launches an ice arrow, "
            "dealing </>"
            "<Damage>%d</>"
            "<Default> ice damage with a chance to slow enemies.</>\n\n"
            "<Small>Slow Effect: </><Default>%.0f%% for %.1f seconds</>"
        ),
        Level,
        ManaCost,
        Cooldown,
        ScaledDamage,
        SlowPercentage * 100.f,
        SlowDuration
    );
}

FString UAuraIceArrow::GetNextLevelDescription(int32 Level)
{
    // 获取下一级和当前级的伤害值
    const int32 NextLevelDamage = Damage.GetValueAtLevel(Level + 1);
    const int32 CurrentLevelDamage = Damage.GetValueAtLevel(Level);
    const int32 DamageIncrease = NextLevelDamage - CurrentLevelDamage;
    
    // 获取下一级的法力消耗和冷却时间
    const float NextManaCost = FMath::Abs(GetManaCost(Level + 1));
    const float NextCooldown = GetCooldown(Level + 1);
    
    return FString::Printf(
        TEXT(
            "<Title>NEXT LEVEL: </>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Damage: </><Damage>%d</><Default> (+%d)</>"
        ),
        Level + 1,
        NextManaCost,
        NextCooldown,
        NextLevelDamage,
        DamageIncrease
    );
}
```

#### 2. 添加 GameplayTag

**AuraGameplayTags.h**:
```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags ...
    
    // 添加新技能的 Tag
    FGameplayTag Abilities_Ice_IceArrow;
    
    // 添加冷却 Tag
    FGameplayTag Cooldown_Ice_IceArrow;
    
    // 如果使用新的伤害类型，添加伤害和抗性 Tag
    FGameplayTag Damage_Ice;
    FGameplayTag Attributes_Resistance_Ice;
};
```

**AuraGameplayTags.cpp**:
```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有初始化 ...
    
    // 初始化技能 Tag
    GameplayTags.Abilities_Ice_IceArrow = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Abilities.Ice.IceArrow"),
            FString("Ice Arrow ability - launches an ice projectile")
        );
    
    // 初始化冷却 Tag
    GameplayTags.Cooldown_Ice_IceArrow = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Cooldown.Ice.IceArrow"),
            FString("Ice Arrow cooldown tag")
        );
    
    // 初始化伤害类型 Tag（如果是新伤害类型）
    GameplayTags.Damage_Ice = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Damage.Ice"),
            FString("Ice damage type")
        );
    
    // 初始化抗性 Tag
    GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Attributes.Resistance.Ice"),
            FString("Ice resistance attribute")
        );
    
    // 添加到伤害类型到抗性的映射
    GameplayTags.DamageTypesToResistances.Add(
        GameplayTags.Damage_Ice,
        GameplayTags.Attributes_Resistance_Ice
    );
}
```

#### 3. 创建技能蓝图

1. **创建蓝图类**
   - 右键点击 `AuraIceArrow` C++ 类
   - 选择 "Create Blueprint class based on AuraIceArrow"
   - 命名为 `BP_GA_IceArrow`
   - 保存到 `Content/Blueprints/AbilitySystem/Abilities/Ice/`

2. **配置技能属性**
   - **Ability Tags**: 添加 `Abilities.Ice.IceArrow`
   - **Startup Input Tag**: `InputTag.RMB`

3. **配置技能参数**
   - **Projectile Class**: `BP_Projectile_IceArrow`（需要创建）
   - **Num Projectiles**: 1（单发）
   - **Damage**: 配置曲线表（Level 1: 50, Level 2: 75, ...）
   - **Damage Type**: `Damage.Ice`
   - **Damage Effect Class**: `GE_IceArrow_Damage`
   - **Debuff Chance**: 25.0（25% 几率减速）
   - **Debuff Damage**: 5.0（每秒 5 点伤害）
   - **Debuff Duration**: 3.0（持续 3 秒）
   - **Debuff Frequency**: 1.0（每秒触发一次）

#### 4. 创建 GameplayEffect

**GE_IceArrow_Cost**:
- Duration Policy: `Instant`
- Modifier: `Mana`, `Subtract`, `SetByCaller(Data.ManaCost)`

**GE_IceArrow_Cooldown**:
- Duration Policy: `HasDuration`
- Duration Magnitude: 5.0（5秒冷却）
- Granted Tags: `Cooldown.Ice.IceArrow`
- Ignore Tags: `Cooldown.Ice.IceArrow`

**GE_IceArrow_Damage**:
- Duration Policy: `Instant`
- Execution Calculation: `ExecCalc_Damage`
- Modifier: `IncomingDamage`, `Additive`, `SetByCaller(Damage.Ice)`

#### 5. 创建投射物蓝图

1. **创建投射物蓝图**
   - 基于 `BP_Projectile` 创建
   - 命名为 `BP_Projectile_IceArrow`

2. **配置投射物**
   - **Initial Speed**: 550
   - **Max Speed**: 550
   - **Projectile Gravity Scale**: 0
   - **Impact Effect**: 冰霜碰撞特效
   - **Impact Sound**: 冰霜碰撞音效
   - **Life Span**: 15.0

#### 6. 配置 AbilityInfo

在 `DA_AbilityInfo` 中添加：
- **Ability Tag**: `Abilities.Ice.IceArrow`
- **Input Tag**: `InputTag.RMB`
- **Status Tag**: `Abilities.Status.Locked`
- **Cooldown Tag**: `Cooldown.Ice.IceArrow`
- **Ability Type**: `Abilities.Type.Offensive`
- **Icon**: IceArrow_Icon（技能图标）
- **Background Material**: Ice_Background（背景材质）
- **Level Requirement**: 5（5级解锁）
- **Ability**: `BP_GA_IceArrow`

#### 7. 测试

1. 编译项目
2. 运行游戏
3. 达到 5 级后解锁技能
4. 使用法术点解锁
5. 装备到鼠标右键
6. 测试技能激活、伤害、视觉效果

#### 8. 完整文件结构

```
Source/Aura/
├── Public/AbilitySystem/Abilities/
│   └── AuraIceArrow.h
├── Private/AbilitySystem/Abilities/
│   └── AuraIceArrow.cpp
└── Public/
    └── AuraGameplayTags.h (修改)
    └── Private/
        └── AuraGameplayTags.cpp (修改)

Content/
├── Blueprints/AbilitySystem/
│   ├── Abilities/Ice/
│   │   └── BP_GA_IceArrow.uasset
│   └── Effects/
│       ├── Cost/
│       │   └── GE_IceArrow_Cost.uasset
│       ├── Cooldown/
│       │   └── GE_IceArrow_Cooldown.uasset
│       └── Damage/
│           └── GE_IceArrow_Damage.uasset
├── Blueprints/Actor/
│   └── Projectiles/
│       └── BP_Projectile_IceArrow.uasset
└── Data/
    └── DA_AbilityInfo.uasset (修改)
```

---

## 常见问题

### Q1: 技能无法激活

**症状**: 按下输入键后技能没有反应

**可能原因**:
1. **GameplayTag 未正确设置**
   - 技能 Tag 与 AbilityInfo 不匹配
   - Input Tag 未绑定

2. **法力不足**
   - 当前法力值小于技能消耗

3. **技能在冷却中**
   - 技能刚使用过，还在冷却

4. **输入标签未绑定**
   - InputConfig 中未配置输入映射

5. **技能未装备**
   - 技能状态为 `Unlocked` 但未装备到槽位

6. **阻止标签存在**
   - 角色有眩晕、沉默等状态标签

**解决方法**:
1. 检查技能 Tag 是否正确设置
2. 检查 Input Tag 是否与 AbilityInfo 匹配
3. 检查 InputConfig 数据资产中的输入映射
4. 检查技能状态（是否已装备）
5. 检查角色是否有阻止标签
6. 检查法力值是否充足
7. 检查冷却时间是否已结束

**调试方法**:
```cpp
// 在技能激活时添加日志
UE_LOG(LogTemp, Warning, TEXT("Attempting to activate ability: %s"), 
    *AbilityTags.ToString());
UE_LOG(LogTemp, Warning, TEXT("Current Mana: %.2f, Required: %.2f"), 
    CurrentMana, GetManaCost(GetAbilityLevel()));
```

### Q2: 技能没有伤害

**症状**: 技能激活了，但没有对目标造成伤害

**可能原因**:
1. **伤害 GameplayEffect 未正确配置**
   - `DamageEffectClass` 未设置
   - Execution Calculation 未设置

2. **SetByCaller 标签不匹配**
   - 伤害效果中的 SetByCaller 标签与技能中的 DamageType 不匹配
   - 例如：效果中使用 `Damage.Fire`，但技能使用 `Damage.Ice`

3. **伤害值未设置**
   - `Damage` 曲线表未配置
   - 伤害值为 0

4. **目标没有 AbilitySystemComponent**
   - 目标 Actor 没有 ASC，无法应用伤害

5. **伤害被完全减免**
   - 目标抗性过高，伤害被完全减免

**解决方法**:
1. 检查 `DamageEffectClass` 是否在技能蓝图中设置
2. 检查伤害效果中的 SetByCaller 标签
   - 必须与技能中的 `DamageType` 完全匹配
   - 例如：如果 `DamageType = Damage.Ice`，则 SetByCaller 必须是 `Damage.Ice`
3. 检查 `Damage` 曲线表配置
   - 确保每个等级都有伤害值
   - 检查值是否合理（不为 0）
4. 检查目标是否有 AbilitySystemComponent
5. 检查目标的抗性值
6. 在 `CauseDamage` 或投射物碰撞时添加日志

**调试方法**:
```cpp
// 在伤害应用时添加日志
UE_LOG(LogTemp, Warning, TEXT("Applying damage: %.2f of type %s"), 
    ScaledDamage, *DamageType.ToString());
```

### Q3: 技能描述不显示

**症状**: 在技能菜单中看不到技能描述

**可能原因**:
1. **`GetDescription` 方法未实现**
   - 方法返回空字符串
   - 方法未正确重写

2. **AbilityInfo 中未配置**
   - 技能未添加到 AbilityInfo
   - Ability Tag 不匹配

3. **技能未解锁**
   - 技能状态为 `Locked`，显示锁定描述

4. **UI Widget 未正确绑定**
   - Widget Controller 未正确设置
   - 描述委托未绑定

**解决方法**:
1. 检查 `GetDescription` 方法实现
   - 确保返回有效的字符串
   - 检查格式化是否正确
2. 检查 AbilityInfo 配置
   - 确保技能已添加
   - 检查 Ability Tag 是否匹配
3. 检查技能状态
   - 如果锁定，会显示 `GetLockedDescription`
4. 检查 UI 绑定
   - 检查 Widget Controller 是否正确初始化
   - 检查描述委托是否绑定

### Q4: 技能无法升级

**症状**: 无法使用法术点升级技能

**可能原因**:
1. **技能状态未正确更新**
   - 技能状态不是 `Eligible` 或 `Unlocked`
   - 状态管理逻辑错误

2. **法术点不足**
   - 当前法术点小于升级所需

3. **等级要求未满足**
   - 玩家等级未达到 `LevelRequirement`

4. **技能已达到最大等级**
   - 技能等级已达到上限

5. **服务器 RPC 失败**
   - `ServerSpendSpellPoint` 调用失败

**解决方法**:
1. 检查技能状态
   - 使用法术点前，技能状态应为 `Eligible` 或 `Unlocked`
2. 检查法术点
   - 确保有足够的法术点
3. 检查等级要求
   - 确保玩家等级达到要求
4. 检查技能等级上限
   - 某些技能可能有等级上限
5. 检查网络连接
   - 确保服务器 RPC 正常

### Q5: 投射物不显示或不移动

**症状**: 技能激活了，但看不到投射物

**可能原因**:
1. **ProjectileClass 未设置**
   - 技能蓝图中未选择投射物类

2. **投射物生成位置错误**
   - Socket 位置不正确
   - 生成位置在角色内部

3. **投射物移动组件未配置**
   - Initial Speed 为 0
   - Projectile Gravity Scale 设置错误

4. **投射物立即被销毁**
   - Life Span 设置过短
   - 碰撞检测错误

5. **网络复制问题**
   - 投射物未正确复制到客户端

**解决方法**:
1. 检查 `ProjectileClass` 是否设置
2. 检查 Socket Tag 是否正确
3. 检查投射物移动参数
4. 检查投射物生命周期
5. 检查网络复制设置
6. 使用 "Draw Debug Sphere" 可视化生成位置

### Q6: Debuff 不触发

**症状**: 技能造成伤害，但 Debuff 效果不应用

**可能原因**:
1. **DebuffChance 设置过低**
   - 几率太小，难以触发

2. **目标抗性过高**
   - 目标抗性降低了有效 Debuff 几率

3. **Debuff 参数未设置**
   - DebuffDamage、DebuffDuration 等未配置

4. **Debuff GameplayEffect 未创建**
   - 在 `ExecCalc_Damage` 中未成功创建 Debuff

**解决方法**:
1. 检查 `DebuffChance` 值（建议 20-50%）
2. 检查目标抗性值
3. 检查所有 Debuff 参数
4. 在 `ExecCalc_Damage` 中添加日志
5. 检查 Debuff 创建逻辑

### Q7: 技能在多人游戏中不同步

**症状**: 服务器和客户端表现不一致

**可能原因**:
1. **服务器权威问题**
   - 某些逻辑在客户端执行

2. **网络复制未配置**
   - 投射物、效果等未正确复制

3. **RPC 调用失败**
   - 服务器 RPC 未正确调用

**解决方法**:
1. 确保伤害计算在服务器执行
2. 确保投射物生成在服务器执行
3. 检查网络复制设置
4. 使用 `HasAuthority()` 检查执行权限
5. 检查 RPC 函数声明（`Server_`, `Client_`, `Multicast_`）

### Q8: 技能描述格式错误

**症状**: 技能描述显示格式标签而不是格式化文本

**可能原因**:
1. **格式化标签未正确使用**
   - 标签拼写错误
   - 标签未正确闭合

2. **UI Widget 未处理格式化**
   - Widget 未实现格式化逻辑

**解决方法**:
1. 检查标签拼写
2. 确保所有标签正确闭合
3. 检查 UI Widget 的文本处理
4. 参考现有技能的描述格式

---

## 总结

添加新技能是一个系统化的过程，需要多个步骤的协调配合。

### 核心步骤回顾

1. ✅ **确定技能类型** - 选择合适的基类
2. ✅ **创建 C++ 类** - 实现技能逻辑和描述
3. ✅ **添加 GameplayTag** - 在 AuraGameplayTags 中注册
4. ✅ **创建技能蓝图** - 基于 C++ 类创建蓝图
5. ✅ **创建 GameplayEffect** - Cost、Cooldown 和 Damage
6. ✅ **创建投射物（如需要）** - 投射物技能需要投射物类
7. ✅ **配置 AbilityInfo** - 在数据资产中添加技能信息
8. ✅ **配置技能参数** - 在蓝图中设置所有参数
9. ✅ **添加到 StartupAbilities（可选）** - 如果需要初始获得
10. ✅ **测试技能** - 全面测试所有功能

### 关键检查点

在完成每个步骤后，检查：

- ✅ **C++ 代码编译通过**
- ✅ **GameplayTag 正确注册**
- ✅ **蓝图可以正常打开**
- ✅ **GameplayEffect 配置正确**
- ✅ **AbilityInfo 信息完整**
- ✅ **技能参数合理**
- ✅ **测试通过**

### 最佳实践

1. **遵循命名规范**
   - C++ 类：`UAura[SkillName]`
   - 蓝图：`BP_GA_[SkillName]`
   - GameplayEffect：`GE_[SkillName]_[Type]`
   - GameplayTag：`Abilities.[Category].[SkillName]`

2. **使用现有技能作为参考**
   - 参考 FireBolt、Electrocute 等现有技能
   - 复制并修改现有代码

3. **逐步测试**
   - 每完成一个步骤就测试
   - 不要等到最后才测试

4. **文档化**
   - 为技能添加注释
   - 记录特殊参数和配置

5. **版本控制**
   - 提交前确保代码编译通过
   - 使用清晰的提交信息

### 扩展阅读

- [技能系统详细文档](../Core/Ability_System.md) - 深入了解技能系统
- [伤害计算系统](./Systems/Damage_Calculation.md) - 了解伤害计算机制
- [Debuff 系统](./Systems/Debuff_System.md) - 了解 Debuff 实现
- [各技能详细文档](./Abilities/README.md) - 参考现有技能实现

### 获取帮助

如果遇到问题：

1. 检查本文档的常见问题部分
2. 参考现有技能的实现
3. 查看项目日志和错误信息
4. 使用调试工具（断点、日志等）

遵循这些步骤和最佳实践，可以成功添加新技能到项目中。记住，添加技能是一个迭代过程，可能需要多次调整才能达到理想效果。

