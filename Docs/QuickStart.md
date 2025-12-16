# 快速开始指南

## 项目设置

### 前置要求

- **Unreal Engine**: 5.7 或更高版本
- **Visual Studio**: 2019 或更高版本（Windows）
- **C++ 知识**: 基础 C++ 和 Unreal Engine C++ 知识

### 插件要求

确保以下插件已启用：

- GameplayAbilities
- MotionWarping
- ModelViewViewModel
- EnhancedInput

### 编译项目

1. 右键点击 `Aura.uproject`
2. 选择 "Generate Visual Studio project files"
3. 打开 `Aura.sln`
4. 编译项目（F7 或 Build → Build Solution）

---

## 项目结构理解

### 核心目录

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

### 关键文件

- `AuraGameplayTags.h/cpp`: Gameplay Tags 定义
- `AuraAbilitySystemComponent.h/cpp`: 自定义 ASC
- `AuraAttributeSet.h/cpp`: 属性集
- `AuraCharacterBase.h/cpp`: 角色基类

---

## 开发工作流

### 1. 创建新能力

#### 步骤 1: 创建能力类

```cpp
// MyNewAbility.h
UCLASS()
class AURA_API UMyNewAbility : public UAuraDamageGameplayAbility
{
    GENERATED_BODY()
    
public:
    UMyNewAbility();
    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;
};
```

#### 步骤 2: 实现能力逻辑

```cpp
// MyNewAbility.cpp
void UMyNewAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 实现能力逻辑
    CauseDamage(TargetActor);
    
    EndAbility(...);
}
```

#### 步骤 3: 配置能力信息

1. 打开 `AbilityInfo` 数据资产
2. 添加新的 `FAuraAbilityInfo` 条目
3. 设置 AbilityTag, Icon, LevelRequirement 等

#### 步骤 4: 创建 GameplayEffect

1. 创建 Cost GameplayEffect（法力消耗）
2. 创建 Cooldown GameplayEffect（冷却时间）
3. 创建 Damage GameplayEffect（伤害效果）

#### 步骤 5: 添加到角色

在角色的 `StartupAbilities` 数组中添加新能力类。

---

### 2. 添加新属性

#### 步骤 1: 在 AttributeSet 中添加属性

```cpp
// AuraAttributeSet.h
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_NewAttribute, Category = "New Attributes")
FGameplayAttributeData NewAttribute;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, NewAttribute);

UFUNCTION()
void OnRep_NewAttribute(const FGameplayAttributeData& OldNewAttribute) const;
```

#### 步骤 2: 实现复制

```cpp
// AuraAttributeSet.cpp
void UAuraAttributeSet::GetLifetimeReplicatedProps(...)
{
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, NewAttribute, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::OnRep_NewAttribute(...)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, NewAttribute, OldNewAttribute);
}
```

#### 步骤 3: 添加 GameplayTag

在 `AuraGameplayTags.h` 中添加：

```cpp
FGameplayTag Attributes_New_NewAttribute;
```

在 `AuraGameplayTags.cpp` 中初始化。

#### 步骤 4: 创建初始化 GameplayEffect

创建 GameplayEffect 来初始化新属性的值。

---

### 3. 创建 Widget Controller

#### 步骤 1: 创建 Widget Controller 类

```cpp
// MyWidgetController.h
UCLASS()
class AURA_API UMyWidgetController : public UAuraWidgetController
{
    GENERATED_BODY()
    
public:
    virtual void BroadcastInitialValues() override;
    virtual void BindCallbacksToDependencies() override;
    
    // 委托
    UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
    FOnPlayerStatChangedSignature OnMyStatChanged;
};
```

#### 步骤 2: 实现初始化和绑定

```cpp
// MyWidgetController.cpp
void UMyWidgetController::BroadcastInitialValues()
{
    // 广播初始值
    OnMyStatChanged.Broadcast(CurrentValue);
}

void UMyWidgetController::BindCallbacksToDependencies()
{
    // 绑定到属性变化
    AuraAttributeSet->OnMyAttributeChanged.AddLambda([this](const FOnAttributeChangeData& Data)
    {
        OnMyStatChanged.Broadcast(Data.NewValue);
    });
}
```

#### 步骤 3: 在 HUD 中注册

```cpp
// AuraHUD.h
UPROPERTY()
TObjectPtr<UMyWidgetController> MyWidgetController;

UMyWidgetController* GetMyWidgetController(const FWidgetControllerParams& WCParams);
```

---

## 调试技巧

### 1. 查看 Gameplay Tags

在编辑器中：
- Window → Gameplay Tag Editor
- 查看所有已注册的 Gameplay Tags

### 2. 调试能力系统

- 使用 `GASDocumentation` 插件（如果可用）
- 在 `UAuraAbilitySystemComponent` 中添加日志
- 使用断点调试能力激活流程

### 3. 查看属性值

在蓝图中：
- 使用 `Get Attribute Value` 节点
- 查看 AttributeSet 的属性值

### 4. 网络调试

- 使用 `Net PIE` 测试多人游戏
- 查看网络复制日志
- 使用 `stat net` 查看网络统计

---

## 常见问题

### Q: 能力无法激活？

**A**: 检查以下几点：
1. 能力是否已添加到角色的 `StartupAbilities`
2. InputTag 是否正确设置
3. Cost 和 Cooldown GameplayEffect 是否正确配置
4. 能力状态是否为 `Equipped`

### Q: 属性值不正确？

**A**: 检查以下几点：
1. 初始化 GameplayEffect 是否正确应用
2. 属性是否在网络复制中正确设置
3. MMC 计算是否正确

### Q: UI 不更新？

**A**: 检查以下几点：
1. WidgetController 是否正确初始化
2. 委托是否正确绑定
3. `BroadcastInitialValues()` 和 `BindCallbacksToDependencies()` 是否被调用

### Q: 伤害计算不正确？

**A**: 检查以下几点：
1. `ExecCalc_Damage` 是否正确配置
2. 伤害类型标签是否正确
3. 抗性计算是否正确

---

## 最佳实践

### 1. 代码组织

- 保持类职责单一
- 使用接口进行解耦
- 合理使用委托和事件

### 2. 性能优化

- 避免在 Tick 中进行复杂计算
- 使用对象池管理频繁创建的对象
- 合理使用网络复制

### 3. 数据驱动

- 尽可能使用数据资产而非硬编码
- 使用 Data Tables 存储配置数据
- 通过编辑器配置而非代码

### 4. 网络同步

- 服务器权威原则
- 合理使用 RPC
- 避免不必要的网络复制

---

## 学习资源

### 官方文档

- [Unreal Engine Gameplay Ability System](https://docs.unrealengine.com/en-US/Gameplay/GameplayAbilitySystem/)
- [Unreal Engine C++ Programming](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/)

### 推荐阅读

- GAS 文档和示例项目
- Unreal Engine 官方示例
- 社区教程和文章

---

## 下一步

1. 阅读 [架构文档](./Architecture.md) 了解系统设计
2. 查看 [API 参考](./API_Reference.md) 了解核心 API
3. 参考 [系统设计文档](./System_Design.md) 了解各系统细节
4. 开始修改和扩展项目

---

## 贡献指南

### 代码规范

- 遵循 Unreal Engine C++ 编码规范
- 使用有意义的变量和函数名
- 添加必要的注释

### 提交更改

1. 创建功能分支
2. 实现更改
3. 测试更改
4. 提交 Pull Request

---

## 支持

如有问题，请：

1. 查看文档
2. 搜索已有问题
3. 创建新问题并详细描述

---

**祝开发愉快！**

