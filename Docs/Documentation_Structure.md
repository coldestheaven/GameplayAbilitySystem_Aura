# 文档结构说明

> **更新日期**: 2026-01-24  
> **文档版本**: v1.0

---

## 📚 文档体系概览

Aura 项目采用 **双层文档体系**，既有高层次的架构文档，也有详细的技术文档：

```
Docs/
├── 📘 核心架构文档（5个）      ← 高层次，宏观视角
│   ├── README.md              ← 文档导航中心
│   ├── 01_架构总览.md         ← 整体架构
│   ├── 02_GAS系统详解.md      ← GAS 核心
│   ├── 03_角色系统架构.md     ← 角色系统
│   ├── 04_UI架构设计.md       ← UI 架构
│   └── 05_重构建议.md         ← 优化方案
│
└── 📖 详细技术文档（80+个）    ← 细节层次，微观视角
    ├── Abilities/             ← 各技能详解
    ├── Core/                  ← 核心系统
    ├── Gameplay/              ← 游戏框架
    ├── Getting_Started/       ← 快速开始
    ├── Guides/                ← 操作指南
    ├── Overview/              ← 项目概览
    ├── Reference/             ← 参考资料
    └── Systems/               ← 子系统详解
```

---

## 🎯 两套文档的关系

### 核心架构文档（新）
- **目标读者**: 所有开发者、新成员、项目负责人
- **内容特点**: 
  - ✅ 高层次抽象
  - ✅ 架构设计理念
  - ✅ 系统间关系
  - ✅ 数据流向
  - ✅ 设计模式
- **阅读时间**: 2-4 小时
- **更新频率**: 架构变更时

### 详细技术文档（原有）
- **目标读者**: 具体实现者、深入研究者
- **内容特点**:
  - ✅ 具体实现细节
  - ✅ API 接口说明
  - ✅ 代码示例
  - ✅ 配置说明
  - ✅ 操作步骤
- **阅读时间**: 按需查阅
- **更新频率**: 功能变更时

---

## 📖 推荐阅读路径

### 路径 1：新手入门（推荐）
```
1. README.md
   ↓ 了解文档结构
2. 01_架构总览.md
   ↓ 理解整体架构
3. Getting_Started/QuickStart.md
   ↓ 配置运行项目
4. 02_GAS系统详解.md
   ↓ 掌握核心系统
5. 根据需要查阅详细文档
```

### 路径 2：快速上手
```
1. README.md
   ↓
2. Getting_Started/QuickStart.md
   ↓
3. Overview/Project_Overview.md
   ↓
4. 根据任务查阅相关文档
```

### 路径 3：深入研究
```
1. 核心架构文档（全部）
   ↓
2. Core/ 目录（核心系统）
   ↓
3. Systems/ 目录（子系统）
   ↓
4. Guides/ 目录（操作指南）
```

---

## 🗂️ 目录详解

### 📘 核心架构文档

#### README.md
- **作用**: 文档导航中心
- **内容**: 
  - 文档索引
  - 快速开始指南
  - 按角色/任务推荐阅读
  - 常见问题
- **何时阅读**: 首次接触项目

#### 01_架构总览.md
- **作用**: 项目整体架构
- **内容**:
  - 分层架构设计
  - 模块划分
  - 技术栈
  - 设计模式
  - 数据流向
- **何时阅读**: 理解项目结构时

#### 02_GAS系统详解.md
- **作用**: GAS 核心系统
- **内容**:
  - AbilitySystemComponent
  - AttributeSet
  - GameplayAbility
  - GameplayEffect
  - GameplayTags
  - 网络复制
- **何时阅读**: 开发技能/属性功能时

#### 03_角色系统架构.md
- **作用**: 角色系统设计
- **内容**:
  - 角色类层次
  - 接口系统
  - 玩家 vs 敌人
  - 战斗系统
  - 角色类别
- **何时阅读**: 开发角色相关功能时

#### 04_UI架构设计.md
- **作用**: UI 系统架构
- **内容**:
  - MVVM 模式
  - WidgetController
  - 数据绑定
  - UI 更新流程
- **何时阅读**: 开发 UI 功能时

#### 05_重构建议.md
- **作用**: 架构优化方案
- **内容**:
  - 渐进式重构
  - 性能优化
  - 架构升级
  - 实施路线图
- **何时阅读**: 考虑优化项目时

---

### 📖 详细技术文档

#### Abilities/ (10个文档)
- **作用**: 各技能详细说明
- **内容**: 每个技能的实现细节、配置、使用方法
- **文档列表**:
  - FireBolt.md - 火球术
  - Electrocute.md - 电击
  - ArcaneShards.md - 奥术碎片
  - FireBlast.md - 火焰爆炸
  - MeleeAttack.md - 近战攻击
  - SummonAbility.md - 召唤技能
  - LifeSiphon.md - 生命虹吸
  - ManaSiphon.md - 法力虹吸
  - HaloOfProtection.md - 保护光环
  - README.md - 技能索引
- **何时查阅**: 实现具体技能时

#### Core/ (6个文档)
- **作用**: 核心系统深度文档
- **内容**: GAS、架构、API 等核心内容
- **文档列表**:
  - GAS_Implementation.md - GAS 实现详解
  - Ability_System.md - 能力系统
  - System_Design.md - 系统设计
  - Architecture.md - 架构设计
  - API_Reference.md - API 参考
  - README.md - 核心文档索引
- **何时查阅**: 深入理解核心系统时

#### Systems/ (26个文档)
- **作用**: 各子系统详细说明
- **内容**: 每个子系统的实现、配置、API
- **分类**:
  - **属性与效果** (6个)
    - Attribute_System.md
    - Attribute_Synchronization.md
    - GameplayEffect_System.md
    - Damage_Calculation.md
    - Debuff_System.md
    - ModMagCalc_System.md
  - **技能系统** (3个)
    - GameplayAbility_System.md
    - Passive_Ability_System.md
    - Ability_Tasks_System.md
  - **角色与战斗** (3个)
    - Character_System.md
    - AI_System.md
    - Interaction_System.md
  - **UI 与输入** (3个)
    - UI_System.md
    - MVVM_System.md
    - Input_System.md
  - **游戏框架** (3个)
    - Gameplay_Framework.md (在 Gameplay/)
    - Player_System.md
    - Checkpoint_System.md
  - **资源与标签** (6个)
    - GameplayTags_System.md
    - GameplayCue_System.md
    - Data_Assets_System.md
    - Asset_Manager_System.md
    - Asset_Management_Details.md
    - Asset_Packaging.md
  - **其他** (2个)
    - Actor_System.md
    - README.md
- **何时查阅**: 实现具体子系统功能时

#### Guides/ (9个文档)
- **作用**: 操作指南和教程
- **内容**: 如何添加/实现各种功能
- **文档列表**:
  - How_To_Add_New_Ability.md - 添加新技能
  - How_To_Add_New_Character.md - 添加新角色
  - How_To_Add_New_AI.md - 添加新 AI
  - How_To_Add_New_Resource.md - 添加新资源
  - How_To_Implement_Melee_Ability.md - 实现近战技能
  - How_To_Implement_Ranged_Ability.md - 实现远程技能
  - How_To_Implement_Totem_Ability.md - 实现图腾技能
  - How_To_Split_Package_Into_Chunks.md - 分包打包
  - README.md - 指南索引
- **何时查阅**: 需要添加新功能时

#### Getting_Started/ (2个文档)
- **作用**: 快速开始指南
- **内容**: 项目配置、运行、基础概念
- **文档列表**:
  - QuickStart.md - 快速开始
  - README.md - 入门索引
- **何时查阅**: 首次运行项目时

#### Overview/ (1个文档)
- **作用**: 项目概览
- **内容**: 项目介绍、特性、目标
- **文档列表**:
  - Project_Overview.md - 项目概览
  - README.md - 概览索引
- **何时查阅**: 了解项目背景时

#### Reference/ (2个文档)
- **作用**: 参考资料
- **内容**: 架构优化、故障排除等
- **文档列表**:
  - Architecture_Optimization.md - 架构优化
  - README.md - 参考索引
- **何时查阅**: 需要优化或解决问题时

#### Gameplay/ (1个文档)
- **作用**: 游戏框架文档
- **内容**: GameMode、PlayerState 等
- **文档列表**:
  - Gameplay_Framework.md - 游戏框架
  - README.md - 框架索引
- **何时查阅**: 开发游戏框架功能时

---

## 🔗 文档关联关系

### 核心架构文档 → 详细文档
```
01_架构总览.md
  ├→ Core/Architecture.md
  ├→ Core/System_Design.md
  └→ Overview/Project_Overview.md

02_GAS系统详解.md
  ├→ Core/GAS_Implementation.md
  ├→ Core/Ability_System.md
  ├→ Systems/GameplayAbility_System.md
  ├→ Systems/GameplayEffect_System.md
  ├→ Systems/GameplayTags_System.md
  ├→ Systems/Attribute_System.md
  └→ Systems/Damage_Calculation.md

03_角色系统架构.md
  ├→ Systems/Character_System.md
  ├→ Systems/AI_System.md
  ├→ Systems/Player_System.md
  └→ Systems/Interaction_System.md

04_UI架构设计.md
  ├→ Systems/UI_System.md
  ├→ Systems/MVVM_System.md
  └→ Systems/Input_System.md

05_重构建议.md
  ├→ Reference/Architecture_Optimization.md
  ├→ Systems/Asset_Management_Details.md
  └→ Systems/Attribute_Synchronization.md
```

---

## 📊 文档统计

### 按类型统计
| 类型 | 数量 | 说明 |
|------|------|------|
| 核心架构文档 | 5 | 高层次架构设计 |
| 技能文档 | 10 | 各技能详解 |
| 核心系统文档 | 6 | 核心系统深度文档 |
| 子系统文档 | 26 | 各子系统详解 |
| 操作指南 | 9 | 如何添加/实现功能 |
| 快速开始 | 2 | 入门指南 |
| 项目概览 | 1 | 项目介绍 |
| 参考资料 | 2 | 优化和故障排除 |
| 游戏框架 | 1 | GameMode 等 |
| **总计** | **62** | **不含索引文件** |

### 按内容统计
| 内容类型 | 文档数 | 占比 |
|---------|--------|------|
| GAS 相关 | 20 | 32% |
| 角色/AI | 8 | 13% |
| UI/输入 | 5 | 8% |
| 资源管理 | 6 | 10% |
| 技能详解 | 10 | 16% |
| 操作指南 | 9 | 15% |
| 其他 | 4 | 6% |

---

## 🎯 使用建议

### 1. 首次接触项目
```
README.md → 01_架构总览.md → Getting_Started/QuickStart.md
```

### 2. 开发新功能
```
核心架构文档（相关部分） → Guides/（操作指南） → Systems/（详细文档）
```

### 3. 深入研究
```
核心架构文档（全部） → Core/（核心系统） → Systems/（子系统）
```

### 4. 解决问题
```
README.md（常见问题） → Reference/（参考资料） → 相关详细文档
```

### 5. 优化项目
```
05_重构建议.md → Reference/Architecture_Optimization.md → 相关系统文档
```

---

## 📝 文档维护

### 更新原则
1. **核心架构文档**: 架构变更时更新
2. **详细技术文档**: 功能变更时更新
3. **操作指南**: 流程变化时更新
4. **README**: 文档结构变化时更新

### 更新流程
1. 修改相关文档
2. 更新 README 索引
3. 更新 Documentation_Structure.md
4. 记录 CHANGELOG.md

---

## 🔍 快速查找

### 按关键词查找
| 关键词 | 推荐文档 |
|--------|---------|
| **架构** | 01_架构总览.md, Core/Architecture.md |
| **GAS** | 02_GAS系统详解.md, Core/GAS_Implementation.md |
| **技能** | 02_GAS系统详解.md, Abilities/, Guides/How_To_Add_New_Ability.md |
| **属性** | 02_GAS系统详解.md, Systems/Attribute_System.md |
| **角色** | 03_角色系统架构.md, Systems/Character_System.md |
| **UI** | 04_UI架构设计.md, Systems/UI_System.md |
| **MVVM** | 04_UI架构设计.md, Systems/MVVM_System.md |
| **优化** | 05_重构建议.md, Reference/Architecture_Optimization.md |
| **伤害** | 02_GAS系统详解.md, Systems/Damage_Calculation.md |
| **AI** | 03_角色系统架构.md, Systems/AI_System.md |
| **网络** | 02_GAS系统详解.md, Systems/Attribute_Synchronization.md |
| **资源** | Systems/Asset_Management_Details.md |

---

## 📞 反馈与贡献

如果你发现：
- 📄 文档错误或过时
- 🔗 链接失效
- 📝 内容不清晰
- 💡 有改进建议

请通过以下方式反馈：
1. 提交 Issue
2. 提交 Pull Request
3. 联系文档维护团队

---

**文档结构说明更新日期**: 2026-01-24  
**维护者**: 架构团队