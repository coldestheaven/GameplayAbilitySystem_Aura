# 项目总览

## 项目简介

Aura 是一个基于 Unreal Engine 5.7 的完整 Gameplay Ability System (GAS) 实现项目。该项目展示了如何在 UE5 中构建一个功能完整的 ARPG 风格游戏系统，包含完整的技能系统、属性系统、UI 系统、AI 系统、存档系统等。

## 项目特点

- ✅ **完整的 GAS 实现** - 基于 UE5 的 Gameplay Ability System
- ✅ **数据驱动设计** - 使用 Data Assets 配置游戏数据
- ✅ **MVVM UI 架构** - 使用 Model View ViewModel 插件
- ✅ **多职业系统** - 支持 Elementalist, Warrior, Ranger
- ✅ **完整的存档系统** - 支持玩家进度和世界状态保存
- ✅ **AI 系统** - 行为树和黑板实现的敌人 AI
- ✅ **网络支持** - 完整的多人游戏支持

---

## 系统架构总览

### 核心系统层次

```
┌─────────────────────────────────────────┐
│         Presentation Layer              │
│  UI (MVVM) → Widget Controllers         │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│      Gameplay Ability System            │
│  ASC → AttributeSet → Abilities         │
│  ExecCalc → ModMagCalc → Effects        │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         Character System                │
│  PlayerState → Character → Enemy         │
│  PlayerController → Input               │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         Gameplay Framework              │
│  GameMode → GameInstance → SaveGame     │
│  AssetManager → World State             │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         Data Layer                      │
│  DataAssets → DataTables → Config       │
└─────────────────────────────────────────┘
```

---

## 核心系统

### 1. Gameplay Ability System (GAS)

**位置**: `Source/Aura/AbilitySystem/`

**核心组件**:
- `UAuraAbilitySystemComponent` - 扩展的 ASC
- `UAuraAttributeSet` - 属性管理
- `UAuraGameplayAbility` - 技能基类体系
- `UExecCalc_Damage` - 伤害计算
- `UAuraAbilitySystemLibrary` - 工具函数库

**文档**: [技能系统](./Core/Ability_System.md), [属性系统](./Systems/Attribute_System.md), [伤害计算](./Systems/Damage_Calculation.md)

---

### 2. 角色系统

**位置**: `Source/Aura/Character/`, `Source/Aura/Player/`

**核心组件**:
- `AAuraCharacterBase` - 角色基类
- `AAuraCharacter` - 玩家角色
- `AAuraEnemy` - 敌人角色
- `AAuraPlayerState` - 玩家状态
- `AAuraPlayerController` - 玩家控制器

**文档**: [角色系统](./Systems/Character_System.md), [玩家系统](./Systems/Player_System.md)

---

### 3. UI 系统

**位置**: `Source/Aura/UI/`

**核心组件**:
- `UAuraWidgetController` - Widget Controller 基类
- `UOverlayWidgetController` - 覆盖层控制器
- `UAttributeMenuWidgetController` - 属性菜单控制器
- `USpellMenuWidgetController` - 法术菜单控制器
- `UMVVM_LoadScreen` - 加载屏幕 ViewModel

**文档**: [UI 系统](./Systems/UI_System.md), [MVVM 系统](./Systems/MVVM_System.md)

---

### 4. AI 系统

**位置**: `Source/Aura/AI/`

**核心组件**:
- `AAuraAIController` - AI 控制器
- `UBTService_FindNearestPlayer` - 寻找最近玩家服务
- `UBTTask_Attack` - 攻击任务

**文档**: [AI 系统](./Systems/AI_System.md)

---

### 5. 输入系统

**位置**: `Source/Aura/Input/`

**核心组件**:
- `UAuraInputComponent` - 自定义输入组件
- `UAuraInputConfig` - 输入配置数据资产

**文档**: [输入系统](./Systems/Input_System.md)

---

### 6. Actor 系统

**位置**: `Source/Aura/Actor/`

**核心组件**:
- `AAuraEffectActor` - 效果应用 Actor
- `AAuraProjectile` - 投射物
- `AAuraEnemySpawnVolume` - 敌人生成体积
- `AAuraEnemySpawnPoint` - 敌人生成点

**文档**: [Actor 系统](./Systems/Actor_System.md)

---

### 7. 交互系统

**位置**: `Source/Aura/Interaction/`, `Source/Aura/Checkpoint/`

**核心组件**:
- `ICombatInterface` - 战斗接口
- `IEnemyInterface` - 敌人接口
- `IHighlightInterface` - 高亮接口
- `ISaveInterface` - 存档接口
- `IPlayerInterface` - 玩家接口
- `ACheckpoint` - 检查点
- `AMapEntrance` - 地图入口

**文档**: [交互系统](./Systems/Interaction_System.md), [Checkpoint 系统](./Systems/Checkpoint_System.md)

---

### 8. Gameplay 框架

**位置**: `Source/Aura/Game/`

**核心组件**:
- `AAuraGameModeBase` - 游戏模式
- `UAuraGameInstance` - 游戏实例
- `ULoadScreenSaveGame` - 存档数据
- `UAuraAssetManager` - 资源管理器

**文档**: [Gameplay 框架](./Gameplay/Gameplay_Framework.md), [Asset Manager](./Systems/Asset_Manager_System.md)

---

### 9. 数据资产系统

**位置**: `Source/Aura/AbilitySystem/Data/`

**核心组件**:
- `UCharacterClassInfo` - 职业信息
- `UAbilityInfo` - 能力信息
- `UAttributeInfo` - 属性信息
- `ULevelUpInfo` - 等级信息
- `ULootTiers` - 战利品等级

**文档**: [数据资产系统](./Systems/Data_Assets_System.md)

---

## 技能系统

### 技能类型

1. **投射物技能** (`UAuraProjectileSpell`)
   - FireBolt, FireBlast, ArcaneShards
   - 支持多投射物、追踪、扇形分布

2. **光束技能** (`UAuraBeamSpell`)
   - Electrocute
   - 持续伤害、目标链

3. **近战技能** (`UAuraMeleeAttack`)
   - MeleeAttack
   - 动画蒙太奇、武器碰撞

4. **召唤技能** (`UAuraSummonAbility`)
   - SummonAbility
   - 生成召唤物

5. **被动技能** (`UAuraPassiveAbility`)
   - HaloOfProtection, LifeSiphon, ManaSiphon
   - 自动激活、持续效果

**文档**: [技能详细文档](./Abilities/README.md)

---

## 属性系统

### 属性分类

1. **主属性** (Primary Attributes)
   - Strength, Intelligence, Resilience, Vigor

2. **次属性** (Secondary Attributes)
   - Armor, CriticalHit, CriticalHitResistance
   - HealthRegeneration, ManaRegeneration
   - MaxHealth, MaxMana

3. **抗性属性** (Resistance Attributes)
   - FireResistance, LightningResistance
   - ArcaneResistance, PhysicalResistance

4. **生命值属性** (Vital Attributes)
   - Health, Mana

5. **元属性** (Meta Attributes)
   - IncomingDamage, IncomingXP

**文档**: [属性系统](./Systems/Attribute_System.md)

---

## 接口系统

### 核心接口

1. **ICombatInterface** - 战斗接口
   - 战斗相关方法（攻击、死亡、伤害等）

2. **IEnemyInterface** - 敌人接口
   - 敌人特定方法

3. **IHighlightInterface** - 高亮接口
   - 高亮显示可交互对象

4. **ISaveInterface** - 存档接口
   - 存档和加载方法

5. **IPlayerInterface** - 玩家接口
   - 玩家特定方法（经验值、等级等）

**文档**: [交互系统](./Systems/Interaction_System.md)

---

## 数据流

### 技能激活流程

```
输入事件
    ↓
AuraInputComponent
    ↓
AuraAbilitySystemComponent
    ↓
查找 AbilitySpec
    ↓
检查激活条件
    ↓
激活技能
    ↓
执行技能逻辑
    ↓
应用 GameplayEffect
    ↓
更新 UI
```

### 属性变化流程

```
GameplayEffect 应用
    ↓
AttributeSet 属性变化
    ↓
PostAttributeChange
    ↓
WidgetController 委托
    ↓
UI Widget 更新
```

---

## 网络架构

### 复制策略

- **玩家状态**: 复制到所有客户端
- **角色属性**: 通过 GAS 复制
- **技能状态**: 通过 GAS 复制
- **世界状态**: 服务器权威

### 网络组件

- `UAuraAbilitySystemComponent` - 网络复制
- `UAuraAttributeSet` - 属性复制
- `AAuraCharacterBase` - 状态复制

---

## 开发工作流

### 添加新功能

1. **设计阶段**
   - 确定功能需求
   - 选择合适的基础类
   - 设计数据流

2. **实现阶段**
   - 创建 C++ 类
   - 实现核心逻辑
   - 创建蓝图

3. **配置阶段**
   - 配置数据资产
   - 设置 GameplayEffect
   - 配置 UI

4. **测试阶段**
   - 单元测试
   - 集成测试
   - 网络测试

**文档**: [实现指南](./Guides/README.md)

---

## 项目结构

```
Source/Aura/
├── AbilitySystem/        # 能力系统
│   ├── Abilities/       # 技能实现
│   ├── Data/            # 数据资产
│   ├── ExecCalc/        # 执行计算器
│   ├── ModMagCalc/      # 修正量计算器
│   ├── AbilityTasks/    # 能力任务
│   ├── AsyncTasks/      # 异步任务
│   └── ...
├── Character/           # 角色类
├── Player/              # 玩家系统
├── UI/                  # UI 系统
├── AI/                  # AI 系统
├── Input/               # 输入系统
├── Actor/               # Actor 类
├── Interaction/         # 交互接口
├── Checkpoint/          # 检查点
└── Game/                # 游戏系统
```

---

## 技术栈

- **引擎**: Unreal Engine 5.7
- **语言**: C++
- **插件**:
  - GameplayAbilities
  - MotionWarping
  - ModelViewViewModel
  - EnhancedInput

---

## 文档导航

### 新手入门
- [快速开始](./Getting_Started/QuickStart.md) - 从这里开始

### 核心文档
- [架构文档](./Core/Architecture.md) - 系统架构
- [系统设计](./Core/System_Design.md) - 设计思路
- [API 参考](./Core/API_Reference.md) - API 文档
- [技能系统](./Core/Ability_System.md) - 技能系统详解

### 系统文档
- [子系统文档](./Systems/README.md) - 所有子系统文档

### 实现指南
- [实现指南](./Guides/README.md) - 开发指南

### 技能文档
- [技能详细文档](./Abilities/README.md) - 每个技能的说明

---

## 总结

Aura 项目是一个完整的 GAS 实现示例，展示了：

1. ✅ **完整的系统架构** - 从底层到 UI 的完整实现
2. ✅ **数据驱动设计** - 易于配置和调整
3. ✅ **模块化设计** - 清晰的职责划分
4. ✅ **网络支持** - 完整的多人游戏支持
5. ✅ **扩展性** - 易于添加新功能

通过这个项目，可以学习到如何在 UE5 中构建一个完整的 ARPG 游戏系统。


