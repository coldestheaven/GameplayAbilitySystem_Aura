# 架构文档

## 系统架构概览

Aura 项目采用模块化设计，基于 Unreal Engine 5.7 的 Gameplay Ability System (GAS) 构建。整体架构遵循单一职责原则和接口隔离原则。

## 核心架构层次

```
┌─────────────────────────────────────┐
│         UI Layer (MVVM)             │
│  WidgetController → Widget          │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│      Gameplay Ability System        │
│  ASC → AttributeSet → Abilities     │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│      Character System                │
│  PlayerState → Character → Enemy    │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│      Data Layer                      │
│  DataAssets → DataTables            │
└─────────────────────────────────────┘
```

## 1. Gameplay Ability System 架构

### 1.1 核心组件

#### UAuraAbilitySystemComponent
- **职责**: 扩展 UE5 的 AbilitySystemComponent
- **功能**:
  - 能力输入绑定（InputTag）
  - 能力状态管理（Locked, Eligible, Unlocked, Equipped）
  - 能力槽位管理
  - 被动能力激活/停用
  - 属性升级

#### UAuraAttributeSet
- **职责**: 管理所有游戏属性
- **属性分类**:
  - **Primary Attributes**: Strength, Intelligence, Resilience, Vigor
  - **Secondary Attributes**: Armor, CriticalHit, Regeneration 等
  - **Resistance Attributes**: Fire, Lightning, Arcane, Physical
  - **Vital Attributes**: Health, Mana, MaxHealth, MaxMana
  - **Meta Attributes**: IncomingDamage, IncomingXP

### 1.2 能力层次结构

```
UGameplayAbility (UE5 Base)
    ↓
UAuraGameplayAbility (Base Ability)
    ├── UAuraDamageGameplayAbility (Damage Ability)
    │   ├── UAuraProjectileSpell (Projectile)
    │   ├── UAuraBeamSpell (Beam)
    │   └── UAuraSummonAbility (Summon)
    ├── UAuraMeleeAttack (Melee)
    └── UAuraPassiveAbility (Passive)
```

### 1.3 伤害计算系统

#### UExecCalc_Damage
- **职责**: 自定义伤害计算
- **计算流程**:
  1. 捕获源和目标属性
  2. 计算基础伤害
  3. 应用护甲和穿透
  4. 计算暴击
  5. 应用抗性
  6. 处理 Debuff
  7. 处理范围伤害

#### 伤害类型映射
- Fire → FireResistance
- Lightning → LightningResistance
- Arcane → ArcaneResistance
- Physical → PhysicalResistance

## 2. 角色系统架构

### 2.1 角色类层次

```
ACharacter (UE5 Base)
    ↓
AAuraCharacterBase (Abstract Base)
    ├── AAuraCharacter (Player)
    └── AAuraEnemy (Enemy)
```

### 2.2 接口系统

#### ICombatInterface
- 战斗相关接口
- 方法：
  - `GetCombatSocketLocation()` - 获取战斗插槽位置
  - `GetHitReactMontage()` - 受击动画
  - `Die()` - 死亡处理
  - `IsDead()` - 死亡状态
  - `GetPlayerLevel()` - 玩家等级

#### IPlayerInterface
- 玩家相关接口
- 方法：
  - `AddToXP()` - 增加经验
  - `AddToLevel()` - 升级
  - `GetAttributePoints()` - 属性点
  - `GetSpellPoints()` - 法术点

### 2.3 玩家状态系统

#### AAuraPlayerState
- **职责**: 管理玩家状态数据
- **数据**:
  - Level, XP
  - AttributePoints, SpellPoints
  - AbilitySystemComponent (拥有者)
  - AttributeSet (拥有者)

#### AAuraPlayerController
- **职责**: 玩家输入和 UI 控制

## 3. UI 系统架构 (MVVM)

### 3.1 MVVM 模式

```
Model (GAS Data)
    ↓
ViewModel (WidgetController)
    ↓
View (Widget)
```

### 3.2 Widget Controller 层次

```
UAuraWidgetController (Base)
    ├── UOverlayWidgetController
    ├── UAttributeMenuWidgetController
    └── USpellMenuWidgetController
```

### 3.3 数据流

1. **初始化**:
   - WidgetController 从 HUD 获取
   - 设置 WidgetControllerParams (PC, PS, ASC, AS)
   - 调用 `BroadcastInitialValues()`
   - 调用 `BindCallbacksToDependencies()`

2. **更新流程**:
   - GAS 属性变化 → AttributeSet 回调
   - WidgetController 接收回调
   - 广播委托给 Widget
   - Widget 更新显示

## 4. 数据驱动架构

### 4.1 数据资产

#### UAbilityInfo
- 存储能力信息
- 包含：AbilityTag, InputTag, StatusTag, Icon, LevelRequirement

#### UCharacterClassInfo
- 存储职业信息
- 包含：PrimaryAttributes, StartupAbilities, XPReward

#### ULevelUpInfo
- 存储升级信息
- 包含：每级所需经验值

### 4.2 Data Tables

- `CT_Damage.json` - 伤害系数表
- `CT_PrimaryAttributes_*.csv/json` - 职业属性表

## 5. 网络架构

### 5.1 复制策略

- **PlayerState**: 拥有 AbilitySystemComponent (Mixed Replication)
- **Character**: 复制状态标志 (Stunned, Burned, BeingShocked)
- **AbilitySystemComponent**: Mixed Replication Mode

### 5.2 RPC 使用

- `ServerUpgradeAttribute()` - 属性升级
- `ServerSpendSpellPoint()` - 消耗法术点
- `ServerEquipAbility()` - 装备能力
- `MulticastHandleDeath()` - 死亡处理
- `MulticastActivatePassiveEffect()` - 被动效果激活

## 6. 设计模式

### 6.1 单例模式
- `FAuraGameplayTags` - Gameplay Tags 单例

### 6.2 工厂模式
- `UAuraAbilitySystemLibrary` - 工具函数集合

### 6.3 观察者模式
- 委托系统（Delegates）
- 属性变化回调

### 6.4 策略模式
- 不同能力类型的实现策略

## 7. 系统交互流程

### 7.1 能力激活流程

```
Input → InputComponent
    ↓
AbilityInputTagPressed/Held
    ↓
AuraAbilitySystemComponent
    ↓
TryActivateAbility
    ↓
Ability.Activate()
    ↓
Execute Ability Logic
```

### 7.2 伤害处理流程

```
Ability.CauseDamage()
    ↓
MakeDamageEffectParams
    ↓
ApplyGameplayEffectSpecToTarget
    ↓
ExecCalc_Damage.Execute()
    ↓
AttributeSet.PostGameplayEffectExecute()
    ↓
HandleIncomingDamage()
    ↓
Update Health
    ↓
Broadcast to UI
```

### 7.3 UI 更新流程

```
Attribute Changed
    ↓
AttributeSet Callback
    ↓
WidgetController.BindCallbacksToDependencies()
    ↓
Delegate Broadcast
    ↓
Widget Update
```

## 8. 扩展点

### 8.1 添加新能力
1. 继承 `UAuraGameplayAbility` 或子类
2. 在 `AbilityInfo` 数据资产中添加信息
3. 配置 GameplayEffect (Cost, Cooldown)
4. 添加到角色的 `StartupAbilities`

### 8.2 添加新属性
1. 在 `UAuraAttributeSet` 中添加属性
2. 添加对应的 GameplayTag
3. 创建初始化 GameplayEffect
4. 在 UI 中添加显示

### 8.3 添加新 Debuff
1. 添加 Debuff GameplayTag
2. 在 `ExecCalc_Damage` 中处理 Debuff 逻辑
3. 创建 Debuff GameplayEffect
4. 添加 DebuffNiagaraComponent

## 9. 性能考虑

### 9.1 优化策略
- 使用对象池管理投射物
- 限制同时激活的能力数量
- 使用事件驱动而非 Tick
- 合理使用网络复制

### 9.2 内存管理
- 使用 TObjectPtr 管理 UObject 引用
- 及时清理委托绑定
- 避免循环引用

## 10. 测试建议

### 10.1 单元测试
- 属性计算测试
- 伤害计算测试
- 能力激活测试

### 10.2 集成测试
- 能力系统集成
- UI 更新测试
- 网络同步测试

