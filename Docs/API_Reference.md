# API 参考文档

## 核心类 API

### UAuraAbilitySystemComponent

#### 能力管理

```cpp
// 添加角色能力
void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

// 添加被动能力
void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

// 从存档加载能力
void AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData);
```

#### 输入处理

```cpp
// 输入按下
void AbilityInputTagPressed(const FGameplayTag& InputTag);

// 输入按住
void AbilityInputTagHeld(const FGameplayTag& InputTag);

// 输入释放
void AbilityInputTagReleased(const FGameplayTag& InputTag);
```

#### 能力查询

```cpp
// 获取能力标签
static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

// 获取输入标签
static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

// 获取状态标签
static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);

// 根据能力标签获取 Spec
FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
```

#### 能力装备

```cpp
// 服务器端装备能力
UFUNCTION(Server, Reliable)
void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);

// 客户端装备能力回调
UFUNCTION(Client, Reliable)
void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, 
                        const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
```

#### 属性升级

```cpp
// 升级属性
void UpgradeAttribute(const FGameplayTag& AttributeTag);

// 服务器端升级属性
UFUNCTION(Server, Reliable)
void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);
```

#### 委托

```cpp
// 效果资产标签委托
FEffectAssetTags EffectAssetTags;

// 能力已给予委托
FAbilitiesGiven AbilitiesGivenDelegate;

// 能力状态改变委托
FAbilityStatusChanged AbilityStatusChanged;

// 能力装备委托
FAbilityEquipped AbilityEquipped;

// 被动能力激活委托
FActivatePassiveEffect ActivatePassiveEffect;
```

---

### UAuraAttributeSet

#### 属性访问器宏

所有属性都使用 `ATTRIBUTE_ACCESSORS` 宏，提供：
- `Get{PropertyName}()` - 获取当前值
- `Get{PropertyName}Attribute()` - 获取属性
- `Set{PropertyName}(float)` - 设置值
- `Init{PropertyName}(float)` - 初始化值

#### 主属性

```cpp
FGameplayAttributeData Strength;
FGameplayAttributeData Intelligence;
FGameplayAttributeData Resilience;
FGameplayAttributeData Vigor;
```

#### 次属性

```cpp
FGameplayAttributeData Armor;
FGameplayAttributeData ArmorPenetration;
FGameplayAttributeData BlockChance;
FGameplayAttributeData CriticalHitChance;
FGameplayAttributeData CriticalHitDamage;
FGameplayAttributeData CriticalHitResistance;
FGameplayAttributeData HealthRegeneration;
FGameplayAttributeData ManaRegeneration;
FGameplayAttributeData MaxHealth;
FGameplayAttributeData MaxMana;
```

#### 抗性属性

```cpp
FGameplayAttributeData FireResistance;
FGameplayAttributeData LightningResistance;
FGameplayAttributeData ArcaneResistance;
FGameplayAttributeData PhysicalResistance;
```

#### 生命值属性

```cpp
FGameplayAttributeData Health;
FGameplayAttributeData Mana;
```

#### 元属性

```cpp
FGameplayAttributeData IncomingDamage;
FGameplayAttributeData IncomingXP;
```

#### 回调函数

```cpp
// 属性改变前
virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

// 属性改变后
virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

// 效果执行后
virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
```

---

### UAuraGameplayAbility

#### 描述方法

```cpp
// 获取能力描述
virtual FString GetDescription(int32 Level);

// 获取下一级描述
virtual FString GetNextLevelDescription(int32 Level);

// 获取锁定描述
static FString GetLockedDescription(int32 Level);
```

#### 成本查询

```cpp
// 获取法力消耗
float GetManaCost(float InLevel = 1.f) const;

// 获取冷却时间
float GetCooldown(float InLevel = 1.f) const;
```

#### 属性

```cpp
// 启动输入标签
UPROPERTY(EditDefaultsOnly, Category="Input")
FGameplayTag StartupInputTag;
```

---

### UAuraDamageGameplayAbility

#### 伤害方法

```cpp
// 造成伤害
UFUNCTION(BlueprintCallable)
void CauseDamage(AActor* TargetActor);

// 制作伤害效果参数
UFUNCTION(BlueprintPure)
FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(
    AActor* TargetActor = nullptr,
    FVector InRadialDamageOrigin = FVector::ZeroVector,
    bool bOverrideKnockbackDirection = false,
    FVector KnockbackDirectionOverride = FVector::ZeroVector,
    bool bOverrideDeathImpulse = false,
    FVector DeathImpulseDirectionOverride = FVector::ZeroVector,
    bool bOverridePitch = false,
    float PitchOverride = 0.f) const;

// 获取当前等级伤害
UFUNCTION(BlueprintPure)
float GetDamageAtLevel() const;
```

#### 伤害属性

```cpp
// 伤害类型
UPROPERTY(EditDefaultsOnly, Category = "Damage")
FGameplayTag DamageType;

// 伤害值（可扩展）
UPROPERTY(EditDefaultsOnly, Category = "Damage")
FScalableFloat Damage;

// Debuff 相关
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float DebuffChance = 20.f;
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float DebuffDamage = 5.f;
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float DebuffFrequency = 1.f;
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float DebuffDuration = 5.f;

// 击退和死亡冲量
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float DeathImpulseMagnitude = 1000.f;
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float KnockbackForceMagnitude = 1000.f;
UPROPERTY(EditDefaultsOnly, Category = "Damage")
float KnockbackChance = 0.f;

// 范围伤害
UPROPERTY(EditDefaultsOnly, Category = "Damage")
bool bIsRadialDamage = false;
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
float RadialDamageInnerRadius = 0.f;
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
float RadialDamageOuterRadius = 0.f;
```

---

### AAuraCharacterBase

#### 能力系统接口

```cpp
// 获取能力系统组件
virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

// 获取属性集
UAttributeSet* GetAttributeSet() const;
```

#### 战斗接口实现

```cpp
// 获取战斗插槽位置
virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;

// 获取受击动画
virtual UAnimMontage* GetHitReactMontage_Implementation() override;

// 死亡
virtual void Die(const FVector& DeathImpulse) override;

// 是否死亡
virtual bool IsDead_Implementation() const override;

// 获取攻击动画
virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
```

#### 初始化方法

```cpp
// 初始化能力 Actor 信息
virtual void InitAbilityActorInfo();

// 初始化默认属性
virtual void InitializeDefaultAttributes() const;

// 添加角色能力
void AddCharacterAbilities();
```

#### 效果应用

```cpp
// 对自己应用效果
void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
```

#### 委托

```cpp
// ASC 注册委托
FOnASCRegistered OnAscRegistered;

// 死亡委托
FOnDeathSignature OnDeathDelegate;

// 伤害委托
FOnDamageSignature OnDamageDelegate;
```

---

### AAuraPlayerState

#### 状态数据

```cpp
// 获取玩家等级
FORCEINLINE int32 GetPlayerLevel() const;

// 获取经验值
FORCEINLINE int32 GetXP() const;

// 获取属性点
FORCEINLINE int32 GetAttributePoints() const;

// 获取法术点
FORCEINLINE int32 GetSpellPoints() const;
```

#### 状态修改

```cpp
// 增加经验
void AddToXP(int32 InXP);

// 增加等级
void AddToLevel(int32 InLevel);

// 增加属性点
void AddToAttributePoints(int32 InPoints);

// 增加法术点
void AddToSpellPoints(int32 InPoints);

// 设置经验
void SetXP(int32 InXP);

// 设置等级
void SetLevel(int32 InLevel);
```

#### 委托

```cpp
// 经验值改变委托
FOnPlayerStatChanged OnXPChangedDelegate;

// 等级改变委托
FOnLevelChanged OnLevelChangedDelegate;

// 属性点改变委托
FOnPlayerStatChanged OnAttributePointsChangedDelegate;

// 法术点改变委托
FOnPlayerStatChanged OnSpellPointsChangedDelegate;
```

---

### UAuraWidgetController

#### 初始化

```cpp
// 设置 Widget Controller 参数
UFUNCTION(BlueprintCallable)
void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

// 广播初始值
UFUNCTION(BlueprintCallable)
virtual void BroadcastInitialValues();

// 绑定回调到依赖
virtual void BindCallbacksToDependencies();
```

#### 能力信息

```cpp
// 广播能力信息
void BroadcastAbilityInfo();

// 能力信息委托
UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
FAbilityInfoSignature AbilityInfoDelegate;
```

#### 辅助方法

```cpp
// 获取 Aura Player Controller
AAuraPlayerController* GetAuraPC();

// 获取 Aura Player State
AAuraPlayerState* GetAuraPS();

// 获取 Aura Ability System Component
UAuraAbilitySystemComponent* GetAuraASC();

// 获取 Aura Attribute Set
UAuraAttributeSet* GetAuraAS();
```

---

### UAuraAbilitySystemLibrary

#### Widget Controller 工具

```cpp
// 制作 Widget Controller 参数
static bool MakeWidgetControllerParams(const UObject* WorldContextObject, 
                                       FWidgetControllerParams& OutWCParams, 
                                       AAuraHUD*& OutAuraHUD);

// 获取覆盖层 Widget Controller
static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

// 获取属性菜单 Widget Controller
static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

// 获取法术菜单 Widget Controller
static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);
```

#### 能力信息

```cpp
// 获取能力信息数据资产
static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
```

#### 伤害相关

```cpp
// 是否为范围伤害
static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);

// 获取范围伤害原点
static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);

// 获取范围伤害内半径
static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);

// 获取范围伤害外半径
static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);
```

---

### FAuraGameplayTags

#### 单例访问

```cpp
// 获取 Gameplay Tags 单例
static const FAuraGameplayTags& Get();

// 初始化原生 Gameplay Tags
static void InitializeNativeGameplayTags();
```

#### 属性标签

```cpp
// 主属性
FGameplayTag Attributes_Primary_Strength;
FGameplayTag Attributes_Primary_Intelligence;
FGameplayTag Attributes_Primary_Resilience;
FGameplayTag Attributes_Primary_Vigor;

// 次属性
FGameplayTag Attributes_Secondary_Armor;
FGameplayTag Attributes_Secondary_CriticalHitChance;
// ... 更多次属性

// 抗性
FGameplayTag Attributes_Resistance_Fire;
FGameplayTag Attributes_Resistance_Lightning;
FGameplayTag Attributes_Resistance_Arcane;
FGameplayTag Attributes_Resistance_Physical;
```

#### 输入标签

```cpp
FGameplayTag InputTag_LMB;
FGameplayTag InputTag_RMB;
FGameplayTag InputTag_1;
FGameplayTag InputTag_2;
FGameplayTag InputTag_3;
FGameplayTag InputTag_4;
FGameplayTag InputTag_Passive_1;
FGameplayTag InputTag_Passive_2;
```

#### 能力标签

```cpp
FGameplayTag Abilities_Attack;
FGameplayTag Abilities_Summon;
FGameplayTag Abilities_Status_Locked;
FGameplayTag Abilities_Status_Eligible;
FGameplayTag Abilities_Status_Unlocked;
FGameplayTag Abilities_Status_Equipped;
```

#### 伤害标签

```cpp
FGameplayTag Damage;
FGameplayTag Damage_Fire;
FGameplayTag Damage_Lightning;
FGameplayTag Damage_Arcane;
FGameplayTag Damage_Physical;
```

#### Debuff 标签

```cpp
FGameplayTag Debuff_Burn;
FGameplayTag Debuff_Stun;
FGameplayTag Debuff_Arcane;
FGameplayTag Debuff_Physical;
```

---

### 数据结构

#### FDamageEffectParams

```cpp
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
    TObjectPtr<UObject> WorldContextObject;
    TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;
    TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;
    TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;
    float BaseDamage;
    float AbilityLevel;
    FGameplayTag DamageType;
    float DebuffChance;
    float DebuffDamage;
    float DebuffDuration;
    float DebuffFrequency;
    float DeathImpulseMagnitude;
    FVector DeathImpulse;
    float KnockbackForceMagnitude;
    float KnockbackChance;
    FVector KnockbackForce;
    bool bIsRadialDamage;
    float RadialDamageInnerRadius;
    float RadialDamageOuterRadius;
    FVector RadialDamageOrigin;
};
```

#### FWidgetControllerParams

```cpp
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
    TObjectPtr<APlayerController> PlayerController;
    TObjectPtr<APlayerState> PlayerState;
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    TObjectPtr<UAttributeSet> AttributeSet;
};
```

#### FAuraAbilityInfo

```cpp
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
    FGameplayTag AbilityTag;
    FGameplayTag InputTag;
    FGameplayTag StatusTag;
    FGameplayTag CooldownTag;
    FGameplayTag AbilityType;
    TObjectPtr<const UTexture2D> Icon;
    TObjectPtr<const UMaterialInterface> BackgroundMaterial;
    int32 LevelRequirement;
    TSubclassOf<UGameplayAbility> Ability;
};
```

