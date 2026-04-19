// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/AnimationInterface.h"
#include "Interaction/SummonInterface.h"
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
 * - 实现 ICombatInterface 核心战斗接口
 * - 实现 IAnimationInterface 动画查询接口
 * - 实现 ISummonInterface 召唤物管理接口
 * - 管理角色的 Debuff 特效（燃烧、眩晕）和被动技能特效（光环）
 * - 提供溶解死亡动画的蓝图事件接口
 *
 * 接口职责分离：
 * - ICombatInterface：核心战斗（Die、GetPlayerLevel、GetCombatSocketLocation 等）
 * - IAnimationInterface：动画查询（GetHitReactMontage、GetAttackMontages 等）
 * - ISummonInterface：召唤物管理（GetMinionCount、IncrementMinionCount）
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface, public IAnimationInterface, public ISummonInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual void Tick(float DeltaTime) override;

	/** 注册需要网络同步的属性（bIsStunned、bIsBurned、bIsBeingShocked） */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	/** 重写 TakeDamage，用于处理来自 GAS 之外的伤害 */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	/** 实现 IAbilitySystemInterface，返回角色持有的 ASC */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取角色的属性集（AttributeSet） */
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** ======================== ICombatInterface 实现 ======================== */

	/** 返回角色等级 */
	virtual int32 GetPlayerLevel_Implementation() override { return 0; }

	/**
	 * 角色死亡处理
	 * @param DeathImpulse 死亡时施加的物理冲量
	 */
	virtual void Die(const FVector& DeathImpulse) override;

	/** 返回死亡委托引用 */
	virtual FOnDeathSignature& GetOnDeathDelegate() override;

	/**
	 * 根据 MontageTag 返回对应的战斗插槽世界位置
	 * @param MontageTag 标识插槽类型的 GameplayTag
	 */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;

	/** 返回角色是否已死亡 */
	virtual bool IsDead_Implementation() const override;

	/** 返回角色的 Avatar Actor（即自身） */
	virtual AActor* GetAvatar_Implementation() override;

	/** 返回死亡时播放的血液 Niagara 特效 */
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;

	/** 返回角色职业类型 */
	virtual ECharacterClass GetCharacterClass_Implementation() override;

	/** 返回 ASC 注册完成委托的引用 */
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;

	/** 返回武器骨骼网格体组件 */
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;

	/**
	 * 设置角色是否正在被电击
	 * @param bInShock true 表示进入电击状态
	 */
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;

	/** 返回角色当前是否处于电击状态 */
	virtual bool IsBeingShocked_Implementation() const override;

	/** 返回受到伤害时的委托引用 */
	virtual FOnDamageSignature& GetOnDamageSignature() override;

	/** ======================== end ICombatInterface ======================== */

	/** ======================== IAnimationInterface 实现 ======================== */

	/** 返回受击动画蒙太奇 */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	/** 返回所有带标签的攻击蒙太奇数组 */
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;

	/**
	 * 根据 MontageTag 查找并返回对应的 FTaggedMontage 结构体
	 * @param MontageTag 要查找的标签
	 */
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;

	/** ======================== end IAnimationInterface ======================== */

	/** ======================== ISummonInterface 实现 ======================== */

	/** 返回当前召唤物数量 */
	virtual int32 GetMinionCount_Implementation() override;

	/**
	 * 增加/减少召唤物计数
	 * @param Amount 正数增加，负数减少
	 */
	virtual void IncrementMinionCount_Implementation(int32 Amount) override;

	/** ======================== end ISummonInterface ======================== */

	/** ASC 注册完成时广播 */
	FOnASCRegistered OnAscRegistered;

	/** 角色死亡时广播 */
	FOnDeathSignature OnDeathDelegate;

	/** 角色受到伤害时广播 */
	FOnDamageSignature OnDamageDelegate;

	/**
	 * 多播 RPC：在所有客户端上执行死亡逻辑
	 * @param DeathImpulse 死亡冲量，用于布娃娃物理
	 */
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	/**
	 * 带标签的攻击蒙太奇数组
	 * 在蓝图或 Details 面板中配置
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

	/** 是否处于眩晕状态（网络同步） */
	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;

	/** 是否处于燃烧状态（网络同步） */
	UPROPERTY(ReplicatedUsing=OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;

	/** 是否正在被电击（网络同步） */
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBeingShocked = false;

	/** 眩晕状态同步回调 */
	UFUNCTION()
	virtual void OnRep_Stunned();

	/** 燃烧状态同步回调 */
	UFUNCTION()
	virtual void OnRep_Burned();

	/** 设置角色职业类型 */
	void SetCharacterClass(ECharacterClass InClass) { CharacterClass = InClass; }

protected:
	virtual void BeginPlay() override;

	/** 武器骨骼网格体 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	/** 武器尖端插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	/** 左手插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	/** 右手插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;

	/** 尾部插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName;

	/** 角色是否已死亡 */
	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	/**
	 * 眩晕标签变化回调
	 * @param CallbackTag 触发回调的标签
	 * @param NewCount 当前标签数量
	 */
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/** 基础行走速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 600.f;

	/** 能力系统组件 */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 属性集 */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** 初始化 AbilityActorInfo */
	virtual void InitAbilityActorInfo();

	/** 主要属性初始化 GE */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	/** 次要属性初始化 GE */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	/** 生命值/法力值初始化 GE */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	/**
	 * 将指定 GE 应用到自身
	 * @param GameplayEffectClass 要应用的 GE 类
	 * @param Level 应用时使用的等级
	 */
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	/** 按顺序应用三组默认属性 GE */
	virtual void InitializeDefaultAttributes() const;

	/** 赋予角色初始技能 */
	void AddCharacterAbilities();

	/* ======================== 溶解死亡特效 ======================== */

	/** 触发角色身体和武器的溶解动画 */
	void Dissolve();

	/** 蓝图事件：启动角色身体溶解时间轴 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/** 蓝图事件：启动武器溶解时间轴 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/** 角色身体溶解材质 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	/** 武器溶解材质 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	/** 死亡时播放的血液 Niagara 粒子特效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UNiagaraSystem* BloodEffect;

	/** 死亡时播放的音效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;

	/* ======================== 召唤物管理 ======================== */

	/** 当前存活的召唤物数量 */
	int32 MinionCount = 0;

	/** 角色职业类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	/** 燃烧 Debuff Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	/** 眩晕 Debuff Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
private:

	/** 初始主动技能列表 */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/** 初始被动技能列表 */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	/** 受击动画蒙太奇 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/** 被动技能"保护光环"的 Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	/** 被动技能"生命虹吸"的 Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	/** 被动技能"法力虹吸"的 Niagara 组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	/** 所有被动特效 Niagara 组件的挂载根节点 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
};
