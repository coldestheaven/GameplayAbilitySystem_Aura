// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UPassiveNiagaraComponent;
class UDebuffNiagaraComponent;
class UNiagaraSystem;
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;

/**
 * 所有 Aura 角色的抽象基类（玩家角色与敌人角色均继承自此类）
 *
 * 职责：
 * - 持有并初始化 AbilitySystemComponent（ASC）和 AttributeSet
 * - 实现 ICombatInterface 战斗接口的通用逻辑（死亡、受击、插槽位置等）
 * - 管理角色的 Debuff 特效（燃烧、眩晕）和被动技能特效（光环）
 * - 提供溶解死亡动画的蓝图事件接口
 *
 * 使用示例：
 *   // 子类在 PossessedBy 或 BeginPlay 中调用 InitAbilityActorInfo() 完成 ASC 初始化
 *   // 然后调用 InitializeDefaultAttributes() 应用默认属性 GE
 *   // 最后调用 AddCharacterAbilities() 赋予初始技能
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual void Tick(float DeltaTime) override;

	/** 注册需要网络同步的属性（bIsStunned、bIsBurned、bIsBeingShocked） */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	/** 重写 TakeDamage，用于处理来自 GAS 之外的伤害（通常不走此路径，GAS 走 AttributeSet） */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	/** 实现 IAbilitySystemInterface，返回角色持有的 ASC */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取角色的属性集（AttributeSet） */
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** ======================== Combat Interface 实现 ======================== */

	/** 返回受击动画蒙太奇 */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	/**
	 * 角色死亡处理
	 * @param DeathImpulse 死亡时施加的物理冲量方向与大小，用于击飞效果
	 */
	virtual void Die(const FVector& DeathImpulse) override;

	/** 返回死亡委托引用，外部可绑定死亡回调 */
	virtual FOnDeathSignature& GetOnDeathDelegate() override;

	/**
	 * 根据 MontageTag 返回对应的战斗插槽世界位置（如武器尖端、左手、右手等）
	 * @param MontageTag 标识插槽类型的 GameplayTag
	 */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;

	/** 返回角色是否已死亡 */
	virtual bool IsDead_Implementation() const override;

	/** 返回角色的 Avatar Actor（即自身） */
	virtual AActor* GetAvatar_Implementation() override;

	/** 返回所有带标签的攻击蒙太奇数组 */
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;

	/** 返回死亡时播放的血液 Niagara 特效 */
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;

	/**
	 * 根据 MontageTag 查找并返回对应的 FTaggedMontage 结构体
	 * @param MontageTag 要查找的标签
	 */
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;

	/** 返回当前召唤物数量 */
	virtual int32 GetMinionCount_Implementation() override;

	/**
	 * 增加/减少召唤物计数
	 * @param Amount 正数增加，负数减少
	 */
	virtual void IncremenetMinionCount_Implementation(int32 Amount) override;

	/** 返回角色职业类型（战士、法师、游侠等） */
	virtual ECharacterClass GetCharacterClass_Implementation() override;

	/** 返回 ASC 注册完成委托的引用，供外部监听 ASC 初始化完成事件 */
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;

	/** 返回武器骨骼网格体组件 */
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;

	/**
	 * 设置角色是否正在被电击（Shock 状态）
	 * @param bInShock true 表示进入电击状态，false 表示退出
	 */
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;

	/** 返回角色当前是否处于电击状态 */
	virtual bool IsBeingShocked_Implementation() const override;

	/** 返回受到伤害时的委托引用，可用于绑定伤害数值显示等回调 */
	virtual FOnDamageSignature& GetOnDamageSignature() override;

	/** ======================== end Combat Interface ======================== */

	/** ASC 注册完成时广播（服务器和客户端均会触发） */
	FOnASCRegistered OnAscRegistered;

	/** 角色死亡时广播 */
	FOnDeathSignature OnDeathDelegate;

	/** 角色受到伤害时广播，携带伤害数值 */
	FOnDamageSignature OnDamageDelegate;

	/**
	 * 多播 RPC：在所有客户端上执行死亡逻辑（布娃娃、溶解特效、死亡音效等）
	 * @param DeathImpulse 死亡冲量，用于布娃娃物理
	 */
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	/**
	 * 带标签的攻击蒙太奇数组
	 * 每个元素包含一个 AnimMontage 和对应的 GameplayTag（用于标识攻击类型和插槽）
	 * 在蓝图或 Details 面板中配置
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

	/** 是否处于眩晕状态（网络同步，变化时触发 OnRep_Stunned） */
	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;

	/** 是否处于燃烧状态（网络同步，变化时触发 OnRep_Burned） */
	UPROPERTY(ReplicatedUsing=OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;

	/** 是否正在被电击（网络同步，由 SetIsBeingShocked_Implementation 控制） */
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBeingShocked = false;

	/** 眩晕状态同步回调：在客户端更新眩晕特效和移动速度 */
	UFUNCTION()
	virtual void OnRep_Stunned();

	/** 燃烧状态同步回调：在客户端更新燃烧特效 */
	UFUNCTION()
	virtual void OnRep_Burned();

	/** 设置角色职业类型（通常由 SpawnPoint 在生成时调用） */
	void SetCharacterClass(ECharacterClass InClass) { CharacterClass = InClass; }

protected:
	virtual void BeginPlay() override;

	/**
	 * 武器骨骼网格体
	 * 挂载在角色骨骼的武器插槽上，用于显示武器模型和提供战斗插槽位置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	/** 武器尖端插槽名称（用于投射物生成位置） */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	/** 左手插槽名称（用于近战左手攻击位置） */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	/** 右手插槽名称（用于近战右手攻击位置） */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;

	/** 尾部插槽名称（用于特殊怪物的尾部攻击位置） */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName;

	/** 角色是否已死亡（本地标记，不参与网络同步，由 MulticastHandleDeath 设置） */
	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	/**
	 * 眩晕标签变化回调
	 * 当 ASC 上的眩晕 GameplayTag 数量发生变化时调用
	 * @param CallbackTag 触发回调的标签
	 * @param NewCount 当前标签数量（0 表示眩晕结束，>0 表示眩晕中）
	 */
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/** 基础行走速度（眩晕时会被设为 0，恢复时还原此值） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 600.f;

	/** 能力系统组件（GAS 核心，管理技能、属性、效果） */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 属性集（存储所有 GameplayAttribute，如生命值、法力值、攻击力等） */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/**
	 * 初始化 AbilityActorInfo
	 * 玩家角色在 PossessedBy/OnRep_PlayerState 中调用，敌人在 BeginPlay 中调用
	 * 必须在 ASC 可用后立即调用，否则技能无法正常工作
	 */
	virtual void InitAbilityActorInfo();

	/** 主要属性初始化 GameplayEffect（力量、智力、韧性、活力） */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	/** 次要属性初始化 GameplayEffect（护甲、暴击率、格挡率等，由主属性派生） */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	/** 生命值/法力值初始化 GameplayEffect（将 Health/Mana 设为最大值） */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	/**
	 * 将指定 GameplayEffect 应用到自身
	 * @param GameplayEffectClass 要应用的 GE 类
	 * @param Level 应用时使用的等级（影响 ScalableFloat 曲线取值）
	 */
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	/**
	 * 按顺序应用三组默认属性 GE（主属性 → 次属性 → 生命/法力）
	 * 子类可重写以实现不同的初始化逻辑（如从存档加载）
	 */
	virtual void InitializeDefaultAttributes() const;

	/** 从 StartupAbilities 和 StartupPassiveAbilities 数组赋予角色初始技能 */
	void AddCharacterAbilities();

	/* ======================== 溶解死亡特效 ======================== */

	/** 触发角色身体和武器的溶解动画 */
	void Dissolve();

	/**
	 * 蓝图事件：启动角色身体溶解时间轴
	 * @param DynamicMaterialInstance 动态材质实例，蓝图中通过修改其参数驱动溶解效果
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/**
	 * 蓝图事件：启动武器溶解时间轴
	 * @param DynamicMaterialInstance 武器的动态材质实例
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/** 角色身体溶解材质（需要包含溶解参数的材质实例） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	/** 武器溶解材质 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	/** 死亡时播放的血液/受击 Niagara 粒子特效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UNiagaraSystem* BloodEffect;

	/** 死亡时播放的音效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;

	/* ======================== 召唤物管理 ======================== */

	/** 当前存活的召唤物数量（由 IncremenetMinionCount_Implementation 维护） */
	int32 MinionCount = 0;

	/**
	 * 角色职业类型（默认为战士）
	 * 决定初始属性曲线、可用技能列表和 XP 奖励
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	/** 燃烧 Debuff Niagara 组件（当 bIsBurned 为 true 时激活） */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	/** 眩晕 Debuff Niagara 组件（当 bIsStunned 为 true 时激活） */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
private:

	/**
	 * 初始主动技能列表
	 * 在 AddCharacterAbilities() 中赋予角色，通常包含攻击、技能等主动能力
	 */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/**
	 * 初始被动技能列表
	 * 在 AddCharacterAbilities() 中赋予角色，持续生效的被动效果（如光环、生命恢复等）
	 */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	/** 受击动画蒙太奇（被攻击时播放的反应动画） */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/** 被动技能"保护光环"的 Niagara 组件（装备对应被动技能时激活） */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	/** 被动技能"生命虹吸"的 Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	/** 被动技能"法力虹吸"的 Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	/** 所有被动特效 Niagara 组件的挂载根节点（跟随角色移动但不随骨骼旋转） */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
};
