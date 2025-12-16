# 如何添加新角色

## 概述

本文档详细说明如何在 Aura 项目中添加新的角色类型，包括玩家角色和敌人角色。新角色可以继承现有的角色基类，并配置独特的属性、能力和行为。

---

## 1. 角色类型选择

### 1.1 角色类型

项目中有两种主要角色类型：

#### 玩家角色 (`AAuraCharacter`)
- 继承自 `AAuraCharacterBase`
- 实现 `IPlayerInterface`
- 使用 PlayerState 的 ASC
- 支持存档系统
- 有相机系统

#### 敌人角色 (`AAuraEnemy`)
- 继承自 `AAuraCharacterBase`
- 实现 `IEnemyInterface` 和 `IHighlightInterface`
- 拥有自己的 ASC
- 使用 AI 控制器
- 有行为树

### 1.2 选择指南

**选择玩家角色，如果**:
- 需要玩家控制
- 需要存档支持
- 需要经验值和升级系统

**选择敌人角色，如果**:
- 需要 AI 控制
- 需要自动战斗
- 需要高亮系统

---

## 2. 添加新玩家角色

### 2.1 创建 C++ 类

#### 步骤 1: 创建类文件

在 `Source/Aura/Public/Character/` 和 `Source/Aura/Private/Character/` 中创建新文件：

**MyNewCharacter.h**:
```cpp
// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacter.h"
#include "MyNewCharacter.generated.h"

/**
 * 新的玩家角色类
 */
UCLASS()
class AURA_API AMyNewCharacter : public AAuraCharacter
{
    GENERATED_BODY()
    
public:
    AMyNewCharacter();
    
protected:
    virtual void BeginPlay() override;
    
    // 可以添加角色特定的初始化
    virtual void InitAbilityActorInfo() override;
};
```

**MyNewCharacter.cpp**:
```cpp
// Copyright Druid Mechanics

#include "Character/MyNewCharacter.h"

AMyNewCharacter::AMyNewCharacter()
{
    // 设置默认职业
    CharacterClass = ECharacterClass::Warrior;  // 或其他职业
    
    // 设置默认移动速度
    BaseWalkSpeed = 600.f;
}

void AMyNewCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 角色特定的初始化
}

void AMyNewCharacter::InitAbilityActorInfo()
{
    Super::InitAbilityActorInfo();
    
    // 可以添加额外的初始化逻辑
}
```

#### 步骤 2: 编译项目

1. 关闭 Unreal Editor
2. 右键 `.uproject` 文件
3. 选择 "Generate Visual Studio project files"
4. 打开 Visual Studio
5. 编译项目

---

### 2.2 创建蓝图

#### 步骤 1: 创建角色蓝图

1. **打开 Content Browser**
2. **右键点击** → `Blueprint Class`
3. **选择父类**: `MyNewCharacter`
4. **命名**: `BP_MyNewCharacter`

#### 步骤 2: 配置角色蓝图

1. **设置网格**
   - 在 `Mesh` 组件中设置 Skeletal Mesh
   - 设置动画蓝图

2. **设置武器**
   - 在 `Weapon` 组件中设置武器网格
   - 确保 Socket 名称匹配

3. **设置相机**（如果需要）
   - 调整 `CameraBoom` 和 `TopDownCameraComponent`
   - 设置相机位置和角度

4. **设置职业**
   - `Character Class`: 选择职业类型

---

### 2.3 配置职业信息

#### 步骤 1: 在 CharacterClassInfo 中添加职业

如果添加新职业类型：

1. **修改 ECharacterClass 枚举**

```cpp
// 在 CharacterClassInfo.h 中
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
    Elementalist,
    Warrior,
    Ranger,
    MyNewClass  // 新职业
};
```

2. **在 CharacterClassInfo 数据资产中配置**

打开 `DA_CharacterClassInfo`:
- 在 `CharacterClassInformation` 中添加新职业条目
- 设置 `PrimaryAttributes` GameplayEffect
- 设置 `StartupAbilities` 数组
- 配置 `XPReward` 曲线

#### 步骤 2: 创建主属性 GameplayEffect

1. **创建 GameplayEffect**
   - 继承自 `GE_PrimaryAttributes_SetByCaller`
   - 设置新职业的属性值

2. **配置属性值**
   - Strength
   - Intelligence
   - Resilience
   - Vigor

---

### 2.4 配置初始能力

#### 步骤 1: 在角色蓝图中设置

在 `BP_MyNewCharacter` 中：
- `Startup Abilities`: 添加初始能力列表
- `Startup Passive Abilities`: 添加被动能力列表

#### 步骤 2: 在 CharacterClassInfo 中设置

在 `DA_CharacterClassInfo` 中：
- 为新职业设置 `StartupAbilities`
- 或在 `CommonAbilities` 中添加通用能力

---

## 3. 添加新敌人角色

### 3.1 创建 C++ 类

#### 步骤 1: 创建类文件

**MyNewEnemy.h**:
```cpp
// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraEnemy.h"
#include "MyNewEnemy.generated.h"

/**
 * 新的敌人角色类
 */
UCLASS()
class AURA_API AMyNewEnemy : public AAuraEnemy
{
    GENERATED_BODY()
    
public:
    AMyNewEnemy();
    
protected:
    virtual void BeginPlay() override;
    virtual void InitAbilityActorInfo() override;
    
    // 敌人特定的属性
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
    float SpecialAbilityCooldown = 5.f;
};
```

**MyNewEnemy.cpp**:
```cpp
// Copyright Druid Mechanics

#include "Character/MyNewEnemy.h"

AMyNewEnemy::AMyNewEnemy()
{
    // 设置默认职业
    CharacterClass = ECharacterClass::Warrior;
    
    // 设置敌人等级
    Level = 1;
    
    // 设置生命周期（死亡后销毁时间）
    LifeSpan = 5.f;
}

void AMyNewEnemy::BeginPlay()
{
    Super::BeginPlay();
    
    // 敌人特定的初始化
}

void AMyNewEnemy::InitAbilityActorInfo()
{
    Super::InitAbilityActorInfo();
    
    // 可以添加额外的初始化逻辑
}
```

---

### 3.2 创建蓝图

#### 步骤 1: 创建敌人蓝图

1. **创建蓝图**: `BP_MyNewEnemy`
2. **父类**: `MyNewEnemy`

#### 步骤 2: 配置敌人蓝图

1. **设置网格和动画**
   - Skeletal Mesh
   - Animation Blueprint
   - 动画蒙太奇

2. **设置 AI**
   - `Behavior Tree`: 指定行为树
   - `AIController Class`: 使用 `AuraAIController`

3. **设置生命值条**
   - `Health Bar`: 配置 Widget Component

4. **设置职业和等级**
   - `Character Class`: 选择职业
   - `Level`: 设置敌人等级

---

### 3.3 配置 AI 行为

#### 步骤 1: 创建或使用现有行为树

1. **创建行为树**（如果需要）
   - 继承现有行为树
   - 或创建新的行为树

2. **配置行为树**
   - 设置查找玩家服务
   - 设置攻击任务
   - 配置移动和战斗逻辑

#### 步骤 2: 在敌人蓝图中设置

- `Behavior Tree`: 指定行为树资源

---

## 4. 配置角色属性

### 4.1 设置初始属性

#### 方法 1: 通过 CharacterClassInfo

在 `DA_CharacterClassInfo` 中为新职业配置：
- 主属性值
- 初始能力
- 经验值奖励

#### 方法 2: 在角色蓝图中设置

在角色蓝图中直接设置：
- `Default Primary Attributes`
- `Default Secondary Attributes`
- `Default Vital Attributes`

---

### 4.2 设置动画

#### 步骤 1: 配置动画蒙太奇

1. **创建动画蒙太奇**
   - 攻击动画
   - 受击动画
   - 死亡动画

2. **在角色蓝图中设置**
   - `Hit React Montage`: 受击动画
   - `Attack Montages`: 攻击动画数组

#### 步骤 2: 配置动画蓝图

1. **创建或使用现有动画蓝图**
2. **设置状态机**
3. **在角色蓝图中指定**

---

### 4.3 设置视觉效果

#### 步骤 1: 配置粒子效果

1. **死亡效果**
   - `Death Sound`: 死亡音效
   - `Blood Effect`: 血液粒子效果

2. **溶解效果**
   - `Dissolve Material Instance`: 溶解材质
   - `Weapon Dissolve Material Instance`: 武器溶解材质

#### 步骤 2: 配置 Debuff 效果

角色基类已包含：
- `BurnDebuffComponent`
- `StunDebuffComponent`

在蓝图中可以调整这些组件的设置。

---

## 5. 配置战斗系统

### 5.1 设置武器

#### 步骤 1: 配置武器网格

1. **创建或导入武器模型**
2. **在角色蓝图中设置**
   - `Weapon`: 设置武器 Skeletal Mesh
   - 确保 Socket 名称匹配

#### 步骤 2: 配置 Socket

在角色网格中确保有以下 Socket：
- `WeaponHandSocket`: 武器挂载点
- `TipSocket`: 武器尖端（用于投射物生成）
- `LeftHandSocket`: 左手（用于技能）
- `RightHandSocket`: 右手（用于技能）
- `TailSocket`: 尾部（用于技能）

---

### 5.2 设置攻击动画

#### 步骤 1: 创建 Tagged Montage

在角色蓝图中配置 `Attack Montages`:

```
Attack Montages:
  [0]:
    Montage: AM_Attack_1
    MontageTag: Montage.Attack.1
  [1]:
    Montage: AM_Attack_2
    MontageTag: Montage.Attack.2
  ...
```

#### 步骤 2: 注册 Gameplay Tags

确保所有使用的 Gameplay Tags 已在 `AuraGameplayTags` 中注册。

---

## 6. 配置能力系统

### 6.1 设置初始能力

#### 步骤 1: 在角色蓝图中

在 `BP_MyNewCharacter` 或 `BP_MyNewEnemy` 中：
- `Startup Abilities`: 添加能力类
- `Startup Passive Abilities`: 添加被动能力

#### 步骤 2: 在 CharacterClassInfo 中

在 `DA_CharacterClassInfo` 中：
- 为新职业设置 `StartupAbilities`
- 或在 `CommonAbilities` 中添加

---

### 6.2 配置能力输入

#### 步骤 1: 创建输入配置

如果角色有独特的输入需求：

1. **创建 InputConfig 数据资产**
2. **配置输入映射**
3. **在角色或控制器中设置**

---

## 7. 完整示例：添加新职业角色

### 7.1 示例：添加 Necromancer（死灵法师）职业

#### 步骤 1: 添加职业枚举

```cpp
// 在 CharacterClassInfo.h 中
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
    Elementalist,
    Warrior,
    Ranger,
    Necromancer  // 新职业
};
```

#### 步骤 2: 创建角色类

```cpp
// NecromancerCharacter.h
UCLASS()
class AURA_API ANecromancerCharacter : public AAuraCharacter
{
    GENERATED_BODY()
    
public:
    ANecromancerCharacter()
    {
        CharacterClass = ECharacterClass::Necromancer;
    }
};
```

#### 步骤 3: 配置 CharacterClassInfo

在 `DA_CharacterClassInfo` 中：

```
CharacterClassInformation:
  Necromancer:
    PrimaryAttributes: GE_PrimaryAttributes_Necromancer
    StartupAbilities:
      - BP_GA_SummonSkeleton
      - BP_GA_DarkBolt
    XPReward:
      Level 1: 12
      Level 2: 18
      ...
```

#### 步骤 4: 创建主属性 GameplayEffect

1. **创建**: `GE_PrimaryAttributes_Necromancer`
2. **配置属性**:
   - Strength: 较低
   - Intelligence: 很高
   - Resilience: 中等
   - Vigor: 中等

#### 步骤 5: 创建角色蓝图

1. **创建**: `BP_NecromancerCharacter`
2. **设置网格**: 死灵法师模型
3. **设置职业**: Necromancer
4. **设置能力**: 召唤和暗影能力

---

## 8. 测试新角色

### 8.1 测试检查清单

#### 基础功能测试

- [ ] 角色可以正常生成
- [ ] 网格和动画正常显示
- [ ] 移动正常
- [ ] 属性正确初始化
- [ ] 能力可以激活

#### GAS 测试

- [ ] ASC 正确初始化
- [ ] 属性正确应用
- [ ] 能力正确给予
- [ ] 能力可以激活
- [ ] 冷却和成本正常

#### 战斗测试

- [ ] 攻击动画正常
- [ ] 受击反应正常
- [ ] 死亡处理正常
- [ ] 伤害计算正确
- [ ] Debuff 正常应用

#### 网络测试（多人游戏）

- [ ] 角色正确复制
- [ ] 状态同步正常
- [ ] 能力同步正常
- [ ] 动画同步正常

---

### 8.2 调试技巧

#### 查看角色状态

```cpp
// 在控制台中
ShowDebug AbilitySystem
```

#### 查看属性值

```cpp
// 在代码中
if (UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet))
{
    UE_LOG(LogAura, Log, TEXT("Health: %f"), AS->GetHealth());
    UE_LOG(LogAura, Log, TEXT("Mana: %f"), AS->GetMana());
}
```

#### 查看能力列表

```cpp
// 在代码中
if (UAuraAbilitySystemComponent* ASC = 
    Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->ForEachAbility([](const FGameplayAbilitySpec& Spec)
    {
        UE_LOG(LogAura, Log, TEXT("Ability: %s"), 
            *Spec.Ability->GetName());
        return true;
    });
}
```

---

## 9. 常见问题

### Q1: 角色无法生成？

**A**: 检查：
1. 蓝图是否正确编译
2. 父类是否正确
3. 网格是否设置
4. GameMode 中是否设置了正确的 Pawn 类

### Q2: 属性未初始化？

**A**: 检查：
1. `InitAbilityActorInfo()` 是否被调用
2. `InitializeDefaultAttributes()` 是否被调用
3. CharacterClassInfo 是否正确配置
4. GameplayEffect 是否正确设置

### Q3: 能力无法激活？

**A**: 检查：
1. 能力是否被正确给予
2. 输入绑定是否正确
3. 冷却和成本是否满足
4. 能力标签是否正确

### Q4: 动画不播放？

**A**: 检查：
1. 动画蒙太奇是否正确设置
2. 动画蓝图是否正确配置
3. Montage Tag 是否正确
4. 网格和动画是否兼容

### Q5: AI 敌人不移动？

**A**: 检查：
1. 行为树是否正确设置
2. AI Controller 是否正确分配
3. 导航网格是否存在
4. 行为树是否运行

---

## 10. 高级配置

### 10.1 自定义初始化

```cpp
// 在角色类中重写初始化
void AMyNewCharacter::InitAbilityActorInfo()
{
    Super::InitAbilityActorInfo();
    
    // 自定义初始化逻辑
    // 例如：设置特殊属性、给予特殊能力等
}
```

### 10.2 自定义死亡处理

```cpp
// 重写死亡方法
void AMyNewCharacter::Die(const FVector& DeathImpulse)
{
    // 自定义死亡逻辑
    // 例如：特殊死亡效果、掉落物等
    
    Super::Die(DeathImpulse);
}
```

### 10.3 自定义受击反应

```cpp
// 重写受击方法
void AMyNewCharacter::HitReactTagChanged(
    const FGameplayTag CallbackTag, 
    int32 NewCount
)
{
    // 自定义受击逻辑
    // 例如：特殊受击动画、音效等
    
    Super::HitReactTagChanged(CallbackTag, NewCount);
}
```

---

## 11. 最佳实践

### 11.1 代码组织

- **使用继承**: 继承现有角色类而非从头创建
- **保持一致性**: 遵循现有代码风格
- **添加注释**: 说明特殊逻辑

### 11.2 配置管理

- **使用数据资产**: 尽可能使用 CharacterClassInfo
- **避免硬编码**: 使用可配置的值
- **版本控制**: 确保蓝图和配置都在版本控制中

### 11.3 性能考虑

- **优化网格**: 使用 LOD
- **优化动画**: 减少不必要的动画更新
- **优化能力**: 避免过于复杂的能力逻辑

---

## 12. 完整工作流示例

### 12.1 添加新玩家角色工作流

```
1. 创建 C++ 类
   ↓
2. 编译项目
   ↓
3. 创建蓝图
   ↓
4. 配置网格和动画
   ↓
5. 配置职业信息（如果需要新职业）
   ↓
6. 设置初始能力
   ↓
7. 配置属性
   ↓
8. 测试功能
   ↓
9. 在 GameMode 中设置默认 Pawn
```

### 12.2 添加新敌人角色工作流

```
1. 创建 C++ 类
   ↓
2. 编译项目
   ↓
3. 创建蓝图
   ↓
4. 配置网格和动画
   ↓
5. 设置 AI（行为树、控制器）
   ↓
6. 配置初始能力
   ↓
7. 设置等级和属性
   ↓
8. 配置生命值条
   ↓
9. 测试功能
   ↓
10. 在生成点中设置
```

---

## 13. 总结

添加新角色是一个系统化的过程：

1. ✅ **选择角色类型** - 玩家或敌人
2. ✅ **创建 C++ 类** - 继承现有基类
3. ✅ **创建蓝图** - 配置视觉和属性
4. ✅ **配置职业信息** - 设置属性和能力
5. ✅ **设置动画和效果** - 配置视觉表现
6. ✅ **测试验证** - 确保功能正常

### 关键要点

- **继承现有类**: 利用现有功能，减少重复代码
- **使用数据资产**: 通过 CharacterClassInfo 配置，易于调整
- **遵循现有模式**: 保持代码一致性
- **充分测试**: 确保所有功能正常

通过这个流程，可以高效地添加新的角色类型，同时保持代码的清晰和可维护性。

---

## 相关文档

- [角色系统](../Systems/Character_System.md) - 角色系统详细文档
- [玩家系统](../Systems/Player_System.md) - 玩家系统文档
- [AI 系统](../Systems/AI_System.md) - AI 系统文档（敌人角色）
- [数据资产系统](../Systems/Data_Assets_System.md) - CharacterClassInfo 配置
- [如何添加新技能](./How_To_Add_New_Ability.md) - 添加角色能力

