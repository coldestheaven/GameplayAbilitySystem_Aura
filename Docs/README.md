# Aura Gameplay Ability System 项目文档

## 项目概述

Aura 是一个基于 Unreal Engine 5.7 的完整 Gameplay Ability System (GAS) 实现项目。该项目展示了如何在 UE5 中构建一个功能完整的 ARPG 风格游戏系统，包括能力系统、属性系统、UI 系统、AI 系统等。

> 📖 **完整项目总览**: 查看 [项目总览](./Project_Overview.md) 了解整个项目的架构和系统概览

## 文档索引

> 📑 **完整文档索引**: 查看 [文档完整索引](./Documentation_Index.md) 获取所有文档的详细列表和导航

### 入门文档
- [项目总览](./Project_Overview.md) - 项目整体架构和系统概览
- [快速开始](./Getting_Started/README.md) - 项目设置和开发指南

### 核心文档
- [核心文档](./Core/README.md) - 架构、设计和 API 文档
  - [架构文档](./Core/Architecture.md) - 系统架构和设计模式
  - [系统设计](./Core/System_Design.md) - 各系统详细设计说明
  - [API 参考](./Core/API_Reference.md) - 核心类和 API 文档
  - [技能系统](./Core/Ability_System.md) - 技能系统详细分析文档
  - [GAS 实现文档](./Core/GAS_Implementation.md) - Gameplay Ability System 完整实现文档
  - [架构优化建议](./Architecture_Optimization.md) - 性能优化和架构改进建议

### 系统文档
- [子系统详细文档](./Systems/README.md) - 各子系统的详细技术文档
  - **核心系统**: 属性、属性同步、伤害计算、Debuff、数据资产
  - **GAS 核心组件**: GameplayTags、GameplayEffect、GameplayAbility、GameplayCue、被动技能
  - **角色和玩家系统**: 角色、玩家状态
  - **交互系统**: UI、输入、AI、Actor、交互、Checkpoint
  - **辅助系统**: MVVM、ModMagCalc、Ability Tasks、Asset Manager

### 技能文档
- [技能详细文档](./Abilities/README.md) - 每个技能的详细说明
  - **主动技能**: FireBolt、FireBlast、Electrocute、ArcaneShards、MeleeAttack、SummonAbility
  - **被动技能**: HaloOfProtection、LifeSiphon、ManaSiphon

### 实现指南
- [实现指南](./Guides/README.md) - 技能和功能实现指南
  - **通用指南**: 如何添加新技能、新角色、新AI、新资源
  - **技能类型指南**: 近战、远程、图腾
  - **打包和分发指南**: 如何将包分为两个部分

### 游戏玩法
- [Gameplay 框架](./Gameplay/README.md) - Gameplay 框架详细文档

## 核心特性

### 1. Gameplay Ability System (GAS)
- 完整的能力系统实现
- 自定义 AbilitySystemComponent
- 伤害计算系统
- Debuff 系统（Burn, Stun, Arcane, Physical）
- 被动能力系统

### 2. 属性系统
- 主属性：Strength, Intelligence, Resilience, Vigor
- 次属性：Armor, CriticalHit, Health/Mana Regeneration 等
- 抗性系统：Fire, Lightning, Arcane, Physical
- 生命值系统：Health, Mana, MaxHealth, MaxMana

### 3. 角色系统
- 多职业支持（Warrior, Ranger, Elementalist）
- 玩家角色和敌人角色
- 战斗接口系统
- 死亡和溶解效果

### 4. UI 系统
- MVVM 架构（Model-View-ViewModel）
- Widget Controller 模式
- 属性菜单、法术菜单、覆盖层 UI

### 5. AI 系统
- 行为树集成
- AI 控制器
- 敌人行为

### 6. 数据驱动
- Data Tables 配置
- 能力信息数据资产
- 职业信息数据资产

## 技术栈

- **引擎版本**: Unreal Engine 5.7
- **编程语言**: C++
- **主要插件**:
  - GameplayAbilities
  - MotionWarping
  - ModelViewViewModel
  - EnhancedInput

## 项目结构

```
Source/Aura/
├── AbilitySystem/        # 能力系统核心
│   ├── Abilities/       # 能力实现
│   ├── Data/            # 数据资产
│   ├── ExecCalc/        # 执行计算器
│   └── ModMagCalc/      # 修正量计算器
├── Character/           # 角色类
├── Player/              # 玩家系统
├── UI/                  # UI 系统
├── AI/                  # AI 系统
├── Interaction/         # 交互接口
└── Game/                # 游戏系统
```

## 快速导航

### 🚀 新手入门路径
1. [项目总览](./Project_Overview.md) - 了解项目整体架构
2. [快速开始指南](./Getting_Started/QuickStart.md) - 从这里开始
3. [架构文档](./Core/Architecture.md) - 理解系统架构
4. [系统设计](./Core/System_Design.md) - 了解设计思路

### 🏗️ 核心文档
- [架构文档](./Core/Architecture.md) - 系统架构和设计模式
- [系统设计](./Core/System_Design.md) - 各系统详细设计说明
- [GAS 实现文档](./Core/GAS_Implementation.md) - GAS 完整实现
- [API 参考](./Core/API_Reference.md) - 核心类和 API 文档
- [技能系统](./Core/Ability_System.md) - 技能系统详解
- [架构优化建议](./Architecture_Optimization.md) - 性能优化和架构改进建议

### ⚙️ 系统文档
- [子系统文档](./Systems/README.md) - 各子系统详细技术文档
  - **核心系统**: 属性、属性同步、伤害计算、Debuff、数据资产
  - **GAS 核心组件**: GameplayTags、GameplayEffect、GameplayAbility、GameplayCue、被动技能
  - **角色和玩家系统**: 角色、玩家状态
  - **交互系统**: UI、输入、AI、Actor、交互、Checkpoint
  - **辅助系统**: MVVM、ModMagCalc、Ability Tasks、Asset Manager

### 🎮 技能文档
- [技能详细文档](./Abilities/README.md) - 每个技能的详细说明
  - **主动技能**: FireBolt、FireBlast、Electrocute、ArcaneShards、MeleeAttack、SummonAbility
  - **被动技能**: HaloOfProtection、LifeSiphon、ManaSiphon

### 📖 实现指南
- [实现指南](./Guides/README.md) - 技能和功能实现指南
  - **通用指南**: 如何添加新技能、新角色、新AI、新资源
  - **技能类型指南**: 近战、远程、图腾
  - **打包和分发指南**: 如何将包分为两个部分

### 🎯 游戏玩法
- [Gameplay 框架](./Gameplay/Gameplay_Framework.md) - Gameplay 框架详细文档

## 版权信息

Copyright Druid Mechanics

