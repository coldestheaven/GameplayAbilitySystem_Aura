# Aura 项目架构文档

> **GameplayAbilitySystem_Aura** 完整架构文档  
> **引擎版本**: Unreal Engine 5.7  
> **文档版本**: v1.0  
> **更新日期**: 2026-01-24

---

## 📚 文档导航

### 🌟 核心架构文档（推荐优先阅读）
1. **[架构总览](./01_架构总览.md)** ⭐⭐⭐⭐⭐
   - 项目整体架构设计
   - 分层架构详解
   - 技术栈和设计模式
   - 数据流向分析
   - **推荐首先阅读**

2. **[GAS 系统详解](./02_GAS系统详解.md)** ⭐⭐⭐⭐⭐
   - Gameplay Ability System 深度解析
   - AbilitySystemComponent 详解
   - AttributeSet 属性系统
   - 技能系统架构
   - 效果计算和标签系统
   - 网络复制机制

3. **[角色系统架构](./03_角色系统架构.md)** ⭐⭐⭐⭐
   - 角色类层次结构
   - 接口系统设计
   - 玩家角色 vs 敌人角色
   - 角色类别系统
   - 战斗系统实现

4. **[UI 架构设计](./04_UI架构设计.md)** ⭐⭐⭐⭐
   - MVVM 模式详解
   - WidgetController 系统
   - 数据绑定机制
   - UI 更新流程
   - 实战示例

5. **[重构建议](./05_重构建议.md)** ⭐⭐⭐
   - 渐进式重构方案
   - 架构升级建议
   - 性能优化策略
   - 代码质量提升
   - 实施路线图

---

### 📖 详细文档索引

#### 🚀 快速开始
- **[快速开始指南](./Getting_Started/QuickStart.md)** - 项目配置和运行
- **[项目概览](./Overview/Project_Overview.md)** - 项目介绍和特性

#### 🎯 核心系统
- **[GAS 实现详解](./Core/GAS_Implementation.md)** - GAS 深度实现
- **[能力系统](./Core/Ability_System.md)** - 完整的能力系统文档
- **[系统设计](./Core/System_Design.md)** - 系统设计原则
- **[架构设计](./Core/Architecture.md)** - 架构设计文档
- **[API 参考](./Core/API_Reference.md)** - API 接口文档

#### 🎮 子系统详解
**属性与效果**
- [属性系统](./Systems/Attribute_System.md) - 属性定义和管理
- [属性同步](./Systems/Attribute_Synchronization.md) - 网络同步机制
- [GameplayEffect 系统](./Systems/GameplayEffect_System.md) - 效果系统
- [伤害计算](./Systems/Damage_Calculation.md) - 伤害计算公式
- [Debuff 系统](./Systems/Debuff_System.md) - 负面效果系统

**技能系统**
- [GameplayAbility 系统](./Systems/GameplayAbility_System.md) - 技能系统
- [被动技能系统](./Systems/Passive_Ability_System.md) - 被动技能
- [技能任务系统](./Systems/Ability_Tasks_System.md) - AbilityTasks
- [ModMagCalc 系统](./Systems/ModMagCalc_System.md) - 属性修改计算

**角色与战斗**
- [角色系统](./Systems/Character_System.md) - 角色管理
- [AI 系统](./Systems/AI_System.md) - AI 行为树
- [交互系统](./Systems/Interaction_System.md) - 角色交互

**UI 与输入**
- [UI 系统](./Systems/UI_System.md) - UI 框架
- [MVVM 系统](./Systems/MVVM_System.md) - MVVM 模式
- [输入系统](./Systems/Input_System.md) - 增强输入系统

**游戏框架**
- [游戏框架](./Gameplay/Gameplay_Framework.md) - GameMode、PlayerState 等
- [玩家系统](./Systems/Player_System.md) - 玩家管理
- [检查点系统](./Systems/Checkpoint_System.md) - 存档点系统

**资源与标签**
- [GameplayTags 系统](./Systems/GameplayTags_System.md) - 标签系统
- [GameplayCue 系统](./Systems/GameplayCue_System.md) - 视觉效果提示
- [数据资产系统](./Systems/Data_Assets_System.md) - DataAssets
- [资源管理器](./Systems/Asset_Manager_System.md) - 资源管理
- [资源管理详解](./Systems/Asset_Management_Details.md) - 详细说明
- [资源打包](./Systems/Asset_Packaging.md) - 打包策略

**其他系统**
- [Actor 系统](./Systems/Actor_System.md) - 游戏 Actor

#### ⚔️ 技能详解
- [火球术 (FireBolt)](./Abilities/FireBolt.md) - 投射物技能
- [电击 (Electrocute)](./Abilities/Electrocute.md) - 光束技能
- [奥术碎片 (ArcaneShards)](./Abilities/ArcaneShards.md) - 多重投射物
- [火焰爆炸 (FireBlast)](./Abilities/FireBlast.md) - AOE 技能
- [近战攻击 (MeleeAttack)](./Abilities/MeleeAttack.md) - 近战技能
- [召唤技能 (SummonAbility)](./Abilities/SummonAbility.md) - 召唤系统
- [生命虹吸 (LifeSiphon)](./Abilities/LifeSiphon.md) - 被动技能
- [法力虹吸 (ManaSiphon)](./Abilities/ManaSiphon.md) - 被动技能
- [保护光环 (HaloOfProtection)](./Abilities/HaloOfProtection.md) - 被动技能

#### 📘 操作指南
- [如何添加新技能](./Guides/How_To_Add_New_Ability.md) - 完整的技能创建流程
- [如何添加新角色](./Guides/How_To_Add_New_Character.md) - 角色创建指南
- [如何添加新 AI](./Guides/How_To_Add_New_AI.md) - AI 创建指南
- [如何添加新资源](./Guides/How_To_Add_New_Resource.md) - 资源添加指南
- [如何实现近战技能](./Guides/How_To_Implement_Melee_Ability.md) - 近战技能实现
- [如何实现远程技能](./Guides/How_To_Implement_Ranged_Ability.md) - 远程技能实现
- [如何实现图腾技能](./Guides/How_To_Implement_Totem_Ability.md) - 图腾技能实现
- [如何分包打包](./Guides/How_To_Split_Package_Into_Chunks.md) - 资源分包策略

#### 🔧 参考资料
- [架构优化](./Reference/Architecture_Optimization.md) - 架构优化建议
- [文档索引](./Documentation_Index.md) - 完整文档索引

---

## 🎯 快速开始

### 新手入门路径
```
1. 阅读 [架构总览](./01_架构总览.md)
   ↓ 了解整体架构
2. 阅读 [快速开始指南](./Getting_Started/QuickStart.md)
   ↓ 配置和运行项目
3. 阅读 [GAS 系统详解](./02_GAS系统详解.md)
   ↓ 理解核心系统
4. 阅读 [角色系统架构](./03_角色系统架构.md)
   ↓ 掌握角色设计
5. 阅读 [UI 架构设计](./04_UI架构设计.md)
   ↓ 学习 UI 模式
6. 根据需要查阅详细文档
```

### 按角色阅读
| 角色 | 推荐文档 | 说明 |
|------|---------|------|
| **项目负责人** | 核心架构文档 → 参考资料 | 全面了解架构和优化方向 |
| **游戏程序员** | 核心架构文档 → 子系统详解 → 操作指南 | 重点关注 GAS 和角色系统 |
| **UI 程序员** | 架构总览 → UI 架构设计 → UI 系统 → MVVM 系统 | 重点关注 MVVM 架构 |
| **技术美术** | 架构总览 → 角色系统 → GameplayCue 系统 | 了解角色和特效系统 |
| **策划** | 快速开始 → 技能详解 → 操作指南 | 了解技能配置和数据设计 |
| **新成员** | 快速开始 → 核心架构文档 | 先了解整体再深入细节 |

### 按任务阅读
| 任务 | 推荐文档路径 |
|------|-------------|
| **添加新技能** | GAS 系统详解 → 技能详解 → 如何添加新技能 |
| **添加新角色** | 角色系统架构 → 角色系统 → 如何添加新角色 |
| **修改 UI** | UI 架构设计 → UI 系统 → MVVM 系统 |
| **优化性能** | 重构建议 → 架构优化 |
| **理解伤害计算** | GAS 系统详解 → 伤害计算 → Damage_Calculation.md |
| **实现被动技能** | GAS 系统详解 → 被动技能系统 |

---

## 📊 项目概览

### 核心特性
- ✅ **完整的 GAS 集成** - 技能、属性、效果、标签系统
- ✅ **MVVM UI 架构** - 数据驱动的用户界面
- ✅ **网络多人支持** - 属性复制、RPC 调用
- ✅ **数据驱动设计** - DataAssets 和 DataTables 配置
- ✅ **模块化架构** - 清晰的职责分离
- ✅ **接口解耦** - 基于接口的多态设计

### 技术栈
```cpp
// 核心依赖
- Unreal Engine 5.7
- GameplayAbilities (GAS)
- EnhancedInput
- Niagara (粒子系统)
- UMG (UI 系统)
- AIModule (AI 系统)
```

### 项目规模
```
总代码文件数: 150+
核心模块数: 12
技能类型数: 5+
属性数量: 20+
核心文档: 5 个
详细文档: 80+ 个
```

---

## 🏗️ 架构图

### 整体架构
```
┌─────────────────────────────────────┐
│      表现层 (Presentation)          │  ← UI、Widget、HUD
├─────────────────────────────────────┤
│      应用层 (Application)           │  ← WidgetController、ViewModel
├─────────────────────────────────────┤
│      领域层 (Domain)                │  ← GAS、Character、Combat
├─────────────────────────────────────┤
│      基础设施层 (Infrastructure)    │  ← GameMode、PlayerState、Input
├─────────────────────────────────────┤
│      数据层 (Data)                  │  ← DataAssets、SaveGame
└─────────────────────────────────────┘
```

### 模块划分
```
Aura/
├── AbilitySystem/        (35%) - GAS 核心
│   ├── Abilities/
│   ├── AbilityTasks/
│   ├── AsyncTasks/
│   ├── Data/
│   ├── Debuff/
│   ├── ExecCalc/
│   ├── ModMagCalc/
│   └── Passive/
├── Character/            (20%) - 角色系统
│   ├── AuraCharacterBase
│   ├── AuraCharacter
│   └── AuraEnemy
├── UI/                   (20%) - 用户界面
│   ├── HUD/
│   ├── ViewModel/
│   ├── Widget/
│   └── WidgetController/
├── Player/               (10%) - 玩家相关
│   ├── AuraPlayerController
│   └── AuraPlayerState
├── Game/                 (5%)  - 游戏框架
│   ├── AuraGameMode
│   ├── AuraGameInstance
│   └── LoadScreenSaveGame
├── AI/                   (5%)  - AI 系统
│   ├── AuraAIController
│   ├── BTService_FindNearestPlayer
│   └── BTTask_Attack
└── Actor/                (5%)  - 游戏 Actor
    ├── AuraProjectile
    ├── AuraEffectActor
    └── PointCollection
```

---

## 🔍 核心概念速查

### GAS 核心组件
| 组件 | 说明 | 核心文档 | 详细文档 |
|------|------|---------|---------|
| **AbilitySystemComponent** | GAS 核心，管理技能和效果 | [02_GAS系统详解](./02_GAS系统详解.md#核心组件) | [GAS_Implementation](./Core/GAS_Implementation.md) |
| **AttributeSet** | 属性集合（生命、法力等） | [02_GAS系统详解](./02_GAS系统详解.md#属性系统) | [Attribute_System](./Systems/Attribute_System.md) |
| **GameplayAbility** | 技能基类 | [02_GAS系统详解](./02_GAS系统详解.md#技能系统) | [GameplayAbility_System](./Systems/GameplayAbility_System.md) |
| **GameplayEffect** | 效果（Buff、Debuff、伤害） | [02_GAS系统详解](./02_GAS系统详解.md#效果系统) | [GameplayEffect_System](./Systems/GameplayEffect_System.md) |
| **GameplayTags** | 标签系统 | [02_GAS系统详解](./02_GAS系统详解.md#标签系统) | [GameplayTags_System](./Systems/GameplayTags_System.md) |

### 角色系统
| 类 | 说明 | 核心文档 | 详细文档 |
|------|------|---------|---------|
| **AAuraCharacterBase** | 角色基类 | [03_角色系统架构](./03_角色系统架构.md#auracharacterbase) | [Character_System](./Systems/Character_System.md) |
| **AAuraCharacter** | 玩家角色 | [03_角色系统架构](./03_角色系统架构.md#auracharacter) | [Player_System](./Systems/Player_System.md) |
| **AAuraEnemy** | 敌人角色 | [03_角色系统架构](./03_角色系统架构.md#auraenemy) | [AI_System](./Systems/AI_System.md) |
| **ICombatInterface** | 战斗接口 | [03_角色系统架构](./03_角色系统架构.md#接口系统) | [Interaction_System](./Systems/Interaction_System.md) |

### UI 系统
| 类 | 说明 | 核心文档 | 详细文档 |
|------|------|---------|---------|
| **UAuraWidgetController** | WidgetController 基类 | [04_UI架构设计](./04_UI架构设计.md#widgetcontroller-系统) | [UI_System](./Systems/UI_System.md) |
| **UOverlayWidgetController** | 主界面控制器 | [04_UI架构设计](./04_UI架构设计.md#overlaywidgetcontroller) | [MVVM_System](./Systems/MVVM_System.md) |
| **UAttributeMenuWidgetController** | 属性菜单控制器 | [04_UI架构设计](./04_UI架构设计.md#attributemenuwidgetcontroller) | [UI_System](./Systems/UI_System.md) |
| **USpellMenuWidgetController** | 技能菜单控制器 | [04_UI架构设计](./04_UI架构设计.md#spellmenuwidgetcontroller) | [UI_System](./Systems/UI_System.md) |

---

## 🎨 设计模式

| 模式 | 应用场景 | 核心文档 | 详细文档 |
|------|---------|---------|---------|
| **MVVM** | UI 架构 | [04_UI架构设计](./04_UI架构设计.md) | [MVVM_System](./Systems/MVVM_System.md) |
| **单例模式** | GameplayTags 管理 | [02_GAS系统详解](./02_GAS系统详解.md#标签系统) | [GameplayTags_System](./Systems/GameplayTags_System.md) |
| **工厂模式** | 技能创建 | [02_GAS系统详解](./02_GAS系统详解.md#技能系统) | [GameplayAbility_System](./Systems/GameplayAbility_System.md) |
| **观察者模式** | 事件委托 | [01_架构总览](./01_架构总览.md#设计模式应用) | [Attribute_Synchronization](./Systems/Attribute_Synchronization.md) |
| **策略模式** | 伤害计算 | [02_GAS系统详解](./02_GAS系统详解.md#效果系统) | [Damage_Calculation](./Systems/Damage_Calculation.md) |
| **命令模式** | 输入处理 | [05_重构建议](./05_重构建议.md#命令模式封装操作) | [Input_System](./Systems/Input_System.md) |

---

## 🚀 优化建议

### 性能优化
- **对象池** - 减少投射物创建销毁开销 → [05_重构建议](./05_重构建议.md#对象池系统)
- **异步加载** - 避免同步加载卡顿 → [05_重构建议](./05_重构建议.md#异步资源加载) | [Asset_Management_Details](./Systems/Asset_Management_Details.md)
- **网络优化** - 条件复制、批量 RPC → [05_重构建议](./05_重构建议.md#网络复制优化) | [Attribute_Synchronization](./Systems/Attribute_Synchronization.md)

### 架构优化
- **事件总线** - 减少组件耦合 → [05_重构建议](./05_重构建议.md#引入事件总线系统)
- **服务定位器** - 统一管理全局服务 → [05_重构建议](./05_重构建议.md#服务定位器模式)
- **配置化技能** - 减少 C++ 类数量 → [05_重构建议](./05_重构建议.md#配置化技能系统)
- **架构优化详解** → [Architecture_Optimization](./Reference/Architecture_Optimization.md)

---

## 📝 常见问题

### Q1: 如何添加新技能？
**A**: 
- **快速参考**: [02_GAS系统详解 - 技能系统](./02_GAS系统详解.md#技能系统)
- **详细指南**: [如何添加新技能](./Guides/How_To_Add_New_Ability.md)
- **技能示例**: [技能文档目录](./Abilities/)

### Q2: 如何添加新属性？
**A**: 
- **快速参考**: [02_GAS系统详解 - 属性系统](./02_GAS系统详解.md#属性系统)
- **详细文档**: [属性系统](./Systems/Attribute_System.md)
- **同步机制**: [属性同步](./Systems/Attribute_Synchronization.md)

### Q3: 如何创建新的 UI 界面？
**A**: 
- **快速参考**: [04_UI架构设计 - 实战示例](./04_UI架构设计.md#实战示例)
- **详细文档**: [UI 系统](./Systems/UI_System.md)
- **MVVM 模式**: [MVVM 系统](./Systems/MVVM_System.md)

### Q4: 如何优化性能？
**A**: 
- **快速参考**: [05_重构建议 - 性能优化](./05_重构建议.md#性能优化建议)
- **详细文档**: [架构优化](./Reference/Architecture_Optimization.md)
- **资源管理**: [资源管理详解](./Systems/Asset_Management_Details.md)

### Q5: 项目可以重构吗？
**A**: 
- **重构方案**: [05_重构建议](./05_重构建议.md)
- **架构优化**: [Architecture_Optimization](./Reference/Architecture_Optimization.md)
- **建议**: 采用渐进式重构，优先解耦核心系统

### Q6: 如何理解伤害计算？
**A**:
- **快速参考**: [02_GAS系统详解 - 效果系统](./02_GAS系统详解.md#效果系统)
- **详细文档**: [伤害计算](./Systems/Damage_Calculation.md)

### Q7: 如何实现被动技能？
**A**:
- **快速参考**: [02_GAS系统详解 - 技能系统](./02_GAS系统详解.md#技能系统)
- **详细文档**: [被动技能系统](./Systems/Passive_Ability_System.md)
- **示例**: [生命虹吸](./Abilities/LifeSiphon.md)

---

## 🔗 外部资源

### 官方文档
- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [Gameplay Ability System](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Enhanced Input](https://docs.unrealengine.com/5.7/en-US/enhanced-input-in-unreal-engine/)

### 社区资源
- [GAS Documentation (Tranek)](https://github.com/tranek/GASDocumentation)
- [Unreal Slackers Discord](https://unrealslackers.org/)

---

## 📞 联系方式

### 文档维护
- **负责人**: 架构团队
- **更新频率**: 重大架构变更时更新
- **反馈渠道**: 项目 Issue 或内部沟通群

### 贡献指南
1. 发现文档错误或过时内容，请提交 Issue
2. 有改进建议，欢迎提交 PR
3. 重大架构变更，请更新相关文档

---

## 📅 更新日志

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2026-01-24 | 初始版本，完整架构文档，整合新旧文档体系 |

---

## 📖 完整文档列表

### 核心架构文档
1. [01_架构总览.md](./01_架构总览.md) - 项目整体架构
2. [02_GAS系统详解.md](./02_GAS系统详解.md) - Gameplay Ability System
3. [03_角色系统架构.md](./03_角色系统架构.md) - 角色系统设计
4. [04_UI架构设计.md](./04_UI架构设计.md) - MVVM UI 架构
5. [05_重构建议.md](./05_重构建议.md) - 架构优化方案

### 详细文档（按目录）
- **[Abilities/](./Abilities/)** - 各技能详细文档（10个）
- **[Core/](./Core/)** - 核心系统文档（6个）
- **[Gameplay/](./Gameplay/)** - 游戏框架文档（1个）
- **[Getting_Started/](./Getting_Started/)** - 快速开始（2个）
- **[Guides/](./Guides/)** - 操作指南（9个）
- **[Overview/](./Overview/)** - 项目概览（1个）
- **[Reference/](./Reference/)** - 参考资料（2个）
- **[Systems/](./Systems/)** - 子系统文档（26个）

---

**感谢阅读！祝开发顺利！** 🎮✨