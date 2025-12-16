# AI 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [AI 控制器](#ai-控制器)
3. [敌人角色](#敌人角色)
4. [行为树系统](#行为树系统)
5. [黑板系统](#黑板系统)
6. [目标查找服务](#目标查找服务)
7. [攻击任务](#攻击任务)
8. [AI 状态管理](#ai-状态管理)
9. [高亮系统](#高亮系统)
10. [生命值条系统](#生命值条系统)
11. [配置指南](#配置指南)
12. [使用示例](#使用示例)

---

## 系统概述

AI 系统使用 Unreal Engine 的行为树和黑板系统来控制敌人行为，提供了目标查找、攻击、状态管理等核心功能。

### 核心组件

- **AAuraAIController**: AI 控制器，管理行为树和黑板
- **AAuraEnemy**: 敌人角色类
- **UBTService_FindNearestPlayer**: 查找最近玩家的服务
- **UBTTask_Attack**: 攻击任务

### 系统特点

- ✅ 行为树集成
- ✅ 黑板数据管理
- ✅ 目标查找和追踪
- ✅ 攻击行为
- ✅ 状态响应（受击、眩晕）
- ✅ 高亮系统
- ✅ 生命值条显示

---

## AI 控制器

### AAuraAIController

AI 控制器继承自 `AAIController`，负责管理行为树和黑板。

#### 类定义

```cpp
UCLASS()
class AURA_API AAuraAIController : public AAIController
{
    GENERATED_BODY()
    
public:
    AAuraAIController();
    
protected:
    UPROPERTY()
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
```

#### 初始化

```cpp
AAuraAIController::AAuraAIController()
{
    // 创建黑板组件
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
    check(Blackboard);
    
    // 创建行为树组件
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
    check(BehaviorTreeComponent);
}
```

#### 组件说明

- **BlackboardComponent**: 存储 AI 状态数据（目标、距离、状态标志等）
- **BehaviorTreeComponent**: 执行行为树逻辑

---

## 敌人角色

### AAuraEnemy

敌人角色类，继承自 `AAuraCharacterBase`，实现 `IEnemyInterface` 和 `IHighlightInterface`。

#### 核心属性

```cpp
// AI 相关
UPROPERTY(EditAnywhere, Category = "AI")
TObjectPtr<UBehaviorTree> BehaviorTree;

UPROPERTY()
TObjectPtr<AAuraAIController> AuraAIController;

// 战斗相关
UPROPERTY(BlueprintReadWrite, Category = "Combat")
TObjectPtr<AActor> CombatTarget;

UPROPERTY(BlueprintReadOnly, Category = "Combat")
bool bHitReacting = false;

// 等级和生命值
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
int32 Level = 1;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UWidgetComponent> HealthBar;

// 生命周期
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
float LifeSpan = 5.f;
```

#### 构造函数

```cpp
AAuraEnemy::AAuraEnemy()
{
    // 设置碰撞响应
    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    
    // 创建 AbilitySystemComponent
    AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
    
    // 禁用控制器旋转
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    
    // 创建 AttributeSet
    AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
    
    // 创建生命值条
    HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
    HealthBar->SetupAttachment(GetRootComponent());
    
    // 设置自定义深度（用于高亮）
    GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
    Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
    
    // 设置基础移动速度
    BaseWalkSpeed = 250.f;
}
```

#### PossessedBy

当 AI 控制器控制敌人时调用：

```cpp
void AAuraEnemy::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // 只在服务器端执行
    if (!HasAuthority()) return;
    
    // 获取 AI 控制器
    AuraAIController = Cast<AAuraAIController>(NewController);
    
    // 初始化黑板
    AuraAIController->GetBlackboardComponent()->InitializeBlackboard(
        *BehaviorTree->BlackboardAsset
    );
    
    // 运行行为树
    AuraAIController->RunBehaviorTree(BehaviorTree);
    
    // 设置初始黑板值
    AuraAIController->GetBlackboardComponent()->SetValueAsBool(
        FName("HitReacting"), 
        false
    );
    
    // 设置是否为远程攻击者
    AuraAIController->GetBlackboardComponent()->SetValueAsBool(
        FName("RangedAttacker"), 
        CharacterClass != ECharacterClass::Warrior
    );
}
```

---

## 行为树系统

### 行为树结构

行为树定义了 AI 的行为逻辑流程。典型的敌人行为树结构：

```
Root
├── Selector (或 Sequence)
│   ├── Service: FindNearestPlayer
│   ├── Decorator: IsDead?
│   │   └── Task: Idle
│   ├── Decorator: IsStunned?
│   │   └── Task: Wait
│   ├── Decorator: IsHitReacting?
│   │   └── Task: Wait
│   ├── Decorator: IsTargetInRange?
│   │   └── Task: Attack
│   └── Task: MoveToTarget
```

### 行为树节点类型

#### Selector（选择器）

- 从左到右执行子节点
- 如果子节点成功，停止执行
- 如果所有子节点失败，返回失败

#### Sequence（序列）

- 从左到右执行子节点
- 如果子节点失败，停止执行
- 如果所有子节点成功，返回成功

#### Decorator（装饰器）

- 条件检查节点
- 决定是否执行子节点

#### Service（服务）

- 在后台持续运行
- 更新黑板值

#### Task（任务）

- 执行具体行为
- 返回成功或失败

---

## 黑板系统

### 黑板键（Blackboard Keys）

AI 系统使用以下黑板键：

#### TargetToFollow

- **类型**: Object (Actor)
- **用途**: 存储要追踪的目标
- **更新**: 由 `BTService_FindNearestPlayer` 更新

#### DistanceToTarget

- **类型**: Float
- **用途**: 存储到目标的距离
- **更新**: 由 `BTService_FindNearestPlayer` 更新

#### HitReacting

- **类型**: Bool
- **用途**: 标记是否正在受击反应
- **更新**: 由 `HitReactTagChanged` 更新

#### RangedAttacker

- **类型**: Bool
- **用途**: 标记是否为远程攻击者
- **初始化**: 在 `PossessedBy` 中根据职业设置

#### Dead

- **类型**: Bool
- **用途**: 标记是否死亡
- **更新**: 在 `Die` 函数中设置为 true

#### Stunned

- **类型**: Bool
- **用途**: 标记是否被眩晕
- **更新**: 由 `StunTagChanged` 更新

### 黑板操作

#### 设置值

```cpp
// 设置布尔值
AIController->GetBlackboardComponent()->SetValueAsBool(
    FName("HitReacting"), 
    true
);

// 设置对象值
AIController->GetBlackboardComponent()->SetValueAsObject(
    FName("TargetToFollow"), 
    TargetActor
);

// 设置浮点值
AIController->GetBlackboardComponent()->SetValueAsFloat(
    FName("DistanceToTarget"), 
    100.f
);
```

#### 获取值

```cpp
// 获取布尔值
bool bHitReacting = AIController->GetBlackboardComponent()->GetValueAsBool(
    FName("HitReacting")
);

// 获取对象值
AActor* Target = Cast<AActor>(
    AIController->GetBlackboardComponent()->GetValueAsObject(
        FName("TargetToFollow")
    )
);

// 获取浮点值
float Distance = AIController->GetBlackboardComponent()->GetValueAsFloat(
    FName("DistanceToTarget")
);
```

---

## 目标查找服务

### UBTService_FindNearestPlayer

查找最近的玩家或敌人的服务节点。

#### 类定义

```cpp
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService_BlueprintBase
{
    GENERATED_BODY()
    
protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;
    
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FBlackboardKeySelector TargetToFollowSelector;
    
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FBlackboardKeySelector DistanceToTargetSelector;
};
```

#### 实现逻辑

```cpp
void UBTService_FindNearestPlayer::TickNode(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, 
    float DeltaSeconds
)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    // 获取拥有者 Pawn
    APawn* OwningPawn = AIOwner->GetPawn();
    
    // 确定目标标签（玩家查找敌人，敌人查找玩家）
    const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) 
        ? FName("Enemy") 
        : FName("Player");
    
    // 获取所有带有目标标签的 Actor
    TArray<AActor*> ActorsWithTag;
    UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);
    
    // 查找最近的 Actor
    float ClosestDistance = TNumericLimits<float>::Max();
    AActor* ClosestActor = nullptr;
    
    for (AActor* Actor : ActorsWithTag)
    {
        if (IsValid(Actor) && IsValid(OwningPawn))
        {
            const float Distance = OwningPawn->GetDistanceTo(Actor);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestActor = Actor;
            }
        }
    }
    
    // 更新黑板值
    UBTFunctionLibrary::SetBlackboardValueAsObject(
        this, 
        TargetToFollowSelector, 
        ClosestActor
    );
    UBTFunctionLibrary::SetBlackboardValueAsFloat(
        this, 
        DistanceToTargetSelector, 
        ClosestDistance
    );
}
```

#### 工作原理

1. **确定目标类型**: 根据拥有者的标签确定查找目标（Player 或 Enemy）
2. **查找所有目标**: 使用 `GetAllActorsWithTag` 查找所有目标
3. **计算距离**: 计算到每个目标的距离
4. **选择最近目标**: 选择距离最近的目标
5. **更新黑板**: 更新目标对象和距离到黑板

---

## 攻击任务

### UBTTask_Attack

攻击任务节点，执行攻击行为。

#### 类定义

```cpp
UCLASS()
class AURA_API UBTTask_Attack : public UBTTask_BlueprintBase
{
    GENERATED_BODY()
    
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

#### 实现

```cpp
EBTNodeResult::Type UBTTask_Attack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory
)
{
    return Super::ExecuteTask(OwnerComp, NodeMemory);
}
```

**说明**: 实际的攻击逻辑通常在蓝图中实现，通过 `ExecuteTask` 调用蓝图函数。

#### 蓝图实现

在蓝图中，攻击任务通常：

1. 获取 AbilitySystemComponent
2. 获取攻击能力标签
3. 激活攻击能力
4. 等待攻击完成
5. 返回成功或失败

---

## AI 状态管理

### 受击反应

#### HitReactTagChanged

当受击反应 Tag 变化时调用：

```cpp
void AAuraEnemy::HitReactTagChanged(
    const FGameplayTag CallbackTag, 
    int32 NewCount
)
{
    // 更新受击反应状态
    bHitReacting = NewCount > 0;
    
    // 如果正在受击反应，停止移动
    GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
    
    // 更新黑板值
    if (AuraAIController && AuraAIController->GetBlackboardComponent())
    {
        AuraAIController->GetBlackboardComponent()->SetValueAsBool(
            FName("HitReacting"), 
            bHitReacting
        );
    }
}
```

#### 注册 Tag 事件

```cpp
void AAuraEnemy::BeginPlay()
{
    // ... 其他初始化 ...
    
    // 注册受击反应 Tag 事件
    AbilitySystemComponent->RegisterGameplayTagEvent(
        FAuraGameplayTags::Get().Effects_HitReact, 
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &AAuraEnemy::HitReactTagChanged);
}
```

### 眩晕状态

#### StunTagChanged

当眩晕 Tag 变化时调用：

```cpp
void AAuraEnemy::StunTagChanged(
    const FGameplayTag CallbackTag, 
    int32 NewCount
)
{
    Super::StunTagChanged(CallbackTag, NewCount);
    
    // 更新黑板值
    if (AuraAIController && AuraAIController->GetBlackboardComponent())
    {
        AuraAIController->GetBlackboardComponent()->SetValueAsBool(
            FName("Stunned"), 
            bIsStunned
        );
    }
}
```

### 死亡状态

#### Die

当敌人死亡时调用：

```cpp
void AAuraEnemy::Die(const FVector& DeathImpulse)
{
    // 设置生命周期
    SetLifeSpan(LifeSpan);
    
    // 更新黑板死亡标志
    if (AuraAIController) 
    {
        AuraAIController->GetBlackboardComponent()->SetValueAsBool(
            FName("Dead"), 
            true
        );
    }
    
    // 生成战利品
    SpawnLoot();
    
    // 调用父类死亡处理
    Super::Die(DeathImpulse);
}
```

---

## 高亮系统

### IHighlightInterface

敌人实现 `IHighlightInterface` 接口，支持高亮显示。

#### HighlightActor

高亮角色：

```cpp
void AAuraEnemy::HighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(true);
    Weapon->SetRenderCustomDepth(true);
}
```

#### UnHighlightActor

取消高亮：

```cpp
void AAuraEnemy::UnHighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(false);
    Weapon->SetRenderCustomDepth(false);
}
```

#### SetMoveToLocation

设置移动目标位置（用于高亮交互）：

```cpp
void AAuraEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
    // 不改变目标位置（敌人不响应移动命令）
}
```

### 自定义深度值

```cpp
// 在构造函数中设置
GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
```

**说明**: `CUSTOM_DEPTH_RED` 是自定义的深度值，用于渲染高亮效果。

---

## 生命值条系统

### HealthBar WidgetComponent

敌人使用 `WidgetComponent` 显示生命值条。

#### 初始化

```cpp
// 在构造函数中创建
HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
HealthBar->SetupAttachment(GetRootComponent());

// 在 BeginPlay 中设置 Widget Controller
if (UAuraUserWidget* AuraUserWidget = 
    Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
{
    AuraUserWidget->SetWidgetController(this);
}
```

#### 属性绑定

```cpp
void AAuraEnemy::BeginPlay()
{
    // ... 其他初始化 ...
    
    // 绑定生命值变化
    if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
    {
        // 绑定当前生命值
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AuraAS->GetHealthAttribute()
        ).AddLambda(
            [this](const FOnAttributeChangeData& Data)
            {
                OnHealthChanged.Broadcast(Data.NewValue);
            }
        );
        
        // 绑定最大生命值
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AuraAS->GetMaxHealthAttribute()
        ).AddLambda(
            [this](const FOnAttributeChangeData& Data)
            {
                OnMaxHealthChanged.Broadcast(Data.NewValue);
            }
        );
        
        // 广播初始值
        OnHealthChanged.Broadcast(AuraAS->GetHealth());
        OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
    }
}
```

#### 委托

```cpp
UPROPERTY(BlueprintAssignable)
FOnAttributeChangedSignature OnHealthChanged;

UPROPERTY(BlueprintAssignable)
FOnAttributeChangedSignature OnMaxHealthChanged;
```

---

## 配置指南

### 创建敌人

#### 步骤 1: 创建敌人蓝图

1. 创建继承自 `BP_AuraEnemy` 的蓝图
2. 设置网格、动画、武器等

#### 步骤 2: 配置行为树

1. 创建或使用现有的行为树
2. 配置行为树节点
3. 设置黑板键

#### 步骤 3: 设置敌人属性

在敌人蓝图中设置：

- **BehaviorTree**: 行为树资产
- **Level**: 敌人等级
- **CharacterClass**: 职业类型
- **LifeSpan**: 死亡后生命周期

### 创建行为树

#### 步骤 1: 创建行为树资产

1. 在编辑器中创建 `Behavior Tree` 资产
2. 创建对应的 `Blackboard` 资产

#### 步骤 2: 配置黑板

在黑板中创建以下键：

- **TargetToFollow** (Object, Actor)
- **DistanceToTarget** (Float)
- **HitReacting** (Bool)
- **RangedAttacker** (Bool)
- **Dead** (Bool)
- **Stunned** (Bool)

#### 步骤 3: 构建行为树

1. 添加 `BTService_FindNearestPlayer` 服务
2. 添加条件检查装饰器
3. 添加攻击和移动任务

### 配置目标查找服务

在行为树中配置 `BTService_FindNearestPlayer`：

1. 设置 `TargetToFollowSelector` 为 `TargetToFollow` 黑板键
2. 设置 `DistanceToTargetSelector` 为 `DistanceToTarget` 黑板键
3. 设置服务执行频率（Tick Interval）

---

## 使用示例

### 自定义行为树服务

#### 创建新的服务

```cpp
// MyBTService.h
UCLASS()
class AURA_API UMyBTService : public UBTService_BlueprintBase
{
    GENERATED_BODY()
    
protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;
    
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FBlackboardKeySelector MyKeySelector;
};
```

```cpp
// MyBTService.cpp
void UMyBTService::TickNode(...)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    // 自定义逻辑
    APawn* OwningPawn = AIOwner->GetPawn();
    
    // 更新黑板值
    UBTFunctionLibrary::SetBlackboardValueAsBool(
        this, 
        MyKeySelector, 
        true
    );
}
```

### 自定义行为树任务

#### 创建新的任务

```cpp
// MyBTTask.h
UCLASS()
class AURA_API UMyBTTask : public UBTTask_BlueprintBase
{
    GENERATED_BODY()
    
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

```cpp
// MyBTTask.cpp
EBTNodeResult::Type UMyBTTask::ExecuteTask(...)
{
    // 获取 AI 控制器
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    
    // 执行自定义逻辑
    // ...
    
    return EBTNodeResult::Succeeded;
}
```

### 在蓝图中扩展

#### 扩展攻击任务

在蓝图中继承 `BTTask_Attack`：

1. 重写 `ExecuteTask` 函数
2. 获取 AbilitySystemComponent
3. 激活攻击能力
4. 等待攻击完成
5. 返回结果

---

## 最佳实践

### 1. 行为树设计

- **模块化**: 将行为分解为可复用的节点
- **清晰结构**: 使用 Selector 和 Sequence 组织逻辑
- **性能优化**: 合理设置服务执行频率

### 2. 黑板管理

- **最小化键**: 只存储必要的数据
- **类型明确**: 使用正确的数据类型
- **命名规范**: 使用清晰的命名

### 3. 状态管理

- **及时更新**: 状态变化时及时更新黑板
- **状态同步**: 确保状态在服务器和客户端同步
- **状态清理**: 在适当时机清理状态

### 4. 性能优化

- **服务频率**: 合理设置服务执行频率
- **距离检查**: 使用距离检查优化目标查找
- **对象池**: 考虑使用对象池管理敌人

---

## 总结

AI 系统提供了完整的敌人行为控制功能：

- ✅ **行为树集成**: 使用 UE5 行为树系统
- ✅ **黑板管理**: 灵活的数据存储和访问
- ✅ **目标查找**: 自动查找和追踪目标
- ✅ **状态响应**: 响应受击、眩晕等状态
- ✅ **高亮系统**: 支持高亮显示
- ✅ **生命值条**: 显示敌人生命值

通过这个系统，开发者可以创建智能的敌人 AI，提供丰富的战斗体验。

