# MVVM 系统文档

## 概述

MVVM（Model-View-ViewModel）系统用于加载屏幕的 UI 管理。系统使用 Unreal Engine 的 Model View ViewModel 插件，实现了数据绑定和视图模型模式，使 UI 逻辑与视图分离。

## 核心组件

### UMVVM_LoadScreen

加载屏幕的 ViewModel，管理所有存档槽的视图模型。

#### 类层次结构

```
UMVVMViewModelBase (UE5 Plugin)
    ↓
UMVVM_LoadScreen
```

#### 核心功能

1. **存档槽管理**
   - 创建和管理多个存档槽 ViewModel
   - 选择当前存档槽
   - 删除存档槽

2. **存档操作**
   - 新建存档
   - 加载存档
   - 删除存档
   - 开始游戏

#### 关键属性

```cpp
// 存档槽 ViewModel 类
UPROPERTY(EditDefaultsOnly)
TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

// 存档槽映射
UPROPERTY()
TMap<int32, UMVVM_LoadSlot*> LoadSlots;

// 选中的存档槽
UPROPERTY()
UMVVM_LoadSlot* SelectedSlot;

// 存档槽数量（数据绑定）
UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter)
int32 NumLoadSlots;
```

#### 关键方法

**初始化**:
```cpp
void InitializeLoadSlots();
void LoadData();
```

**存档槽操作**:
```cpp
UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;
void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);
void NewGameButtonPressed(int32 Slot);
void SelectSlotButtonPressed(int32 Slot);
void DeleteButtonPressed();
void PlayButtonPressed();
```

#### 委托

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

UPROPERTY(BlueprintAssignable)
FSlotSelected SlotSelected;
```

---

### UMVVM_LoadSlot

单个存档槽的 ViewModel，管理存档槽的数据和状态。

#### 核心功能

1. **存档数据**
   - 存档名称
   - 存档槽索引
   - 地图名称
   - 玩家等级
   - 存档时间

2. **状态管理**
   - 是否为空
   - 是否被选中
   - 是否应该显示

#### 关键属性

```cpp
// 存档槽索引
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
int32 SlotIndex;

// 玩家名称
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
FString PlayerName;

// 地图名称
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
FString MapName;

// 玩家等级
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
int32 PlayerLevel;

// 是否为空
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
bool bIsEmpty;

// 是否被选中
UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
bool bIsSelected;
```

#### 关键方法

**数据设置**:
```cpp
void SetPlayerName(const FString& InPlayerName);
void SetMapName(const FString& InMapName);
void SetPlayerLevel(int32 InPlayerLevel);
void SetIsEmpty(bool bInIsEmpty);
```

**数据加载**:
```cpp
void LoadSlotData();
```

---

## MVVM 数据绑定

### Field Notify

使用 `FieldNotify` 实现数据绑定：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter)
int32 NumLoadSlots;

// Setter
void SetNumLoadSlots(int32 InNumLoadSlots)
{
    if (NumLoadSlots != InNumLoadSlots)
    {
        NumLoadSlots = InNumLoadSlots;
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NumLoadSlots);
    }
}

// Getter
int32 GetNumLoadSlots() const { return NumLoadSlots; }
```

### 在蓝图中绑定

1. 在 Widget 中创建 ViewModel 引用
2. 使用 "Bind" 节点绑定属性
3. 设置绑定模式（One Way, Two Way 等）

---

## 使用流程

### 1. 初始化

```cpp
void UMVVM_LoadScreen::InitializeLoadSlots()
{
    // 创建存档槽 ViewModel
    for (int32 i = 0; i < NumLoadSlots; ++i)
    {
        UMVVM_LoadSlot* LoadSlot = NewObject<UMVVM_LoadSlot>(
            this, LoadSlotViewModelClass
        );
        LoadSlot->SetSlotIndex(i);
        LoadSlots.Add(i, LoadSlot);
    }
    
    // 加载存档数据
    LoadData();
}
```

### 2. 加载数据

```cpp
void UMVVM_LoadScreen::LoadData()
{
    // 从 GameMode 获取存档数据
    AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
    
    if (AuraGameMode)
    {
        // 加载每个存档槽的数据
        for (auto& Pair : LoadSlots)
        {
            ULoadScreenSaveGame* SaveGame = 
                AuraGameMode->GetSaveSlotData(Pair.Key);
            
            if (SaveGame)
            {
                Pair.Value->SetPlayerName(SaveGame->PlayerName);
                Pair.Value->SetMapName(SaveGame->MapName);
                Pair.Value->SetPlayerLevel(SaveGame->PlayerLevel);
                Pair.Value->SetIsEmpty(false);
            }
            else
            {
                Pair.Value->SetIsEmpty(true);
            }
        }
    }
}
```

### 3. 选择存档槽

```cpp
void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
    if (UMVVM_LoadSlot* LoadSlot = LoadSlots.FindRef(Slot))
    {
        // 取消之前的选择
        if (SelectedSlot)
        {
            SelectedSlot->SetIsSelected(false);
        }
        
        // 选择新的存档槽
        SelectedSlot = LoadSlot;
        SelectedSlot->SetIsSelected(true);
        
        // 广播选择事件
        SlotSelected.Broadcast();
    }
}
```

### 4. 新建存档

```cpp
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
    // 创建新存档
    AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
    
    if (AuraGameMode)
    {
        AuraGameMode->SaveSlotData(Slot, EnteredName);
        
        // 更新 ViewModel
        if (UMVVM_LoadSlot* LoadSlot = LoadSlots.FindRef(Slot))
        {
            LoadSlot->LoadSlotData();
        }
    }
}
```

---

## Widget 集成

### LoadScreenWidget

加载屏幕 Widget 使用 ViewModel：

```cpp
// 在 Widget 中
void ULoadScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 获取 ViewModel
    ViewModel = GetViewModel<UMVVM_LoadScreen>();
    
    // 绑定数据
    // PlayerNameText->BindTo(ViewModel->GetPlayerNameProperty());
    // MapNameText->BindTo(ViewModel->GetMapNameProperty());
}
```

---

## 相关文档

- [UI 系统](./UI_System.md) - UI 系统架构
- [Gameplay 框架](../Gameplay/Gameplay_Framework.md) - 存档系统

---

## 总结

MVVM 系统提供了：

1. ✅ **数据绑定** - 自动同步 ViewModel 和 View
2. ✅ **逻辑分离** - UI 逻辑与视图分离
3. ✅ **类型安全** - 使用 C++ 类型确保数据正确性
4. ✅ **易于维护** - 清晰的代码结构
5. ✅ **编辑器支持** - 完整的蓝图支持

通过这个系统，可以高效地管理复杂的 UI 状态和数据绑定。

