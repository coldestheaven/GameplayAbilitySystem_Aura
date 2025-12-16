# 子系统详细文档索引

本文档目录包含项目中各个子系统的详细技术文档。

## 文档列表

### 核心系统

- [属性系统](./Attribute_System.md) - 属性系统的详细实现和设计
- [伤害计算系统](./Damage_Calculation.md) - 伤害计算的详细算法
- [Debuff 系统](./Debuff_System.md) - Debuff 系统的实现机制
- [数据资产系统](./Data_Assets_System.md) - 数据资产系统的配置和使用

### 角色和玩家系统

- [角色系统](./Character_System.md) - 角色基类和角色实现
- [玩家系统](./Player_System.md) - 玩家状态和玩家控制器

### 交互系统

- [UI 系统](./UI_System.md) - UI 系统的 MVVM 架构和实现
- [输入系统](./Input_System.md) - 输入系统的集成和配置
- [AI 系统](./AI_System.md) - AI 系统的行为树和控制器实现
- [Actor 系统](./Actor_System.md) - Actor 系统的效果应用和生成系统
- [交互系统](./Interaction_System.md) - 交互系统的接口和检查点实现
- [Checkpoint 系统](./Checkpoint_System.md) - 检查点和地图入口系统

### 辅助系统

- [MVVM 系统](./MVVM_System.md) - MVVM 架构的加载屏幕实现
- [ModMagCalc 系统](./ModMagCalc_System.md) - 修正量计算器系统
- [Ability Tasks 系统](./Ability_Tasks_System.md) - Ability Tasks 和 Async Tasks
- [Asset Manager 系统](./Asset_Manager_System.md) - 资源管理器系统
- [资产管理详细文档](./Asset_Management_Details.md) - 资产管理的完整细节

## 系统分类

### 核心系统

- **属性系统**: 管理所有游戏属性，包括主属性、次属性、抗性等
- **伤害计算系统**: 处理所有伤害计算逻辑
- **Debuff 系统**: 管理持续伤害和状态效果
- **数据资产系统**: 数据驱动的配置系统

### 角色和玩家系统

- **角色系统**: 角色基类、玩家角色、敌人角色
- **玩家系统**: 玩家状态、玩家控制器、经验值和升级

### 交互系统

- **UI 系统**: MVVM 架构的 UI 实现
- **输入系统**: Enhanced Input 集成和技能输入绑定
- **AI 系统**: 行为树和黑板系统实现的敌人 AI
- **Actor 系统**: 效果应用、敌人生成、投射物系统
- **交互系统**: 接口系统、检查点、高亮系统
- **Checkpoint 系统**: 检查点和地图入口

### 辅助系统

- **MVVM 系统**: MVVM 架构的加载屏幕
- **ModMagCalc 系统**: 属性修正量计算
- **Ability Tasks 系统**: 异步任务和复杂操作
- **Asset Manager 系统**: 资源管理和全局初始化

## 文档特点

每个文档包含：

- 系统概述和架构
- 详细的实现机制
- API 参考
- 代码示例
- 配置指南
- 最佳实践

