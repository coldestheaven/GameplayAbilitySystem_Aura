# 系统设计文档

## 1. 能力系统设计

### 1.1 能力生命周期

#### 能力状态机

```
Locked → Eligible → Unlocked → Equipped
                ↓
            (Spend Spell Point)
                ↓
            (Equip Ability)
```

- **Locked**: 能力未解锁，需要达到等级要求
- **Eligible**: 达到等级要求，可以使用法术点解锁
- **Unlocked**: 已解锁但未装备
- **Equipped**: 已装备到输入槽位

#### 能力激活流程

1. **输入检测**: `AbilityInputTagPressed/Held/Released`
2. **能力查找**: 根据 InputTag 查找对应的 AbilitySpec
3. **激活检查**: 检查能力是否可以激活（冷却、成本等）
4. **激活能力**: 调用 `TryActivateAbility()`
5. **执行逻辑**: 能力执行其具体逻辑
6. **完成/取消**: 能力完成或取消

### 1.2 能力类型系统

#### 主动能力 (Offensive)

- **AuraProjectileSpell**: 投射物法术
  - 生成投射物 Actor
  - 设置伤害参数
  - 处理碰撞检测

- **AuraBeamSpell**: 光束法术
  - 持续伤害
  - 目标追踪

- **AuraMeleeAttack**: 近战攻击
  - 使用动画蒙太奇
  - 近战范围检测

- **AuraSummonAbility**: 召唤能力
  - 生成召唤物
  - 管理召唤物数量

#### 被动能力 (Passive)

- **AuraPassiveAbility**: 被动能力基类
  - 自动激活
  - 持续效果
  - 不可手动触发

### 1.3 能力槽位系统

#### 槽位管理

- 每个能力可以分配到一个输入槽位（LMB, RMB, 1-4）
- 一个槽位只能有一个能力
- 装备新能力时，旧能力会被移除槽位

#### 槽位操作

```cpp
// 检查槽位是否为空
bool SlotIsEmpty(const FGameplayTag& Slot);

// 获取槽位中的能力
FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);

// 分配槽位给能力
void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

// 清除槽位
void ClearSlot(FGameplayAbilitySpec* Spec);
```

---

## 2. 属性系统设计

### 2.1 属性分类

#### 主属性 (Primary Attributes)

影响次属性的基础属性：

- **Strength**: 力量
  - 影响物理伤害
  - 影响最大生命值

- **Intelligence**: 智力
  - 影响法术伤害
  - 影响最大法力值

- **Resilience**: 韧性
  - 影响护甲
  - 影响抗性

- **Vigor**: 活力
  - 影响最大生命值
  - 影响生命恢复

#### 次属性 (Secondary Attributes)

由主属性计算得出：

- **Armor**: 护甲值
- **ArmorPenetration**: 护甲穿透
- **BlockChance**: 格挡几率
- **CriticalHitChance**: 暴击几率
- **CriticalHitDamage**: 暴击伤害
- **CriticalHitResistance**: 暴击抗性
- **HealthRegeneration**: 生命恢复
- **ManaRegeneration**: 法力恢复
- **MaxHealth**: 最大生命值
- **MaxMana**: 最大法力值

#### 抗性属性 (Resistance Attributes)

减少对应类型伤害：

- **FireResistance**: 火焰抗性
- **LightningResistance**: 闪电抗性
- **ArcaneResistance**: 奥术抗性
- **PhysicalResistance**: 物理抗性

### 2.2 属性计算

#### Modifier Magnitude Calculation (MMC)

使用 `MMC_MaxHealth` 和 `MMC_MaxMana` 计算最大生命值和法力值：

```cpp
// 示例：MaxHealth 计算
float MaxHealth = BaseValue + (Vigor * Coefficient) + (Strength * Coefficient);
```

### 2.3 属性初始化

#### 初始化流程

1. **应用主属性**: `DefaultPrimaryAttributes` GameplayEffect
2. **应用次属性**: `DefaultSecondaryAttributes` GameplayEffect
3. **应用生命值**: `DefaultVitalAttributes` GameplayEffect

#### 职业差异

不同职业有不同的初始属性值，通过 `CharacterClassInfo` 数据资产配置。

---

## 3. 伤害系统设计

### 3.1 伤害计算流程

#### ExecCalc_Damage 执行流程

```
1. 捕获源和目标属性
   ↓
2. 获取基础伤害值（从 SetByCaller）
   ↓
3. 计算护甲减免
   Damage = Damage * (100 - Armor) / 100
   ↓
4. 应用护甲穿透
   EffectiveArmor = Armor - ArmorPenetration
   ↓
5. 计算暴击
   if (Random < CriticalHitChance - CriticalHitResistance)
       Damage = Damage * (1 + CriticalHitDamage)
   ↓
6. 应用抗性
   Damage = Damage * (100 - Resistance) / 100
   ↓
7. 处理范围伤害（如果适用）
   ↓
8. 应用最终伤害
   ↓
9. 处理 Debuff
```

### 3.2 伤害类型

#### 伤害类型映射

- `Damage_Fire` → `Attributes_Resistance_Fire`
- `Damage_Lightning` → `Attributes_Resistance_Lightning`
- `Damage_Arcane` → `Attributes_Resistance_Arcane`
- `Damage_Physical` → `Attributes_Resistance_Physical`

### 3.3 范围伤害

#### 范围伤害处理

1. 使用 `UGameplayStatics::ApplyRadialDamageWithFalloff`
2. 通过 Lambda 捕获实际造成的伤害
3. 更新伤害值用于显示

### 3.4 Debuff 系统

#### Debuff 类型

- **Burn**: 持续火焰伤害
- **Stun**: 眩晕效果
- **Arcane**: 奥术 Debuff
- **Physical**: 物理 Debuff

#### Debuff 应用

1. 计算 Debuff 触发几率
2. 如果触发，创建 Debuff GameplayEffect
3. 设置 Debuff 参数（伤害、持续时间、频率）
4. 应用 Debuff 到目标

---

## 4. UI 系统设计

### 4.1 MVVM 架构

#### Model (数据层)

- `UAuraAttributeSet`: 属性数据
- `UAuraAbilitySystemComponent`: 能力数据
- `AAuraPlayerState`: 玩家状态数据

#### ViewModel (逻辑层)

- `UAuraWidgetController`: Widget Controller 基类
- `UOverlayWidgetController`: 覆盖层控制器
- `UAttributeMenuWidgetController`: 属性菜单控制器
- `USpellMenuWidgetController`: 法术菜单控制器

#### View (视图层)

- UMG Widgets: 各种 UI 组件

### 4.2 Widget Controller 模式

#### 初始化流程

```
1. Widget 请求 WidgetController
   ↓
2. HUD 创建或返回 WidgetController
   ↓
3. 设置 WidgetControllerParams
   ↓
4. 调用 BroadcastInitialValues()
   ↓
5. 调用 BindCallbacksToDependencies()
   ↓
6. Widget 绑定到委托
```

#### 数据更新流程

```
属性变化
   ↓
AttributeSet 回调
   ↓
WidgetController 接收
   ↓
广播委托
   ↓
Widget 更新显示
```

### 4.3 委托系统

#### 常用委托

- `FOnPlayerStatChangedSignature`: 玩家状态改变
- `FAbilityInfoSignature`: 能力信息
- `FOnAttributeChanged`: 属性改变
- `FAbilityStatusChanged`: 能力状态改变
- `FAbilityEquipped`: 能力装备

---

## 5. 角色系统设计

### 5.1 角色类设计

#### AAuraCharacterBase

- **抽象基类**: 不直接实例化
- **接口实现**: IAbilitySystemInterface, ICombatInterface
- **功能**:
  - 能力系统初始化
  - 属性初始化
  - 死亡处理
  - 战斗接口实现

#### AAuraCharacter (玩家)

- 继承自 `AAuraCharacterBase`
- 玩家特定逻辑
- 输入处理

#### AAuraEnemy (敌人)

- 继承自 `AAuraCharacterBase`
- AI 控制
- 敌人特定行为

### 5.2 战斗接口

#### ICombatInterface

提供战斗相关功能：

- `GetCombatSocketLocation()`: 获取战斗插槽位置
- `GetHitReactMontage()`: 受击动画
- `Die()`: 死亡处理
- `IsDead()`: 死亡状态
- `GetAttackMontages()`: 攻击动画

### 5.3 死亡系统

#### 死亡流程

```
1. 生命值降至 0
   ↓
2. 调用 Die()
   ↓
3. MulticastHandleDeath()
   ↓
4. 播放死亡音效
   ↓
5. 武器物理模拟
   ↓
6. 角色物理模拟
   ↓
7. 溶解效果
   ↓
8. 广播死亡委托
```

---

## 6. 玩家系统设计

### 6.1 PlayerState 设计

#### AAuraPlayerState

- **拥有 ASC**: 玩家拥有 AbilitySystemComponent
- **状态数据**: Level, XP, AttributePoints, SpellPoints
- **网络复制**: 所有状态数据都复制到客户端

#### 状态管理

- 使用 `ReplicatedUsing` 进行网络复制
- 状态改变时广播委托
- 客户端通过 `OnRep_*` 函数接收更新

### 6.2 升级系统

#### 经验值系统

- 击败敌人获得经验值
- 经验值达到阈值时升级
- 升级时获得属性点和法术点

#### 属性点系统

- 升级获得属性点
- 消耗属性点升级主属性
- 通过 GameplayEvent 触发属性升级

#### 法术点系统

- 升级获得法术点
- 消耗法术点解锁或升级能力
- 能力状态从 Eligible → Unlocked

---

## 7. AI 系统设计

### 7.1 AI 控制器

#### AAuraAIController

- 控制敌人行为
- 行为树集成
- 目标选择

### 7.2 行为树

#### BTService_FindNearestPlayer

- 查找最近的玩家
- 更新黑板值

#### BTTask_Attack

- 执行攻击
- 激活攻击能力

---

## 8. 数据驱动设计

### 8.1 数据资产

#### UAbilityInfo

存储所有能力的信息：
- AbilityTag
- InputTag
- StatusTag
- Icon
- LevelRequirement
- Ability Class

#### UCharacterClassInfo

存储职业信息：
- PrimaryAttributes GameplayEffect
- StartupAbilities
- XPReward
- DamageCalculationCoefficients

#### ULevelUpInfo

存储升级信息：
- 每级所需经验值

### 8.2 Data Tables

#### 伤害系数表

- `CT_Damage.json`: 伤害计算系数

#### 职业属性表

- `CT_PrimaryAttributes_Warrior.json`
- `CT_PrimaryAttributes_Ranger.json`
- `CT_PrimaryAttributes_Elementalist.csv`

---

## 9. 网络架构设计

### 9.1 复制策略

#### PlayerState 拥有 ASC

- **优势**: 玩家状态数据在服务器权威
- **复制模式**: Mixed Replication
- **适用场景**: 玩家角色

#### Character 拥有 ASC

- **优势**: 敌人可以独立控制
- **复制模式**: Minimal Replication
- **适用场景**: AI 敌人

### 9.2 RPC 使用

#### Server RPC

- `ServerUpgradeAttribute()`: 属性升级
- `ServerSpendSpellPoint()`: 消耗法术点
- `ServerEquipAbility()`: 装备能力

#### Client RPC

- `ClientEquipAbility()`: 装备能力回调
- `ClientUpdateAbilityStatus()`: 更新能力状态
- `ClientEffectApplied()`: 效果应用回调

#### Multicast RPC

- `MulticastHandleDeath()`: 死亡处理
- `MulticastActivatePassiveEffect()`: 被动效果激活

---

## 10. 扩展指南

### 10.1 添加新能力

1. 创建能力类（继承 `UAuraGameplayAbility` 或子类）
2. 在 `AbilityInfo` 数据资产中添加信息
3. 创建 GameplayEffect（Cost, Cooldown）
4. 添加到角色的 `StartupAbilities` 数组

### 10.2 添加新属性

1. 在 `UAuraAttributeSet` 中添加属性
2. 添加对应的 GameplayTag
3. 创建初始化 GameplayEffect
4. 在 UI 中添加显示

### 10.3 添加新 Debuff

1. 添加 Debuff GameplayTag
2. 在 `ExecCalc_Damage` 中处理 Debuff 逻辑
3. 创建 Debuff GameplayEffect
4. 添加 DebuffNiagaraComponent（如果需要视觉效果）

### 10.4 添加新职业

1. 在 `ECharacterClass` 枚举中添加新职业
2. 在 `CharacterClassInfo` 数据资产中配置职业信息
3. 创建职业特定的属性初始化 GameplayEffect
4. 配置职业特定的能力

