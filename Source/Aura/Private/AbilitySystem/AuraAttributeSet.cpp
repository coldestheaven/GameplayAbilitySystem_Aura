// Copyright Druid Mechanics


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "GameFramework/Character.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraEffectContextLibrary.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Player/AuraPlayerController.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	/**
	 * 构造函数：初始化属性集
	 * 建立 GameplayTag 到 FGameplayAttribute 的映射表，用于通过 Tag 动态查找属性
	 */
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	// 主属性映射（力量、智力、韧性、活力）
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);

	// 次属性映射（护甲、暴击率、格挡率等）
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);	
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
	
	// 抗性属性映射（火焰、闪电、奥术、物理抗性）
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	/**
	 * 注册需要网络同步的属性
	 * 所有属性都使用 COND_None（无条件同步）和 REPNOTIFY_Always（总是触发 OnRep 回调）
	 * 确保 GAS 预测系统正确处理属性变化
	 */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 主属性（力量、智力、韧性、活力）
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	// 次属性（护甲、暴击率、格挡率等）
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	// 抗性属性（火焰、闪电、奥术、物理抗性）
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
	
	// 生命/法力属性（当前值）
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	/**
	 * 属性变化前的钳制处理
	 * 在 GE 修改属性之前调用，用于限制属性值范围
	 * 确保生命值和法力值不会超过最大值，也不会低于 0
	 */
	Super::PreAttributeChange(Attribute, NewValue);

	// 钳制生命值范围：0 ~ MaxHealth
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	// 钳制法力值范围：0 ~ MaxMana
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	/**
	 * 从 GE 回调数据中提取并填充 FEffectProperties 结构体
	 * 
	 * Source（效果来源）：
	 *   - SourceASC：来源的 ASC
	 *   - SourceAvatarActor：来源的 Avatar Actor（通常是角色 Pawn）
	 *   - SourceController：来源的控制器（玩家控制器或 AI 控制器）
	 *   - SourceCharacter：来源的角色（ACharacter 类型）
	 * 
	 * Target（效果目标，即此 AttributeSet 的所有者）：
	 *   - TargetASC：目标的 ASC（即此 AttributeSet 所属的 ASC）
	 *   - TargetAvatarActor：目标的 Avatar Actor
	 *   - TargetController：目标的控制器
	 *   - TargetCharacter：目标的角色
	 */
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	// 提取来源信息
	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		
		// 如果 ASC 中没有控制器，尝试从 Pawn 获取
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		
		// 从控制器获取角色
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	// 提取目标信息（此 AttributeSet 的所有者）
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	/**
	 * GameplayEffect 执行完成后的回调（核心伤害/治疗处理入口）
	 * 
	 * 流程：
	 *   1. 提取效果来源和目标信息
	 *   2. 如果目标已死亡，直接返回
	 *   3. 钳制生命值和法力值范围
	 *   4. 处理传入伤害（IncomingDamage > 0）
	 *   5. 处理传入经验值（IncomingXP > 0）
	 */
	Super::PostGameplayEffectExecute(Data);
	
	// 提取效果来源和目标信息
	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	// 如果目标已死亡，不处理后续逻辑
	if(Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter)) return;

	// 钳制生命值和法力值范围（确保在有效范围内）
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	
	// 处理传入伤害（由 ExecCalc_Damage 计算后写入）
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}
	
	// 处理传入经验值（击杀敌人后写入）
	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props);
	}
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	/**
	 * 处理传入伤害（IncomingDamage > 0 时调用）
	 * 
	 * 流程：
	 *   1. 读取并清零 IncomingDamage（防止重复处理）
	 *   2. 扣除生命值
	 *   3. 检测是否致命伤害：
	 *      - 致命：触发死亡逻辑，发送 XP 事件
	 *      - 非致命：触发受击反应，应用击退力
	 *   4. 显示浮动伤害数字
	 *   5. 如果成功触发 Debuff，应用 Debuff 效果
	 */
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);  // 立即清零，防止重复处理
	
	if (LocalIncomingDamage > 0.f)
	{
		// 扣除生命值
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		const bool bFatal = NewHealth <= 0.f;
		if (bFatal)
		{
			// 致命伤害：触发死亡逻辑
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			if (CombatInterface)
			{
				CombatInterface->Die(UAuraEffectContextLibrary::GetDeathImpulse(Props.EffectContextHandle));
			}
			// 发送 XP 事件给击杀者
			SendXPEvent(Props);
		}
		else
		{
			// 非致命伤害：触发受击反应和击退
			// 如果目标不在电击状态，触发受击反应动画
			if (Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter))
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);  // 激活受击反应技能
			}
			
			// 应用击退力（如果有）
			const FVector& KnockbackForce = UAuraEffectContextLibrary::GetKnockbackForce(Props.EffectContextHandle);
			if (!KnockbackForce.IsNearlyZero(1.f))
			{
				Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			}
		}
		
		// 显示浮动伤害数字（格挡/暴击会有不同显示）
		const bool bBlock = UAuraEffectContextLibrary::IsBlockedHit(Props.EffectContextHandle);
		const bool bCriticalHit = UAuraEffectContextLibrary::IsCriticalHit(Props.EffectContextHandle);
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
		
		// 如果成功触发 Debuff，应用 Debuff 效果
		if (UAuraEffectContextLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
		{
			Debuff(Props);
		}
	}
}

void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
	/**
	 * 应用 Debuff 效果
	 * 
	 * 根据 GE 上下文中的 Debuff 参数（类型、伤害、持续时间、频率）创建并应用 Debuff GE
	 * 
	 * 流程：
	 *   1. 从上下文获取 Debuff 参数（伤害类型、伤害值、持续时间、频率）
	 *   2. 动态创建 GameplayEffect（使用临时包，不保存到磁盘）
	 *   3. 配置 GE 属性（持续时间、周期、堆叠类型）
	 *   4. 添加 Debuff 标签（如 Debuff_Burn、Debuff_Stun）
	 *   5. 如果是眩晕 Debuff，添加输入阻止标签
	 *   6. 配置修改器（修改 IncomingDamage 属性）
	 *   7. 应用 GE 到目标
	 */
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	// 创建 GE 上下文并设置来源对象
	FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(Props.SourceAvatarActor);

	// 从上下文获取 Debuff 参数
	const FGameplayTag DamageType = UAuraEffectContextLibrary::GetDamageType(Props.EffectContextHandle);
	const float DebuffDamage = UAuraEffectContextLibrary::GetDebuffDamage(Props.EffectContextHandle);
	const float DebuffDuration = UAuraEffectContextLibrary::GetDebuffDuration(Props.EffectContextHandle);
	const float DebuffFrequency = UAuraEffectContextLibrary::GetDebuffFrequency(Props.EffectContextHandle);

	// 动态创建 GameplayEffect（使用临时包，不保存到磁盘）
	FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageType.ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));

	// 配置 GE 持续时间策略（有持续时间，周期触发）
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	Effect->Period = DebuffFrequency;  // 每隔此时间触发一次伤害
	Effect->DurationMagnitude = FScalableFloat(DebuffDuration);  // 总持续时间

	// 添加 Debuff 标签（根据伤害类型确定对应的 Debuff 标签）
	const FGameplayTag DebuffTag = GameplayTags.DamageTypesToDebuffs[DamageType];
	Effect->InheritableOwnedTagsContainer.AddTag(DebuffTag);
	
	// 如果是眩晕 Debuff，添加输入阻止标签（阻止玩家输入）
	if (DebuffTag.MatchesTagExact(GameplayTags.Debuff_Stun))
	{
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_CursorTrace);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputHeld);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputPressed);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputReleased);
	}

	// 配置堆叠类型（按来源聚合，最多堆叠 1 层）
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	Effect->StackLimitCount = 1;

	// 添加修改器（修改 IncomingDamage 属性，每次触发时造成伤害）
	const int32 Index = Effect->Modifiers.Num();
	Effect->Modifiers.Add(FGameplayModifierInfo());
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];

	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);  // 每次触发的伤害值
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;  // 加法修改
	ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();  // 修改 IncomingDamage 属性
	
	// 创建 GE 规格并应用
	if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f))
	{
		// 设置伤害类型到上下文（用于后续伤害计算）
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
		TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));
		AuraContext->SetDamageType(DebuffDamageType);

		// 应用 Debuff GE 到目标
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	/**
	 * 处理传入经验值（IncomingXP > 0 时调用）
	 * 
	 * 注意：SourceCharacter 是击杀者（获得 XP 的玩家），因为 GA_ListenForEvents 应用 GE_EventBasedEffect 时
	 * 将击杀者设置为 Source，将经验值写入击杀者的 IncomingXP
	 * 
	 * 流程：
	 *   1. 读取并清零 IncomingXP
	 *   2. 计算新等级（根据当前 XP + 获得的 XP）
	 *   3. 如果升级：
	 *      - 增加等级
	 *      - 计算并分配属性点和技能点奖励
	 *      - 标记需要补满生命和法力（升级时自动补满）
	 *      - 触发升级逻辑（播放特效等）
	 *   4. 增加 XP（无论是否升级都要增加）
	 */
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0.f);  // 立即清零，防止重复处理

	// SourceCharacter 是击杀者（获得 XP 的玩家）
	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

		// 计算新等级（根据当前 XP + 获得的 XP）
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		
		if (NumLevelUps > 0)
		{
			// 升级：增加等级
			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);

			// 计算升级奖励（属性点和技能点）
			int32 AttributePointsReward = 0;
			int32 SpellPointsReward = 0;

			// 累加所有升级等级的奖励
			for (int32 i = 0; i < NumLevelUps; ++i)
			{
				SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel + i);
				AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel + i);
			}
			
			// 分配奖励
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);
	
			// 标记需要补满生命和法力（升级时自动补满）
			bTopOffHealth = true;
			bTopOffMana = true;
				
			// 触发升级逻辑（播放升级特效等）
			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
		}
		
		// 增加 XP（无论是否升级都要增加）
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	/**
	 * 属性变化后的回调
	 * 
	 * 用于处理最大值变化时同步当前值：
	 * - 当最大生命值提升时，如果 bTopOffHealth=true，将当前生命值补满
	 * - 当最大法力值提升时，如果 bTopOffMana=true，将当前法力值补满
	 * 
	 * 这通常发生在升级时，确保升级后生命和法力自动补满
	 */
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 最大生命值变化时，如果标记需要补满，则补满当前生命值
	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;  // 清除标记
	}
	// 最大法力值变化时，如果标记需要补满，则补满当前法力值
	if (Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;  // 清除标记
	}
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	/**
	 * 发送 XP 获取事件（通过 GameplayEvent 通知 PlayerInterface）
	 * 
	 * 当目标死亡时调用，计算 XP 奖励并发送 GameplayEvent 给击杀者
	 * 击杀者的 GA_ListenForEvents 技能会监听此事件，应用 GE_EventBasedEffect 增加 IncomingXP
	 * 
	 * XP 奖励计算：
	 *   - 根据目标的职业类型和等级查找对应的 XP 奖励曲线
	 *   - 通过 CharacterClassInfo 数据资产获取奖励值
	 */
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		// 获取目标的等级和职业类型
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
		
		// 根据目标的职业和等级计算 XP 奖励
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

		// 发送 GameplayEvent 给击杀者（SourceCharacter）
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;  // 事件标签
		Payload.EventMagnitude = XPReward;  // XP 奖励数值
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	}
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const
{
	/**
	 * 在目标位置显示浮动伤害数字
	 * 
	 * 显示逻辑：
	 *   - 如果来源是玩家，在来源的 PlayerController 上显示
	 *   - 否则，如果目标是玩家，在目标的 PlayerController 上显示
	 *   - 如果来源和目标相同（自己对自己造成伤害），不显示
	 * 
	 * 显示类型：
	 *   - 普通伤害：白色数字
	 *   - 格挡命中：黄色数字（伤害减半）
	 *   - 暴击：红色大号数字
	 */
	if (!IsValid(Props.SourceCharacter) || !IsValid(Props.TargetCharacter)) return;
	
	// 如果来源和目标相同，不显示伤害数字（避免自己对自己造成伤害时显示）
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		// 优先在来源的 PlayerController 上显示（玩家攻击敌人时）
		if(AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
			return;
		}
		// 否则在目标的 PlayerController 上显示（敌人攻击玩家时）
		if(AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}
