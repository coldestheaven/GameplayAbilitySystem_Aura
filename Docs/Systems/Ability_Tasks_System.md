# Ability Tasks 和 Async Tasks 系统文档

## 概述

Ability Tasks 和 Async Tasks 是 GAS 中用于处理异步操作和复杂任务的重要组件。Ability Tasks 在技能执行期间运行，Async Tasks 可以在任何地方使用，用于等待特定条件。

## Ability Tasks

### UTargetDataUnderMouse

获取鼠标下的目标数据，用于需要鼠标位置或目标的技能。

#### 类层次结构

```
UAbilityTask (UE5 Base)
    ↓
UTargetDataUnderMouse
```

#### 核心功能

1. **鼠标位置检测**
   - 获取鼠标光标下的世界位置
   - 检测鼠标下的 Actor
   - 创建 TargetData

2. **网络同步**
   - 服务器验证目标数据
   - 客户端到服务器的数据同步

#### 关键属性

```cpp
// 有效数据委托
UPROPERTY(BlueprintAssignable)
FMouseTargetDataSignature ValidData;
```

#### 关键方法

**创建 Task**:
```cpp
UFUNCTION(BlueprintCallable, Category="Ability|Tasks", 
    meta = (DisplayName = "TargetDataUnderMouse", 
    HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", 
    BlueprintInternalUseOnly = "true"))
static UTargetDataUnderMouse* CreateTargetDataUnderMouse(
    UGameplayAbility* OwningAbility
);
```

**激活 Task**:
```cpp
virtual void Activate() override;
void SendMouseCursorData();
void OnTargetDataReplicatedCallback(
    const FGameplayAbilityTargetDataHandle& DataHandle, 
    FGameplayTag ActivationTag
);
```

#### 使用示例

```cpp
// 在技能中
UTargetDataUnderMouse* Task = UTargetDataUnderMouse::CreateTargetDataUnderMouse(this);
Task->ValidData.AddDynamic(this, &UAuraProjectileSpell::OnTargetDataReceived);
Task->ReadyForActivation();

// 处理目标数据
void UAuraProjectileSpell::OnTargetDataReceived(
    const FGameplayAbilityTargetDataHandle& DataHandle
)
{
    if (const FGameplayAbilityTargetData_LocationInfo* LocationData = 
        DataHandle.Get(0)->GetScriptStruct<FGameplayAbilityTargetData_LocationInfo>())
    {
        FVector TargetLocation = LocationData->TargetLocation;
        SpawnProjectile(TargetLocation, ...);
    }
}
```

---

## Async Tasks

### UWaitCooldownChange

等待冷却时间变化的异步任务，用于在 UI 中显示冷却时间。

#### 类层次结构

```
UBlueprintAsyncActionBase (UE5 Base)
    ↓
UWaitCooldownChange
```

#### 核心功能

1. **冷却时间监听**
   - 监听特定冷却标签的变化
   - 检测冷却开始和结束

2. **时间更新**
   - 定期更新剩余冷却时间
   - 广播时间变化委托

#### 关键属性

```cpp
// 冷却开始委托
UPROPERTY(BlueprintAssignable)
FCooldownChangeSignature CooldownStart;

// 冷却结束委托
UPROPERTY(BlueprintAssignable)
FCooldownChangeSignature CooldownEnd;

// ASC 引用
UPROPERTY()
TObjectPtr<UAbilitySystemComponent> ASC;

// 冷却标签
FGameplayTag CooldownTag;
```

#### 关键方法

**创建 Task**:
```cpp
UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
static UWaitCooldownChange* WaitForCooldownChange(
    UAbilitySystemComponent* AbilitySystemComponent, 
    const FGameplayTag& InCooldownTag
);
```

**结束 Task**:
```cpp
UFUNCTION(BlueprintCallable)
void EndTask();
```

**标签变化处理**:
```cpp
void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
void OnActiveEffectAdded(
    UAbilitySystemComponent* TargetASC, 
    const FGameplayEffectSpec& SpecApplied, 
    FActiveGameplayEffectHandle ActiveEffectHandle
);
```

#### 使用示例

```cpp
// 在 Widget 中
void UAbilitySlotWidget::UpdateCooldown()
{
    if (UWaitCooldownChange* CooldownTask = 
        UWaitCooldownChange::WaitForCooldownChange(ASC, CooldownTag))
    {
        CooldownTask->CooldownStart.AddDynamic(this, 
            &UAbilitySlotWidget::OnCooldownStart);
        CooldownTask->CooldownEnd.AddDynamic(this, 
            &UAbilitySlotWidget::OnCooldownEnd);
    }
}

void UAbilitySlotWidget::OnCooldownStart(float TimeRemaining)
{
    // 显示冷却时间
    CooldownText->SetText(FText::AsNumber(TimeRemaining));
    CooldownProgressBar->SetPercent(1.f);
}

void UAbilitySlotWidget::OnCooldownEnd(float TimeRemaining)
{
    // 隐藏冷却时间
    CooldownText->SetVisibility(ESlateVisibility::Collapsed);
    CooldownProgressBar->SetPercent(0.f);
}
```

---

## 创建自定义 Ability Task

### 步骤 1: 创建类

```cpp
UCLASS()
class AURA_API UCustomAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
    
public:
    // 创建 Task 的静态方法
    UFUNCTION(BlueprintCallable, Category="Ability|Tasks",
        meta = (DisplayName = "CustomTask", 
        HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",
        BlueprintInternalUseOnly = "true"))
    static UCustomAbilityTask* CreateCustomTask(
        UGameplayAbility* OwningAbility
    );
    
    // 委托
    UPROPERTY(BlueprintAssignable)
    FMyDelegate OnTaskCompleted;
    
protected:
    virtual void Activate() override;
    void ExecuteTask();
};
```

### 步骤 2: 实现创建方法

```cpp
UCustomAbilityTask* UCustomAbilityTask::CreateCustomTask(
    UGameplayAbility* OwningAbility
)
{
    UCustomAbilityTask* Task = NewAbilityTask<UCustomAbilityTask>(OwningAbility);
    return Task;
}
```

### 步骤 3: 实现激活逻辑

```cpp
void UCustomAbilityTask::Activate()
{
    Super::Activate();
    
    // 执行任务逻辑
    ExecuteTask();
}

void UCustomAbilityTask::ExecuteTask()
{
    // 执行任务
    // ...
    
    // 完成任务
    OnTaskCompleted.Broadcast();
    EndTask();
}
```

---

## 创建自定义 Async Task

### 步骤 1: 创建类

```cpp
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UCustomAsyncTask : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()
    
public:
    // 创建 Task 的静态方法
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
    static UCustomAsyncTask* CreateCustomAsyncTask(
        UAbilitySystemComponent* ASC
    );
    
    // 委托
    UPROPERTY(BlueprintAssignable)
    FMyDelegate OnCompleted;
    
    // 结束 Task
    UFUNCTION(BlueprintCallable)
    void EndTask();
    
protected:
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;
    
    void ExecuteTask();
};
```

### 步骤 2: 实现创建方法

```cpp
UCustomAsyncTask* UCustomAsyncTask::CreateCustomAsyncTask(
    UAbilitySystemComponent* ASC
)
{
    UCustomAsyncTask* Task = NewObject<UCustomAsyncTask>();
    Task->ASC = ASC;
    return Task;
}
```

### 步骤 3: 实现任务逻辑

```cpp
void UCustomAsyncTask::ExecuteTask()
{
    // 执行异步任务
    // 可以绑定到委托、定时器等
    
    // 完成任务时
    OnCompleted.Broadcast();
    EndTask();
}
```

---

## 网络同步

### Ability Task 网络同步

Ability Tasks 需要处理网络同步：

```cpp
void UTargetDataUnderMouse::SendMouseCursorData()
{
    // 客户端发送数据到服务器
    FScopedPredictionWindow ScopedPredictionWindow(
        AbilitySystemComponent.Get(), true
    );
    
    // 创建目标数据
    FGameplayAbilityTargetDataHandle DataHandle;
    FGameplayAbilityTargetData_LocationInfo* LocationData = 
        new FGameplayAbilityTargetData_LocationInfo();
    
    // 设置位置
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    LocationData->TargetLocation = HitResult.ImpactPoint;
    
    DataHandle.Add(LocationData);
    
    // 发送到服务器
    FGameplayTag ApplicationTag;
    AbilitySystemComponent->ServerSetReplicatedTargetData(
        GetAbilitySpecHandle(),
        GetActivationPredictionKey(),
        DataHandle,
        ApplicationTag,
        AbilitySystemComponent->ScopedPredictionKey
    );
    
    // 如果服务器确认，广播委托
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(DataHandle);
    }
}
```

---

## 最佳实践

### 1. 清理资源

```cpp
void UCustomAbilityTask::OnDestroy(bool bAbilityEnded)
{
    Super::OnDestroy(bAbilityEnded);
    
    // 清理资源
    // 取消绑定委托
    // 停止定时器
}
```

### 2. 错误处理

```cpp
void UCustomAbilityTask::Activate()
{
    if (!IsValid(AbilitySystemComponent))
    {
        EndTask();
        return;
    }
    
    // 执行任务
}
```

### 3. 网络检查

```cpp
void UCustomAbilityTask::ExecuteTask()
{
    const bool bIsServer = AbilitySystemComponent->GetOwner()->HasAuthority();
    
    if (bIsServer)
    {
        // 服务器逻辑
    }
    else
    {
        // 客户端逻辑
    }
}
```

---

## 相关文档

- [技能系统](../Core/Ability_System.md) - 技能使用 Ability Tasks
- [UI 系统](./UI_System.md) - UI 使用 Async Tasks

---

## 总结

Ability Tasks 和 Async Tasks 系统提供了：

1. ✅ **异步操作** - 处理需要时间的操作
2. ✅ **网络同步** - 客户端和服务器数据同步
3. ✅ **委托系统** - 灵活的事件通知
4. ✅ **可扩展性** - 易于创建自定义 Tasks
5. ✅ **蓝图支持** - 完整的蓝图集成

通过这个系统，可以处理复杂的异步操作和网络同步需求。

