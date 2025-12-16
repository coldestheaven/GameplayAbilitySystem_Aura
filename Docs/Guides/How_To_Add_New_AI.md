# 如何添加新的AI

本指南将详细介绍如何在 Aura 项目中添加新的 AI 系统，包括创建自定义 AI 控制器、行为树、黑板、服务（Service）和任务（Task）。

## 目录

1. [AI 系统概述](#ai-系统概述)
2. [添加新AI的步骤概览](#添加新ai的步骤概览)
3. [创建自定义 AI 控制器](#创建自定义-ai-控制器)
4. [创建行为树和黑板](#创建行为树和黑板)
5. [创建自定义服务（Service）](#创建自定义服务service)
6. [创建自定义任务（Task）](#创建自定义任务task)
7. [创建自定义装饰器（Decorator）](#创建自定义装饰器decorator)
8. [配置敌人角色使用新AI](#配置敌人角色使用新ai)
9. [完整示例：巡逻AI](#完整示例巡逻ai)
10. [测试和调试](#测试和调试)
11. [常见问题和解决方案](#常见问题和解决方案)
12. [最佳实践](#最佳实践)

---

## AI 系统概述

Aura 项目使用 Unreal Engine 的行为树（Behavior Tree）和黑板（Blackboard）系统来实现 AI 行为。核心组件包括：

### 核心组件

- **AAuraAIController**: 基础 AI 控制器，管理行为树和黑板组件
- **AAuraEnemy**: 敌人角色类，包含 AI 相关配置
- **UBTService_FindNearestPlayer**: 查找最近玩家的服务
- **UBTTask_Attack**: 攻击任务
- **UBehaviorTree**: 行为树资产，定义 AI 行为逻辑
- **UBlackboard**: 黑板资产，存储 AI 状态数据

### 系统架构

```
AAuraEnemy (敌人角色)
    └── AAuraAIController (AI控制器)
        ├── UBlackboardComponent (黑板组件)
        └── UBehaviorTreeComponent (行为树组件)
            └── UBehaviorTree (行为树资产)
                ├── Services (服务节点)
                ├── Decorators (装饰器节点)
                └── Tasks (任务节点)
```

---

## 添加新AI的步骤概览

添加新 AI 的完整流程：

1. **确定 AI 需求** - 定义 AI 的行为类型（巡逻、战斗、逃跑等）
2. **创建 AI 控制器**（可选）- 如果需要自定义控制器逻辑
3. **创建黑板** - 定义 AI 需要存储的数据
4. **创建行为树** - 定义 AI 的行为逻辑
5. **创建自定义服务/任务**（可选）- 如果需要特殊行为
6. **配置敌人角色** - 将 AI 系统应用到敌人
7. **测试和调试** - 验证 AI 行为

---

## 创建自定义 AI 控制器

### 步骤 1: 创建 C++ 类

如果只需要基础功能，可以直接使用 `AAuraAIController`。如果需要自定义逻辑，创建新的控制器类：

#### 1.1 创建头文件

在 `Source/Aura/Public/AI/` 目录下创建 `MyAIController.h`：

```cpp
// MyAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AI/AuraAIController.h"
#include "MyAIController.generated.h"

/**
 * 自定义 AI 控制器示例
 */
UCLASS()
class AURA_API AMyAIController : public AAuraAIController
{
    GENERATED_BODY()

public:
    AMyAIController();

    // 自定义函数
    UFUNCTION(BlueprintCallable)
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable)
    void StartPatrol();

protected:
    // 自定义属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolWaitTime = 3.0f;

    // 重写函数
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
};
```

#### 1.2 创建实现文件

在 `Source/Aura/Private/AI/` 目录下创建 `MyAIController.cpp`：

```cpp
// MyAIController.cpp
#include "AI/MyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AMyAIController::AMyAIController()
{
    // 构造函数逻辑
}

void AMyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 自定义初始化逻辑
    if (Blackboard && InPawn)
    {
        // 设置初始黑板值
        Blackboard->SetValueAsVector(FName("PatrolStartLocation"), InPawn->GetActorLocation());
    }
}

void AMyAIController::OnUnPossess()
{
    Super::OnUnPossess();

    // 清理逻辑
}

void AMyAIController::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    
    if (Blackboard && PatrolPoints.Num() > 0)
    {
        Blackboard->SetValueAsVector(FName("NextPatrolPoint"), PatrolPoints[0]);
    }
}

void AMyAIController::StartPatrol()
{
    if (Blackboard)
    {
        Blackboard->SetValueAsBool(FName("ShouldPatrol"), true);
    }
}
```

#### 1.3 编译项目

1. 关闭 Unreal Editor
2. 在 Visual Studio 中编译项目
3. 重新打开 Unreal Editor

---

## 创建行为树和黑板

### 步骤 1: 创建黑板资产

1. **在内容浏览器中创建黑板**：
   - 右键点击内容浏览器
   - 选择 `Artificial Intelligence` → `Blackboard`
   - 命名为 `BB_MyAI`

2. **配置黑板键**：
   
   打开黑板编辑器，添加以下键：

   | 键名 | 类型 | 说明 |
   |------|------|------|
   | `TargetToFollow` | Object (Actor) | 要追踪的目标 |
   | `DistanceToTarget` | Float | 到目标的距离 |
   | `HitReacting` | Bool | 是否正在受击反应 |
   | `RangedAttacker` | Bool | 是否为远程攻击者 |
   | `Dead` | Bool | 是否死亡 |
   | `Stunned` | Bool | 是否被眩晕 |
   | `ShouldPatrol` | Bool | 是否应该巡逻 |
   | `NextPatrolPoint` | Vector | 下一个巡逻点 |
   | `CurrentPatrolIndex` | Int | 当前巡逻点索引 |

### 步骤 2: 创建行为树资产

1. **在内容浏览器中创建行为树**：
   - 右键点击内容浏览器
   - 选择 `Artificial Intelligence` → `Behavior Tree`
   - 命名为 `BT_MyAI`

2. **关联黑板**：
   - 在行为树编辑器中，点击 `Blackboard` 下拉菜单
   - 选择 `BB_MyAI`

3. **构建行为树结构**：

   基本结构示例：

   ```
   Root
   └── Selector (主选择器)
       ├── Service: FindNearestPlayer (查找最近玩家)
       ├── Decorator: IsDead? (是否死亡)
       │   └── Task: Idle (待机)
       ├── Decorator: IsStunned? (是否眩晕)
       │   └── Task: Wait (等待)
       ├── Decorator: IsHitReacting? (是否受击)
       │   └── Task: Wait (等待)
       ├── Decorator: IsTargetInRange? (目标在范围内?)
       │   └── Task: Attack (攻击)
       ├── Decorator: ShouldPatrol? (应该巡逻?)
       │   └── Sequence (巡逻序列)
       │       ├── Task: MoveTo (移动到巡逻点)
       │       └── Task: Wait (等待)
       └── Task: MoveToTarget (移动到目标)
   ```

### 步骤 3: 配置行为树节点

#### 3.1 添加服务节点

1. 在根节点上右键 → `Add Service` → `Find Nearest Player`
2. 配置服务属性：
   - `Target To Follow Selector`: 选择 `TargetToFollow`
   - `Distance To Target Selector`: 选择 `DistanceToTarget`
   - `Tick Interval`: 设置更新频率（如 0.5 秒）

#### 3.2 添加装饰器节点

1. **IsDead 装饰器**：
   - 添加 `Blackboard Based Condition`
   - `Blackboard Key`: 选择 `Dead`
   - `Observer aborts`: 选择 `Both`（当值变化时中断）

2. **IsTargetInRange 装饰器**：
   - 添加 `Blackboard Key Decorator`
   - `Blackboard Key`: 选择 `DistanceToTarget`
   - `Arithmetic Operation`: 选择 `Less`
   - `Value`: 设置攻击范围（如 500）

#### 3.3 添加任务节点

1. **MoveTo 任务**：
   - 添加 `Move To`
   - `Blackboard Key`: 选择 `TargetToFollow`
   - `Acceptable Radius`: 设置接受半径（如 50）

2. **Attack 任务**：
   - 添加 `BTTask_Attack`（如果已创建）
   - 或使用蓝图任务

3. **Wait 任务**：
   - 添加 `Wait`
   - `Wait Time`: 设置等待时间（如 2.0 秒）

---

## 创建自定义服务（Service）

服务节点在后台持续运行，用于更新黑板值。

### 步骤 1: 创建 C++ 服务类

#### 1.1 创建头文件

在 `Source/Aura/Public/AI/` 目录下创建 `BTService_Patrol.h`：

```cpp
// BTService_Patrol.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_Patrol.generated.h"

/**
 * 巡逻服务 - 更新下一个巡逻点
 */
UCLASS()
class AURA_API UBTService_Patrol : public UBTService_BlueprintBase
{
    GENERATED_BODY()

public:
    UBTService_Patrol();

protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

    // 黑板键选择器
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NextPatrolPointSelector;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CurrentPatrolIndexSelector;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolPointsSelector;

    // 配置属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolPointRadius = 100.0f;
};
```

#### 1.2 创建实现文件

在 `Source/Aura/Private/AI/` 目录下创建 `BTService_Patrol.cpp`：

```cpp
// BTService_Patrol.cpp
#include "AI/BTService_Patrol.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_Patrol::UBTService_Patrol()
{
    NodeName = "Update Patrol Point";
    bNotifyTick = true;
    TickInterval = 1.0f; // 每秒更新一次
}

void UBTService_Patrol::TickNode(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, 
    float DeltaSeconds
)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* OwningPawn = AIOwner->GetPawn();
    if (!OwningPawn)
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    // 获取当前巡逻点索引
    int32 CurrentIndex = BlackboardComp->GetValueAsInt(
        CurrentPatrolIndexSelector.SelectedKeyName
    );

    // 获取巡逻点数组（需要从 AI 控制器或其他地方获取）
    // 这里假设巡逻点存储在 AI 控制器中
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return;
    }

    // 检查是否到达当前巡逻点
    FVector CurrentPatrolPoint = BlackboardComp->GetValueAsVector(
        NextPatrolPointSelector.SelectedKeyName
    );

    float DistanceToPoint = FVector::Dist(
        OwningPawn->GetActorLocation(), 
        CurrentPatrolPoint
    );

    // 如果到达巡逻点，移动到下一个
    if (DistanceToPoint < PatrolPointRadius)
    {
        // 这里需要从 AI 控制器获取巡逻点数组
        // 示例：假设有 4 个巡逻点，循环遍历
        int32 NextIndex = (CurrentIndex + 1) % 4; // 假设有 4 个点
        
        // 计算下一个巡逻点位置（这里需要根据实际实现调整）
        FVector NextPoint = CurrentPatrolPoint + FVector(500.0f, 0.0f, 0.0f);
        
        // 更新黑板
        UBTFunctionLibrary::SetBlackboardValueAsVector(
            this, 
            NextPatrolPointSelector, 
            NextPoint
        );
        
        UBTFunctionLibrary::SetBlackboardValueAsInt(
            this, 
            CurrentPatrolIndexSelector, 
            NextIndex
        );
    }
}
```

### 步骤 2: 在行为树中使用服务

1. 在行为树编辑器中，选择要添加服务的节点（通常是根节点或复合节点）
2. 右键 → `Add Service` → 选择 `BTService_Patrol`
3. 配置服务属性：
   - `Next Patrol Point Selector`: 选择 `NextPatrolPoint`
   - `Current Patrol Index Selector`: 选择 `CurrentPatrolIndex`
   - `Patrol Point Radius`: 设置巡逻点半径

---

## 创建自定义任务（Task）

任务节点执行具体的行为，如移动、攻击、等待等。

### 步骤 1: 创建 C++ 任务类

#### 1.1 创建头文件

在 `Source/Aura/Public/AI/` 目录下创建 `BTTask_Patrol.h`：

```cpp
// BTTask_Patrol.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_Patrol.generated.h"

/**
 * 巡逻任务 - 移动到下一个巡逻点
 */
UCLASS()
class AURA_API UBTTask_Patrol : public UBTTask_BlueprintBase
{
    GENERATED_BODY()

public:
    UBTTask_Patrol();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;

protected:
    // 黑板键选择器
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolPointSelector;

    // 配置属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float AcceptableRadius = 50.0f;
};
```

#### 1.2 创建实现文件

在 `Source/Aura/Private/AI/` 目录下创建 `BTTask_Patrol.cpp`：

```cpp
// BTTask_Patrol.cpp
#include "AI/BTTask_Patrol.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"

UBTTask_Patrol::UBTTask_Patrol()
{
    NodeName = "Patrol";
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory
)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    // 获取巡逻点
    FVector PatrolPoint = BlackboardComp->GetValueAsVector(
        PatrolPointSelector.SelectedKeyName
    );

    // 使用导航系统查找有效位置
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation ProjectedLocation;
    bool bProjected = NavSystem->ProjectPointToNavigation(
        PatrolPoint, 
        ProjectedLocation, 
        FVector(100.0f, 100.0f, 100.0f)
    );

    if (!bProjected)
    {
        return EBTNodeResult::Failed;
    }

    // 移动到巡逻点
    AIController->MoveToLocation(
        ProjectedLocation.Location, 
        AcceptableRadius, 
        false,  // bStopOnOverlap
        true,   // bUsePathfinding
        true,   // bCanStrafe
        nullptr, // FilterClass
        true    // bAllowPartialPath
    );

    return EBTNodeResult::InProgress;
}
```

**注意**: 对于需要等待完成的任务（如移动），通常需要在蓝图中实现，使用 `FinishLatentTask` 来标记完成。

### 步骤 2: 创建蓝图任务（推荐）

对于需要等待的任务，建议创建蓝图任务：

1. **创建蓝图任务**：
   - 在内容浏览器中创建蓝图
   - 父类选择 `BTTask_Patrol`
   - 命名为 `BTTask_Patrol_BP`

2. **实现 ExecuteTask**：
   ```cpp
   // 在蓝图中实现
   Event ExecuteTask
   ├── Get AI Owner
   ├── Get Controlled Pawn
   ├── Get Blackboard Component
   ├── Get Value as Vector (PatrolPoint)
   ├── AI Move To (移动到巡逻点)
   └── Wait for Move Complete
       └── Finish Latent Task (Success)
   ```

---

## 创建自定义装饰器（Decorator）

装饰器用于条件检查，决定是否执行子节点。

### 步骤 1: 创建 C++ 装饰器类

#### 1.1 创建头文件

在 `Source/Aura/Public/AI/` 目录下创建 `BTDecorator_IsAtPatrolPoint.h`：

```cpp
// BTDecorator_IsAtPatrolPoint.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_IsAtPatrolPoint.generated.h"

/**
 * 检查是否到达巡逻点的装饰器
 */
UCLASS()
class AURA_API UBTDecorator_IsAtPatrolPoint : public UBTDecorator_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTDecorator_IsAtPatrolPoint();

protected:
    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;

    // 配置属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float AcceptableRadius = 100.0f;
};
```

#### 1.2 创建实现文件

在 `Source/Aura/Private/AI/` 目录下创建 `BTDecorator_IsAtPatrolPoint.cpp`：

```cpp
// BTDecorator_IsAtPatrolPoint.cpp
#include "AI/BTDecorator_IsAtPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsAtPatrolPoint::UBTDecorator_IsAtPatrolPoint()
{
    NodeName = "Is At Patrol Point";
    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAtPatrolPoint, BlackboardKey));
}

bool UBTDecorator_IsAtPatrolPoint::CalculateRawConditionValue(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory
) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return false;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return false;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return false;
    }

    // 获取巡逻点
    FVector PatrolPoint = BlackboardComp->GetValueAsVector(BlackboardKey.SelectedKeyName);
    
    // 计算距离
    float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), PatrolPoint);
    
    // 检查是否在范围内
    return Distance <= AcceptableRadius;
}
```

---

## 配置敌人角色使用新AI

### 步骤 1: 创建或修改敌人蓝图

1. **打开敌人蓝图**（如 `BP_AuraEnemy` 或创建新的）

2. **设置 AI 控制器类**：
   - 在 `Class Defaults` 中
   - `AI Controller Class`: 选择 `MyAIController`（如果创建了自定义控制器）
   - 或使用默认的 `AuraAIController`

3. **设置行为树**：
   - `Behavior Tree`: 选择 `BT_MyAI`

4. **设置其他属性**：
   - `Level`: 设置敌人等级
   - `Character Class`: 设置职业类型
   - `Life Span`: 设置死亡后生命周期

### 步骤 2: 配置 AI 控制器（如果使用自定义控制器）

如果创建了自定义 AI 控制器，需要在敌人蓝图中设置：

1. **设置巡逻点**（如果使用巡逻功能）：
   - 在 `BeginPlay` 事件中调用 `SetPatrolPoints`
   - 或在蓝图中直接设置 `PatrolPoints` 数组

2. **启动 AI**：
   - 在 `BeginPlay` 中调用 `StartPatrol`（如果需要）

### 步骤 3: 设置 Actor 标签

确保敌人有正确的标签：

1. 在敌人蓝图的 `Class Defaults` 中
2. `Actor Tags`: 添加 `Enemy` 标签

---

## 完整示例：巡逻AI

以下是一个完整的巡逻 AI 实现示例。

### 示例：创建巡逻敌人

#### 1. 创建巡逻服务

```cpp
// BTService_UpdatePatrolPoint.h
UCLASS()
class AURA_API UBTService_UpdatePatrolPoint : public UBTService_BlueprintBase
{
    GENERATED_BODY()

protected:
    virtual void TickNode(...) override;

    UPROPERTY(EditAnywhere)
    FBlackboardKeySelector PatrolPointKey;

    UPROPERTY(EditAnywhere)
    FBlackboardKeySelector PatrolIndexKey;

    UPROPERTY(EditAnywhere)
    TArray<FVector> PatrolPoints;
};
```

#### 2. 创建行为树结构

```
Root
└── Selector
    ├── Service: FindNearestPlayer
    ├── Decorator: IsDead?
    │   └── Task: Idle
    ├── Decorator: HasTarget? (有目标?)
    │   └── Sequence (战斗序列)
    │       ├── Decorator: IsTargetInRange?
    │       │   └── Task: Attack
    │       └── Task: MoveToTarget
    └── Sequence (巡逻序列)
        ├── Service: UpdatePatrolPoint
        ├── Task: MoveToPatrolPoint
        └── Task: Wait (2.0秒)
```

#### 3. 配置敌人

在敌人蓝图中：
- `Behavior Tree`: `BT_PatrolEnemy`
- `AI Controller Class`: `AuraAIController`
- `Actor Tags`: `Enemy`

---

## 测试和调试

### 测试检查清单

#### 基础功能测试

- [ ] AI 控制器正确初始化
- [ ] 行为树正确运行
- [ ] 黑板值正确更新
- [ ] AI 能够查找目标
- [ ] AI 能够移动到目标
- [ ] AI 能够执行攻击

#### 巡逻功能测试（如果实现）

- [ ] AI 能够移动到巡逻点
- [ ] AI 到达巡逻点后等待
- [ ] AI 能够循环巡逻
- [ ] AI 在发现目标时中断巡逻

#### 状态管理测试

- [ ] AI 正确响应受击状态
- [ ] AI 正确响应眩晕状态
- [ ] AI 正确响应死亡状态
- [ ] 状态变化时黑板值正确更新

### 调试技巧

#### 1. 使用行为树调试器

1. 在编辑器中运行游戏（PIE）
2. 选择 AI 控制的敌人
3. 打开 `Window` → `Behavior Tree Debugger`
4. 查看行为树执行状态和黑板值

#### 2. 打印调试信息

在服务或任务中添加调试打印：

```cpp
// 在服务中
UE_LOG(LogTemp, Warning, TEXT("Current Target: %s"), 
    *GetNameSafe(ClosestActor));

// 在任务中
UE_LOG(LogTemp, Warning, TEXT("Moving to: %s"), 
    *PatrolPoint.ToString());
```

#### 3. 检查黑板值

在蓝图中或 C++ 中检查黑板值：

```cpp
// C++ 中
if (BlackboardComp)
{
    AActor* Target = Cast<AActor>(
        BlackboardComp->GetValueAsObject(FName("TargetToFollow"))
    );
    float Distance = BlackboardComp->GetValueAsFloat(
        FName("DistanceToTarget")
    );
}
```

#### 4. 使用控制台命令

在游戏中按 `` ` `` 键打开控制台，使用以下命令：

- `ai.DebugBehaviorTrees 1` - 显示行为树调试信息
- `ai.DebugBlackboard 1` - 显示黑板调试信息
- `ai.DebugNavigation 1` - 显示导航调试信息

---

## 常见问题和解决方案

### 问题 1: AI 不移动

**可能原因**：
- 行为树未正确运行
- 导航网格未生成
- 目标位置无效

**解决方案**：
1. 检查行为树是否正确运行（使用调试器）
2. 确保关卡中有导航网格（NavMesh）
3. 检查目标位置是否在导航网格上
4. 检查 `MoveTo` 任务配置

### 问题 2: AI 不攻击

**可能原因**：
- 攻击任务未正确实现
- 能力系统未初始化
- 攻击范围装饰器配置错误

**解决方案**：
1. 检查攻击任务的蓝图实现
2. 确保 `AbilitySystemComponent` 已初始化
3. 检查攻击范围装饰器的距离设置
4. 验证攻击能力标签是否正确

### 问题 3: 黑板值不更新

**可能原因**：
- 服务未正确配置
- 黑板键选择器未设置
- 服务执行频率过低

**解决方案**：
1. 检查服务的 `TickNode` 是否被调用
2. 验证黑板键选择器是否正确设置
3. 调整服务的 `TickInterval`
4. 使用调试器查看黑板值

### 问题 4: AI 行为不符合预期

**可能原因**：
- 行为树结构不正确
- 装饰器条件错误
- 任务执行顺序问题

**解决方案**：
1. 使用行为树调试器逐步检查
2. 验证装饰器条件逻辑
3. 检查 Selector 和 Sequence 的使用
4. 添加更多调试信息

### 问题 5: 性能问题

**可能原因**：
- 服务执行频率过高
- 目标查找范围过大
- 行为树节点过多

**解决方案**：
1. 降低服务的 `TickInterval`
2. 限制目标查找范围
3. 优化行为树结构，减少不必要的节点
4. 使用对象池管理敌人

---

## 最佳实践

### 1. 行为树设计

- **模块化**: 将行为分解为可复用的节点和服务
- **清晰结构**: 使用 Selector 和 Sequence 组织逻辑
- **性能优化**: 合理设置服务执行频率，避免每帧更新
- **可扩展性**: 设计时考虑未来扩展需求

### 2. 黑板管理

- **最小化键**: 只存储必要的数据，避免冗余
- **类型明确**: 使用正确的数据类型（Bool、Float、Object 等）
- **命名规范**: 使用清晰的命名（如 `TargetToFollow`、`DistanceToTarget`）
- **及时更新**: 状态变化时及时更新黑板值

### 3. 服务设计

- **单一职责**: 每个服务只负责一个功能
- **可配置**: 使用 `UPROPERTY(EditAnywhere)` 使服务可配置
- **性能考虑**: 避免在服务中进行昂贵的计算
- **错误处理**: 添加空指针检查和错误处理

### 4. 任务设计

- **异步支持**: 对于需要等待的任务，使用蓝图实现
- **状态返回**: 正确返回任务状态（Success、Failed、InProgress）
- **资源清理**: 在任务完成或失败时清理资源
- **可中断**: 考虑任务是否应该被中断

### 5. 代码组织

- **命名规范**: 使用清晰的类名和函数名
- **注释文档**: 为复杂逻辑添加注释
- **代码复用**: 提取公共逻辑为函数
- **测试友好**: 设计时考虑测试需求

### 6. 性能优化

- **服务频率**: 根据需求设置合理的更新频率
- **距离检查**: 使用距离检查优化目标查找
- **对象池**: 考虑使用对象池管理敌人实例
- **LOD 系统**: 对于大量 AI，考虑使用 LOD 系统

---

## 总结

添加新 AI 的完整流程：

1. ✅ **确定需求** - 定义 AI 行为类型
2. ✅ **创建控制器**（可选）- 自定义 AI 控制器
3. ✅ **创建黑板** - 定义数据存储
4. ✅ **创建行为树** - 定义行为逻辑
5. ✅ **创建服务/任务**（可选）- 自定义行为节点
6. ✅ **配置敌人** - 应用 AI 系统
7. ✅ **测试调试** - 验证功能

通过遵循本指南，你可以创建各种类型的 AI，从简单的巡逻敌人到复杂的战斗 AI。记住要：

- 使用行为树调试器进行调试
- 遵循最佳实践
- 进行充分的测试
- 优化性能

---

## 相关文档

- [AI 系统文档](../Systems/AI_System.md) - AI 系统详细技术文档
- [角色系统文档](../Systems/Character_System.md) - 敌人角色实现细节
- [如何添加新角色](./How_To_Add_New_Character.md) - 添加新角色的指南

