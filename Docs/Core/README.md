# 核心文档

本目录包含项目的核心架构、设计和 API 文档。

## 文档列表

- [架构文档](./Architecture.md) - 系统架构和设计模式
  - 核心架构层次
  - Gameplay Ability System 架构
  - 角色系统架构
  - UI 系统架构（MVVM）
  - 数据驱动架构

- [系统设计](./System_Design.md) - 各系统详细设计说明
  - 能力系统设计
  - 属性系统设计
  - UI 系统设计
  - 输入系统设计
  - 伤害计算系统设计

- [API 参考](./API_Reference.md) - 核心类和 API 文档
  - AbilitySystemComponent API
  - AttributeSet API
  - GameplayAbility API
  - WidgetController API
  - 工具类和辅助函数

- [技能系统](./Ability_System.md) - 技能系统详细分析文档
  - 技能基类体系
  - 技能类型详解
  - 技能激活流程
  - GameplayEffect 配置
  - 技能描述系统

- [GAS 实现文档](./GAS_Implementation.md) - Gameplay Ability System 完整实现文档
  - GAS 架构和核心组件
  - GameplayTags、AttributeSet、GameplayEffect 实现
  - GameplayAbility 和 GameplayCue 实现
  - 伤害计算系统和 Debuff 系统
  - 被动技能系统
  - 网络复制和最佳实践

- [GAS 实现文档](./GAS_Implementation.md) - Gameplay Ability System 完整实现文档
  - GAS 架构和核心组件
  - GameplayTags、AttributeSet、GameplayEffect 实现
  - GameplayAbility 和 GameplayCue 实现
  - 伤害计算系统和 Debuff 系统
  - 被动技能系统
  - 网络复制和最佳实践

## 阅读顺序建议

### 新手路径
1. **快速开始**: [快速开始指南](../Getting_Started/QuickStart.md)
2. **项目总览**: [项目总览](../Project_Overview.md)
3. **架构理解**: [架构文档](./Architecture.md)
4. **系统设计**: [系统设计](./System_Design.md)

### 开发者路径
1. **GAS 实现**: [GAS 实现文档](./GAS_Implementation.md) - 了解 GAS 完整实现
2. **API 参考**: [API 参考](./API_Reference.md) - 开发参考
3. **技能系统**: [技能系统](./Ability_System.md) - 技能系统详解
4. **子系统文档**: [子系统文档](../Systems/README.md) - 具体系统实现

### 技能开发者路径
1. **技能系统**: [技能系统](./Ability_System.md) - 系统架构
2. **实现指南**: [如何添加新技能](../Guides/How_To_Add_New_Ability.md) - 通用指南
3. **技能文档**: [技能详细文档](../Abilities/README.md) - 现有技能参考

## 相关文档

- [子系统详细文档](../Systems/README.md) - 各子系统的技术实现细节
- [技能详细文档](../Abilities/README.md) - 每个技能的详细说明
- [实现指南](../Guides/README.md) - 技能和功能实现指南
- [游戏玩法文档](../Gameplay/README.md) - Gameplay 框架文档

