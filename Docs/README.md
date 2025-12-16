# Aura Gameplay Ability System 项目文档

## 项目概述

Aura 是一个基于 Unreal Engine 5.7 的完整 Gameplay Ability System (GAS) 实现项目。该项目展示了如何在 UE5 中构建一个功能完整的 ARPG 风格游戏系统，包括能力系统、属性系统、UI 系统、AI 系统等。

## 文档索引

- [架构文档](./Architecture.md) - 系统架构和设计模式
- [API 参考](./API_Reference.md) - 核心类和 API 文档
- [系统设计](./System_Design.md) - 各系统详细设计说明
- [技能系统](./Ability_System.md) - 技能系统详细分析文档
- [技能详细文档](./Abilities/README.md) - 每个技能的详细说明
- [如何添加新技能](./How_To_Add_New_Ability.md) - 添加新技能的完整步骤指南
- [图腾技能实现指南](./How_To_Implement_Totem_Ability.md) - 图腾召唤技能实现指南（范围持续伤害）
- [Gameplay 框架](./Gameplay_Framework.md) - Gameplay 框架详细文档
- [子系统详细文档](./Systems/README.md) - 各子系统的详细技术文档
- [快速开始](./QuickStart.md) - 项目设置和开发指南

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

## 快速链接

- [查看架构文档](./Architecture.md)
- [查看 API 参考](./API_Reference.md)
- [查看系统设计](./System_Design.md)
- [查看技能系统文档](./Ability_System.md)
- [查看 Gameplay 框架文档](./Gameplay_Framework.md)
- [查看子系统详细文档](./Systems/README.md)
- [查看快速开始指南](./QuickStart.md)

## 版权信息

Copyright Druid Mechanics

