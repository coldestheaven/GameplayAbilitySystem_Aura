# GameplayTags 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [GameplayTags 基础](#gameplaytags-基础)
3. [项目中的 GameplayTags 实现](#项目中的-gameplaytags-实现)
4. [Tag 分类和结构](#tag-分类和结构)
5. [Tag 初始化和注册](#tag-初始化和注册)
6. [Tag 使用场景](#tag-使用场景)
7. [Tag 查询和匹配](#tag-查询和匹配)
8. [Tag 容器操作](#tag-容器操作)
9. [添加新 Tag](#添加新-tag)
10. [最佳实践](#最佳实践)
11. [常见问题](#常见问题)

---

## 系统概述

GameplayTags 是 Unreal Engine 的标签系统，用于标识和分类游戏对象、能力、效果等。在 Aura 项目中，GameplayTags 是 GAS 系统的核心，用于：

- **属性标识**: 标识不同类型的属性（主属性、次属性、抗性等）
- **能力标识**: 标识不同的技能和能力
- **输入绑定**: 将输入动作绑定到能力
- **伤害类型**: 标识不同的伤害类型（火焰、闪电、奥术、物理）
- **状态标识**: 标识能力状态（锁定、解锁、装备等）
- **效果标识**: 标识不同的游戏效果（受击反应、Debuff 等）

### 核心组件

- **FAuraGameplayTags**: 单例结构，包含所有 Native Gameplay Tags
- **UGameplayTagsManager**: UE5 的 Tag 管理器
- **FGameplayTag**: 单个 Tag 对象
- **FGameplayTagContainer**: Tag 容器，用于存储多个 Tag

---

## GameplayTags 基础

### 什么是 GameplayTag

GameplayTag 是一个层次化的字符串标识符，使用点号（`.`）分隔层级。例如：

```
Attributes.Primary.Strength
Abilities.Fire.FireBolt
Damage.Fire
```

### Tag 层次结构

Tag 使用点号分隔的层次结构：

```
根标签
  └── 一级分类
      └── 二级分类
          └── 具体标签
```

**示例**：
- `Attributes.Primary.Strength` - 主属性：力量
- `Abilities.Fire.FireBolt` - 火焰技能：火球
- `Damage.Fire` - 火焰伤害

### Tag 匹配规则

GameplayTags 支持层次匹配：

- **精确匹配** (`MatchesTagExact`): 完全匹配，包括所有父标签
- **父标签匹配** (`MatchesTag`): 匹配标签或其任何父标签
- **容器匹配**: 检查容器中是否包含匹配的标签

---

## 项目中的 GameplayTags 实现

### FAuraGameplayTags 结构

`FAuraGameplayTags` 是一个单例结构，包含项目中所有 Native Gameplay Tags：

```cpp
struct FAuraGameplayTags
{
public:
    static const FAuraGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeGameplayTags();

    // 属性 Tags
    FGameplayTag Attributes_Primary_Strength;
    FGameplayTag Attributes_Primary_Intelligence;
    // ... 更多属性 Tags

    // 输入 Tags
    FGameplayTag InputTag_LMB;
    FGameplayTag InputTag_RMB;
    // ... 更多输入 Tags

    // 伤害类型 Tags
    FGameplayTag Damage_Fire;
    FGameplayTag Damage_Lightning;
    // ... 更多伤害类型 Tags

    // 能力 Tags
    FGameplayTag Abilities_Fire_FireBolt;
    FGameplayTag Abilities_Fire_FireBlast;
    // ... 更多能力 Tags

    // 映射表
    TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
    TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

private:
    static FAuraGameplayTags GameplayTags;
};
```

### 获取 Tag

使用单例模式获取 Tag：

```cpp
const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
FGameplayTag FireDamageTag = Tags.Damage_Fire;
```

---

## Tag 分类和结构

### 1. 属性 Tags (Attributes)

#### 主属性 (Primary Attributes)

```
Attributes.Primary.Strength      - 力量
Attributes.Primary.Intelligence - 智力
Attributes.Primary.Resilience   - 韧性
Attributes.Primary.Vigor        - 活力
```

#### 次属性 (Secondary Attributes)

```
Attributes.Secondary.Armor                    - 护甲
Attributes.Secondary.ArmorPenetration         - 护甲穿透
Attributes.Secondary.BlockChance              - 格挡几率
Attributes.Secondary.CriticalHitChance        - 暴击几率
Attributes.Secondary.CriticalHitDamage        - 暴击伤害
Attributes.Secondary.CriticalHitResistance    - 暴击抗性
Attributes.Secondary.HealthRegeneration        - 生命回复
Attributes.Secondary.ManaRegeneration         - 法力回复
Attributes.Secondary.MaxHealth                 - 最大生命值
Attributes.Secondary.MaxMana                   - 最大法力值
```

#### 抗性 (Resistances)

```
Attributes.Resistance.Fire      - 火焰抗性
Attributes.Resistance.Lightning - 闪电抗性
Attributes.Resistance.Arcane    - 奥术抗性
Attributes.Resistance.Physical  - 物理抗性
```

#### 元属性 (Meta Attributes)

```
Attributes.Meta.IncomingXP     - 获得经验值
```

### 2. 输入 Tags (Input Tags)

```
InputTag.LMB          - 鼠标左键
InputTag.RMB          - 鼠标右键
InputTag.1            - 数字键 1
InputTag.2            - 数字键 2
InputTag.3            - 数字键 3
InputTag.4            - 数字键 4
InputTag.Passive.1    - 被动技能 1
InputTag.Passive.2    - 被动技能 2
```

### 3. 伤害类型 Tags (Damage Types)

```
Damage                - 基础伤害标签
Damage.Fire           - 火焰伤害
Damage.Lightning      - 闪电伤害
Damage.Arcane         - 奥术伤害
Damage.Physical       - 物理伤害
```

### 4. Debuff Tags

```
Debuff.Burn           - 燃烧 Debuff
Debuff.Stun           - 眩晕 Debuff
Debuff.Arcane         - 奥术 Debuff
Debuff.Physical       - 物理 Debuff

Debuff.Chance         - Debuff 触发几率
Debuff.Damage         - Debuff 伤害值
Debuff.Duration       - Debuff 持续时间
Debuff.Frequency      - Debuff 触发频率
```

### 5. 能力 Tags (Abilities)

#### 能力状态 (Ability Status)

```
Abilities.Status.Locked     - 锁定状态
Abilities.Status.Eligible  - 可解锁状态
Abilities.Status.Unlocked  - 已解锁状态
Abilities.Status.Equipped  - 已装备状态
```

#### 能力类型 (Ability Types)

```
Abilities.Type.None        - 无类型
Abilities.Type.Offensive   - 攻击类型
Abilities.Type.Passive     - 被动类型
```

#### 具体能力

```
Abilities.None                    - 无能力
Abilities.Attack                  - 攻击能力
Abilities.Summon                  - 召唤能力
Abilities.HitReact                - 受击反应能力

Abilities.Fire.FireBolt           - 火球术
Abilities.Fire.FireBlast          - 火焰冲击
Abilities.Lightning.Electrocute   - 电击
Abilities.Arcane.ArcaneShards     - 奥术碎片

Abilities.Passive.HaloOfProtection - 光环保护
Abilities.Passive.LifeSiphon      - 生命汲取
Abilities.Passive.ManaSiphon      - 法力汲取
```

#### 冷却 Tags (Cooldown Tags)

```
Cooldown.Fire.FireBolt      - 火球术冷却
Cooldown.Fire.FireBlast     - 火焰冲击冷却
Cooldown.Lightning.Electrocute - 电击冷却
Cooldown.Arcane.ArcaneShards  - 奥术碎片冷却
```

### 6. 战斗相关 Tags

#### 战斗插槽 (Combat Sockets)

```
CombatSocket.Weapon      - 武器插槽
CombatSocket.RightHand   - 右手插槽
CombatSocket.LeftHand    - 左手插槽
CombatSocket.Tail        - 尾部插槽
```

#### 动画蒙太奇 Tags

```
Montage.Attack.1         - 攻击动画 1
Montage.Attack.2         - 攻击动画 2
Montage.Attack.3         - 攻击动画 3
Montage.Attack.4         - 攻击动画 4
```

### 7. 效果 Tags (Effects)

```
Effects.HitReact         - 受击反应效果
```

### 8. 玩家阻塞 Tags (Player Block Tags)

```
Player.Block.CursorTrace    - 阻塞光标追踪
Player.Block.InputPressed   - 阻塞输入按下
Player.Block.InputHeld      - 阻塞输入保持
Player.Block.InputReleased  - 阻塞输入释放
```

### 9. GameplayCue Tags

```
GameplayCue.FireBlast    - 火焰冲击 GameplayCue
```

---

## Tag 初始化和注册

### 初始化流程

GameplayTags 在 `UAuraAssetManager::StartInitialLoading()` 中初始化：

```cpp
void UAuraAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // 初始化 Native Gameplay Tags
    FAuraGameplayTags::InitializeNativeGameplayTags();
    
    // 初始化 GAS 全局数据
    UAbilitySystemGlobals::Get().InitGlobalData();
}
```

### 注册 Native Tag

在 `FAuraGameplayTags::InitializeNativeGameplayTags()` 中注册所有 Tags：

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // 注册主属性
    GameplayTags.Attributes_Primary_Strength = 
        UGameplayTagsManager::Get().AddNativeGameplayTag(
            FName("Attributes.Primary.Strength"),
            FString("Increases physical damage")
        );
    
    // 注册次属性
    GameplayTags.Attributes_Secondary_Armor = 
        UGameplayTagsManager::Get().AddNativeGameplayTag(
            FName("Attributes.Secondary.Armor"),
            FString("Reduces damage taken, improves Block Chance")
        );
    
    // ... 注册更多 Tags
    
    // 建立映射关系
    GameplayTags.DamageTypesToResistances.Add(
        GameplayTags.Damage_Fire,
        GameplayTags.Attributes_Resistance_Fire
    );
    
    GameplayTags.DamageTypesToDebuffs.Add(
        GameplayTags.Damage_Fire,
        GameplayTags.Debuff_Burn
    );
}
```

### 配置文件中的 Tags

部分 Tags 在 `Config/DefaultGameplayTags.ini` 中定义：

```ini
[/Script/GameplayTags.GameplayTagsSettings]
ImportTagsFromConfig=True
WarnOnInvalidTags=True

+GameplayTagList=(Tag="Attributes.Vital.Health",DevComment="Amount of damage a player can take before death")
+GameplayTagList=(Tag="Attributes.Vital.Mana",DevComment="A resource used to cast spells")
+GameplayTagList=(Tag="Cooldown.Arcane.ArcaneShards",DevComment="")
+GameplayTagList=(Tag="Event.Montage.FireBolt",DevComment="")
+GameplayTagList=(Tag="GameplayCue.FireBlast",DevComment="")
```

---

## Tag 使用场景

### 1. 属性查询

使用 Tag 查询属性值：

```cpp
const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
FGameplayAttribute StrengthAttr = 
    AttributeSet->GetGameplayAttribute(Tags.Attributes_Primary_Strength);
float StrengthValue = AttributeSet->GetStrength();
```

### 2. 能力激活

使用 Input Tag 激活能力：

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            TryActivateAbility(AbilitySpec.Handle);
        }
    }
}
```

### 3. 伤害类型识别

使用 Damage Tag 识别伤害类型：

```cpp
const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDamageType(EffectContextHandle);
if (DamageType == FAuraGameplayTags::Get().Damage_Fire)
{
    // 处理火焰伤害
}
```

### 4. 能力状态检查

检查能力状态：

```cpp
FGameplayTag StatusTag = ASC->GetStatusFromAbilityTag(AbilityTag);
if (StatusTag == FAuraGameplayTags::Get().Abilities_Status_Equipped)
{
    // 能力已装备
}
```

### 5. Debuff 应用

使用 Debuff Tag 应用 Debuff：

```cpp
const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
const FGameplayTag DebuffTag = Tags.DamageTypesToDebuffs[DamageType];
Effect->InheritableOwnedTagsContainer.AddTag(DebuffTag);
```

### 6. 抗性查询

查询伤害类型对应的抗性：

```cpp
const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
FGameplayTag ResistanceTag = Tags.DamageTypesToResistances[DamageType];
FGameplayAttribute ResistanceAttr = AttributeSet->GetGameplayAttribute(ResistanceTag);
```

---

## Tag 查询和匹配

### 精确匹配

```cpp
// 检查 Tag 是否完全匹配
bool bExactMatch = Tag.MatchesTagExact(OtherTag);

// 检查容器中是否包含精确 Tag
bool bHasTag = TagContainer.HasTagExact(Tag);
```

### 父标签匹配

```cpp
// 检查 Tag 是否匹配（包括父标签）
bool bMatches = Tag.MatchesTag(OtherTag);

// 检查容器中是否包含匹配的 Tag
bool bHasMatchingTag = TagContainer.HasTag(Tag);
```

### 查询示例

```cpp
// 检查是否是火焰伤害
const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
bool bIsFireDamage = DamageTag.MatchesTagExact(Tags.Damage_Fire);

// 检查是否是主属性
bool bIsPrimaryAttribute = AttributeTag.MatchesTag(
    FGameplayTag::RequestGameplayTag(FName("Attributes.Primary"))
);

// 检查能力状态
FGameplayTag StatusTag = ASC->GetStatusFromAbilityTag(AbilityTag);
bool bIsEquipped = StatusTag == Tags.Abilities_Status_Equipped;
```

---

## Tag 容器操作

### 创建容器

```cpp
FGameplayTagContainer TagContainer;
TagContainer.AddTag(FAuraGameplayTags::Get().Damage_Fire);
TagContainer.AddTag(FAuraGameplayTags::Get().Damage_Lightning);
```

### 容器查询

```cpp
// 检查是否包含 Tag
bool bHasTag = TagContainer.HasTag(Tag);

// 检查是否包含所有 Tags
bool bHasAll = TagContainer.HasAll(Tags);

// 检查是否包含任何 Tag
bool bHasAny = TagContainer.HasAny(Tags);
```

### 容器操作

```cpp
// 添加 Tag
TagContainer.AddTag(Tag);

// 移除 Tag
TagContainer.RemoveTag(Tag);

// 合并容器
TagContainer.AppendTags(OtherContainer);

// 过滤容器
FGameplayTagContainer FilteredContainer;
TagContainer.Filter(FGameplayTagContainer(), FilteredContainer);
```

---

## 添加新 Tag

### 步骤 1: 在头文件中添加 Tag 变量

在 `Source/Aura/Public/AuraGameplayTags.h` 中添加：

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags
    
    // 新添加的 Tag
    FGameplayTag Damage_Ice;  // 冰霜伤害
    FGameplayTag Attributes_Resistance_Ice;  // 冰霜抗性
    FGameplayTag Debuff_Freeze;  // 冰冻 Debuff
};
```

### 步骤 2: 在实现文件中注册 Tag

在 `Source/Aura/Private/AuraGameplayTags.cpp` 中注册：

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有注册代码
    
    // 注册新 Tags
    GameplayTags.Damage_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Damage.Ice"),
        FString("Ice Damage Type")
    );
    
    GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Attributes.Resistance.Ice"),
        FString("Resistance to Ice damage")
    );
    
    GameplayTags.Debuff_Freeze = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Debuff.Freeze"),
        FString("Freeze Debuff")
    );
    
    // 添加到映射表
    GameplayTags.DamageTypesToResistances.Add(
        GameplayTags.Damage_Ice,
        GameplayTags.Attributes_Resistance_Ice
    );
    
    GameplayTags.DamageTypesToDebuffs.Add(
        GameplayTags.Damage_Ice,
        GameplayTags.Debuff_Freeze
    );
}
```

### 步骤 3: 编译和测试

1. 编译项目
2. 在编辑器中验证 Tag 是否正确注册
3. 使用 Tag 进行测试

---

## 最佳实践

### 1. Tag 命名规范

- **使用点号分隔层级**: `Category.SubCategory.Specific`
- **使用 PascalCase**: `Attributes.Primary.Strength`
- **保持一致性**: 同类 Tags 使用相同的命名模式
- **描述性命名**: Tag 名称应该清晰表达其用途

### 2. Tag 组织

- **按功能分类**: 将相关 Tags 组织在一起
- **使用映射表**: 对于相关 Tags，使用 `TMap` 建立关系
- **避免重复**: 不要创建功能重复的 Tags

### 3. Tag 使用

- **使用常量引用**: 通过 `FAuraGameplayTags::Get()` 获取 Tags
- **避免硬编码**: 不要直接使用字符串创建 Tag
- **使用匹配函数**: 根据需求选择 `MatchesTag` 或 `MatchesTagExact`

### 4. 性能考虑

- **缓存 Tag 引用**: 在需要多次使用的地方缓存 Tag 引用
- **使用 Tag 容器**: 对于多个 Tag 操作，使用 `FGameplayTagContainer`
- **避免频繁查询**: 缓存查询结果，避免重复查询

### 5. 网络复制

- **最小化复制**: 只复制必要的 Tags
- **使用 Tag 索引**: 网络复制使用 Tag 索引而非字符串

---

## 常见问题

### 问题 1: Tag 未找到

**原因**: Tag 未正确注册或初始化顺序问题

**解决方案**:
1. 检查 Tag 是否在 `InitializeNativeGameplayTags()` 中注册
2. 确保 `UAuraAssetManager::StartInitialLoading()` 被调用
3. 检查 Tag 名称拼写是否正确

### 问题 2: Tag 匹配失败

**原因**: 使用了错误的匹配函数

**解决方案**:
- 需要精确匹配时使用 `MatchesTagExact()`
- 需要父标签匹配时使用 `MatchesTag()`
- 检查 Tag 的层次结构

### 问题 3: Tag 容器操作错误

**原因**: 容器操作逻辑错误

**解决方案**:
- 使用 `HasAll()` 检查是否包含所有 Tags
- 使用 `HasAny()` 检查是否包含任何 Tag
- 使用 `AppendTags()` 合并容器

### 问题 4: 网络同步问题

**原因**: Tag 复制配置错误

**解决方案**:
- 检查 `ReplicationMode` 设置
- 确保 Tag 在网络中正确同步
- 使用 Tag 索引而非字符串进行网络传输

---

## 总结

GameplayTags 是 Aura 项目 GAS 系统的核心组件，用于：

- ✅ **标识属性、能力、伤害类型等**
- ✅ **建立映射关系**（伤害类型到抗性、伤害类型到 Debuff）
- ✅ **能力状态管理**（锁定、解锁、装备）
- ✅ **输入绑定**（将输入动作绑定到能力）
- ✅ **效果标识**（受击反应、Debuff 等）

通过合理使用 GameplayTags，可以实现灵活、可扩展的游戏系统。

---

## 相关文档

- [GameplayEffect 系统文档](./GameplayEffect_System.md) - GameplayEffect 详细文档
- [GameplayAbility 系统文档](./GameplayAbility_System.md) - GameplayAbility 详细文档
- [属性系统文档](./Attribute_System.md) - 属性系统实现
- [伤害计算系统文档](./Damage_Calculation.md) - 伤害计算实现

