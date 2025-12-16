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

- **UAuraGameplayAbility**: 基础技能类（通用）
- **UAuraDamageGameplayAbility**: 伤害技能基类
  - **UAuraProjectileSpell**: 投射物技能
  - **UAuraBeamSpell**: 光束技能
- **UAuraMeleeAttack**: 近战攻击
- **UAuraSummonAbility**: 召唤技能
- **UAuraPassiveAbility**: 被动技能

#### 示例

- **火球术**: `UAuraProjectileSpell`
- **闪电链**: `UAuraBeamSpell`
- **召唤生物**: `UAuraSummonAbility`
- **被动护盾**: `UAuraPassiveAbility`

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
}

FString UAuraIceArrow::GetDescription(int32 Level)
{
    const float ScaledDamage = Damage.GetValueAtLevel(Level);
    const float ManaCost = GetManaCost(Level);
    const float Cooldown = GetCooldown(Level);
    
    return FString::Printf(
        TEXT("<Title>冰霜箭</>\n\n")
        TEXT("<Default>发射一支冰霜箭，造成 %.0f 点冰霜伤害。</>\n\n")
        TEXT("<Small>法力消耗: %.0f</>\n")
        TEXT("<Small>冷却时间: %.1f 秒</>\n")
        TEXT("<Small>减速效果: %.0f%% 持续 %.1f 秒</>"),
        ScaledDamage, ManaCost, Cooldown, SlowPercentage * 100.f, SlowDuration
    );
}

FString UAuraIceArrow::GetNextLevelDescription(int32 Level)
{
    const float NextLevelDamage = Damage.GetValueAtLevel(Level + 1);
    const float CurrentLevelDamage = Damage.GetValueAtLevel(Level);
    const float DamageIncrease = NextLevelDamage - CurrentLevelDamage;
    
    return FString::Printf(
        TEXT("下一级伤害: %.0f (+%.0f)"),
        NextLevelDamage, DamageIncrease
    );
}
```

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

#### 5.1 创建法力消耗 GameplayEffect

1. 创建新的 GameplayEffect，命名为 `GE_IceArrow_Cost`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Modifiers**: 
     - **Attribute**: `Mana`
     - **Modifier Op**: `Subtract`
     - **Magnitude Calculation Type**: `SetByCaller`
     - **SetByCaller Magnitude**: `Data.ManaCost`

#### 5.2 创建冷却 GameplayEffect

1. 创建新的 GameplayEffect，命名为 `GE_IceArrow_Cooldown`
2. 设置：
   - **Duration Policy**: `HasDuration`
   - **Duration Magnitude**: 冷却时间（例如 5 秒）
   - **Period**: 0（不周期性触发）
   - **Granted Tags**: `Cooldown.Ice.IceArrow`
   - **Ongoing Tag Requirements**: 
     - **Require Tags**: 无
     - **Ignore Tags**: `Cooldown.Ice.IceArrow`

#### 5.3 在技能蓝图中关联

在技能蓝图中设置：

- **Ability Cost**: `GE_IceArrow_Cost`
- **Cooldown**: `GE_IceArrow_Cooldown`

---

### 步骤 6: 在 AbilityInfo 数据资产中添加技能信息

#### 6.1 打开 AbilityInfo 数据资产

在编辑器中打开 `DA_AbilityInfo` 数据资产。

#### 6.2 添加技能信息条目

在 `AbilityInformation` 数组中添加新条目：

- **Ability Tag**: `Abilities.Ice.IceArrow`
- **Input Tag**: `InputTag.RMB`（根据技能输入设置）
- **Status Tag**: `Abilities.Status.Locked`（初始状态）
- **Cooldown Tag**: `Cooldown.Ice.IceArrow`
- **Ability Type**: `Abilities.Type.Offensive`（或 Passive）
- **Icon**: 技能图标纹理
- **Background Material**: 背景材质
- **Level Requirement**: 解锁等级（例如 5）
- **Ability**: `BP_GA_IceArrow`（技能蓝图类）

---

### 步骤 7: 配置技能参数

#### 7.1 在技能蓝图中配置

根据技能类型配置参数：

**投射物技能**:
- **ProjectileClass**: 投射物类
- **NumProjectiles**: 投射物数量
- **ProjectileSpread**: 投射物散布角度

**伤害技能**:
- **Damage**: 伤害值（Scalable Float）
- **DamageType**: 伤害类型（例如 `Damage.Ice`）
- **DebuffChance**: Debuff 触发几率
- **DebuffDamage**: Debuff 伤害
- **DebuffDuration**: Debuff 持续时间
- **DebuffFrequency**: Debuff 触发频率

#### 7.2 配置伤害效果

如果技能造成伤害，需要创建伤害 GameplayEffect：

1. 创建 `GE_IceArrow_Damage`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Execution Calculation**: `ExecCalc_Damage`
   - **Modifiers**:
     - **Attribute**: `IncomingDamage`
     - **Modifier Op**: `Additive`
     - **Magnitude Calculation Type**: `SetByCaller`
     - **SetByCaller Magnitude**: `Damage.Ice`

---

### 步骤 8: 添加到角色的 StartupAbilities（可选）

如果希望技能在角色创建时自动获得：

#### 8.1 在角色蓝图中添加

1. 打开角色蓝图（例如 `BP_AuraCharacter`）
2. 在 `Startup Abilities` 数组中添加 `BP_GA_IceArrow`

#### 8.2 在 CharacterClassInfo 中配置

1. 打开 `DA_CharacterClassInfo` 数据资产
2. 在对应职业的 `CommonAbilities` 或 `StartupAbilities` 中添加技能

---

### 步骤 9: 测试技能

#### 9.1 编译和运行

1. 编译项目
2. 运行游戏
3. 测试技能激活

#### 9.2 测试要点

- ✅ 技能可以激活
- ✅ 法力消耗正确
- ✅ 冷却时间正确
- ✅ 伤害计算正确
- ✅ 视觉效果正确
- ✅ 音效正确
- ✅ UI 显示正确

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

### 示例：创建冰霜箭技能

#### 1. 创建 C++ 类

**AuraIceArrow.h**:
```cpp
#pragma once
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraIceArrow.generated.h"

UCLASS()
class AURA_API UAuraIceArrow : public UAuraProjectileSpell
{
    GENERATED_BODY()
    
public:
    UAuraIceArrow();
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
};
```

**AuraIceArrow.cpp**:
```cpp
#include "AbilitySystem/Abilities/AuraIceArrow.h"
#include "AuraGameplayTags.h"

UAuraIceArrow::UAuraIceArrow()
{
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    AbilityTags.AddTag(Tags.Abilities_Ice_IceArrow);
    StartupInputTag = Tags.InputTag_RMB;
}

FString UAuraIceArrow::GetDescription(int32 Level)
{
    const float ScaledDamage = Damage.GetValueAtLevel(Level);
    return FString::Printf(
        TEXT("发射冰霜箭，造成 %.0f 点冰霜伤害。"),
        ScaledDamage
    );
}

FString UAuraIceArrow::GetNextLevelDescription(int32 Level)
{
    const float NextDamage = Damage.GetValueAtLevel(Level + 1);
    return FString::Printf(TEXT("下一级伤害: %.0f"), NextDamage);
}
```

#### 2. 添加 GameplayTag

**AuraGameplayTags.h**:
```cpp
FGameplayTag Abilities_Ice_IceArrow;
FGameplayTag Cooldown_Ice_IceArrow;
```

**AuraGameplayTags.cpp**:
```cpp
GameplayTags.Abilities_Ice_IceArrow = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Abilities.Ice.IceArrow"));
GameplayTags.Cooldown_Ice_IceArrow = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Cooldown.Ice.IceArrow"));
```

#### 3. 创建蓝图和 GameplayEffect

- 创建 `BP_GA_IceArrow` 蓝图
- 创建 `GE_IceArrow_Cost` 和 `GE_IceArrow_Cooldown`
- 创建 `GE_IceArrow_Damage`

#### 4. 配置 AbilityInfo

在 `DA_AbilityInfo` 中添加条目，设置所有必要信息。

#### 5. 测试

编译并测试技能。

---

## 常见问题

### Q1: 技能无法激活

**可能原因**:
- GameplayTag 未正确设置
- 法力不足
- 技能在冷却中
- 输入标签未绑定

**解决方法**:
- 检查技能 Tag 是否正确
- 检查法力消耗设置
- 检查冷却时间设置
- 检查输入配置

### Q2: 技能没有伤害

**可能原因**:
- 伤害 GameplayEffect 未正确配置
- SetByCaller 标签不匹配
- 伤害值未设置

**解决方法**:
- 检查伤害效果配置
- 确保 SetByCaller 标签为 `Damage.Ice`（或对应伤害类型）
- 检查技能中的伤害值设置

### Q3: 技能描述不显示

**可能原因**:
- `GetDescription` 方法未实现
- AbilityInfo 中未配置

**解决方法**:
- 实现 `GetDescription` 方法
- 检查 AbilityInfo 配置

### Q4: 技能无法升级

**可能原因**:
- 技能状态未正确更新
- 法术点不足
- 等级要求未满足

**解决方法**:
- 检查技能状态管理
- 检查法术点系统
- 检查等级要求设置

---

## 总结

添加新技能需要以下步骤：

1. ✅ **创建 C++ 类** - 选择合适的基类
2. ✅ **添加 GameplayTag** - 在 AuraGameplayTags 中添加
3. ✅ **创建蓝图** - 基于 C++ 类创建蓝图
4. ✅ **创建 GameplayEffect** - Cost 和 Cooldown
5. ✅ **配置 AbilityInfo** - 在数据资产中添加信息
6. ✅ **配置参数** - 在蓝图中设置技能参数
7. ✅ **测试** - 编译并测试技能

遵循这些步骤，可以成功添加新技能到项目中。

