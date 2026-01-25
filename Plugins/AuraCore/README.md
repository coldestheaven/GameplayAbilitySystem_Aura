# Aura Core Plugin

## 📋 概述

Aura Core 是 Aura 项目的核心插件，提供基础功能和接口。这是整个插件架构的基础层，所有其他插件都依赖于它。

## ✨ 功能

### 核心接口

- **CombatInterface** - 战斗系统接口
  - 角色等级管理
  - 战斗插槽位置
  - 攻击动画
  - 死亡处理
  - 伤害回调

- **EnemyInterface** - 敌人接口
  - 战斗目标管理

- **HighlightInterface** - 高亮接口
  - Actor 高亮显示
  - 移动目标位置

- **PlayerInterface** - 玩家接口
  - 经验值系统
  - 等级提升
  - 属性点管理
  - 技能点管理
  - 存档进度

- **SaveInterface** - 存档接口
  - 加载 Actor
  - 变换加载控制

- **PoolableObject** - 对象池接口
  - 对象获取回调
  - 对象归还回调
  - 池化状态检查

### 核心系统

- **GameplayTags** - 游戏标签系统
  - 属性标签
  - 输入标签
  - 伤害类型标签
  - 技能标签
  - 战斗插槽标签

- **AbilityTypes** - 技能类型定义
  - 伤害效果参数
  - 游戏效果上下文
  - 网络序列化支持

- **AssetManager** - 资产管理器
  - 初始化加载
  - GameplayTags 初始化
  - AbilitySystemGlobals 初始化

### 对象池系统

- **ObjectPoolSubsystem** - 对象池子系统
  - 自动对象复用
  - 动态池扩展
  - 预热机制
  - 统计信息

### 事件总线系统

- **AuraEventBus** - 事件总线
  - 解耦组件通信
  - C++ 模板订阅
  - 蓝图事件发布
  - 自动生命周期管理

- **AuraEventTypes** - 事件类型定义
  - 属性变化事件
  - 技能激活事件
  - 伤害事件
  - 经验值事件
  - 等级提升事件
  - 角色死亡事件

## 🔧 依赖

### 引擎模块

- Core
- CoreUObject
- Engine
- InputCore
- GameplayTags
- GameplayTasks
- GameplayAbilities
- Niagara

## 📦 使用方法

### 在你的模块中使用

在你的模块的 `Build.cs` 文件中添加依赖：

```csharp
PublicDependencyModuleNames.AddRange(new string[] { 
    "AuraCore"
});
```

### 在代码中使用接口

```cpp
#include "Interaction/CombatInterface.h"

// 实现接口
class YOURGAME_API AYourCharacter : public ACharacter, public ICombatInterface
{
    GENERATED_BODY()
    
public:
    // 实现接口方法
    virtual int32 GetPlayerLevel_Implementation() override;
    virtual void Die(const FVector& DeathImpulse) override;
    // ...
};
```

## 📁 目录结构

```
AuraCore/
├── AuraCore.uplugin          # 插件描述文件
├── README.md                 # 本文档
├── Resources/
│   └── Icon128.png           # 插件图标
├── Source/
│   └── AuraCore/
│       ├── AuraCore.Build.cs # 构建配置
│       ├── Private/          # 私有实现
│       │   ├── AuraCoreModule.cpp
│       │   ├── AuraAssetManager.cpp
│       │   ├── GameplayTags/
│       │   │   └── AuraGameplayTags.cpp
│       │   ├── ObjectPool/
│       │   │   └── ObjectPoolSubsystem.cpp
│       │   └── EventSystem/
│       │       └── AuraEventBus.cpp
│       └── Public/           # 公共接口
│           ├── AuraCoreModule.h
│           ├── AuraAssetManager.h
│           ├── AuraAbilityTypes.h
│           ├── Interaction/  # 接口定义
│           │   ├── CombatInterface.h
│           │   ├── EnemyInterface.h
│           │   ├── HighlightInterface.h
│           │   ├── PlayerInterface.h
│           │   └── SaveInterface.h
│           ├── GameplayTags/
│           │   └── AuraGameplayTags.h
│           ├── ObjectPool/
│           │   ├── PoolableObject.h
│           │   └── ObjectPoolSubsystem.h
│           └── EventSystem/
│               ├── AuraEventTypes.h
│               └── AuraEventBus.h
└── Content/                  # 蓝图资源（可选）
```

## 🚀 版本历史

### v1.0 (2026-01-25)

- ✅ 初始版本
- ✅ 迁移 5 个核心接口
- ✅ 配置插件基础架构

## 📚 相关文档

- [模块化插件架构-实施指南](../../Docs/模块化插件架构-实施指南.md)
- [重构计划文档](../../Docs/重构计划文档.md)

## 🤝 贡献

这是 Aura 项目的内部插件，遵循项目的编码规范和提交流程。

## 📄 许可

Copyright Druid Mechanics
