// Copyright Druid Mechanics


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"

/**
 * ASC ActorInfo 设置完成后的初始化
 * 
 * 实现流程：
 * 1. 绑定 GameplayEffect 应用到自身的委托
 * 2. 当 GE 应用到自身时，调用 ClientEffectApplied（客户端 RPC）
 * 
 * 使用场景：
 * - 在 InitAbilityActorInfo 中调用，完成 ASC 初始化
 * 
 * 注意：
 * - ClientEffectApplied 会广播 EffectAssetTags，供 UI 系统监听
 */
// ─────────────────────────────────────────────────────────────────────────────
// 内部辅助：从标签容器中查找第一个匹配指定父标签的子标签
// 消除 GetAbilityTagFromSpec / GetInputTagFromSpec / GetStatusFromSpec 的重复遍历逻辑
// ─────────────────────────────────────────────────────────────────────────────
static FGameplayTag FindFirstTagMatchingParent(const FGameplayTagContainer& Tags, const FName& ParentTagName)
{
	const FGameplayTag ParentTag = FGameplayTag::RequestGameplayTag(ParentTagName);
	for (const FGameplayTag& Tag : Tags)
	{
		if (Tag.MatchesTag(ParentTag))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
	
}

/**
 * 从存档数据恢复角色技能
 * 
 * 实现流程：
 * 1. 遍历存档中的所有技能数据
 * 2. 为每个技能创建 AbilitySpec，设置等级和标签（槽位、状态）
 * 3. 根据技能类型处理：
 *    - 主动技能：直接赋予（GiveAbility）
 *    - 被动技能：
 *      * 如果已装备：赋予并立即激活一次，多播激活被动特效
 *      * 如果未装备：仅赋予（不激活）
 * 4. 设置 bStartupAbilitiesGiven = true
 * 5. 广播 AbilitiesGivenDelegate
 * 
 * @param SaveData 存档数据对象（包含所有已保存的技能状态）
 * 
 * 使用场景：
 * - 从存档加载时恢复技能状态
 * - 在 AuraCharacter::LoadProgress 中调用
 * 
 * 注意：
 * - 被动技能如果已装备，需要立即激活一次（GiveAbilityAndActivateOnce）
 * - 技能状态（Equipped/Unlocked/Eligible）从存档恢复
 */
void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);
		if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Offensive)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		else if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Passive)
		{
			if (Data.AbilityStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);
				MulticastActivatePassiveEffect(Data.AbilityTag, true);
			}
			else
			{
				GiveAbility(LoadedAbilitySpec);
			}
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

/**
 * 添加角色初始主动技能
 * 
 * 实现流程：
 * 1. 遍历所有初始技能类
 * 2. 为每个技能创建 AbilitySpec（等级为 1）
 * 3. 从技能类获取 StartupInputTag（初始输入标签）
 * 4. 设置技能状态为 Equipped（已装备）
 * 5. 赋予技能（GiveAbility）
 * 6. 设置 bStartupAbilitiesGiven = true
 * 7. 广播 AbilitiesGivenDelegate
 * 
 * @param StartupAbilities 初始技能类数组（在 CharacterClassInfo 中配置）
 * 
 * 使用场景：
 * - 首次加载时初始化角色技能
 * - 在 AuraCharacterBase::AddCharacterAbilities 中调用
 * 
 * 注意：
 * - 初始技能默认等级为 1，状态为 Equipped
 * - 只有主动技能会调用此函数，被动技能由 AddCharacterPassiveAbilities 处理
 */
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

/**
 * 添加角色初始被动技能
 * 
 * 实现流程：
 * 1. 遍历所有初始被动技能类
 * 2. 为每个技能创建 AbilitySpec（等级为 1）
 * 3. 设置技能状态为 Equipped（已装备）
 * 4. 赋予技能并立即激活一次（GiveAbilityAndActivateOnce）
 * 
 * @param StartupPassiveAbilities 初始被动技能类数组（在 CharacterClassInfo 中配置）
 * 
 * 使用场景：
 * - 首次加载时初始化角色被动技能
 * - 在 AuraCharacterBase::AddCharacterAbilities 中调用
 * 
 * 注意：
 * - 被动技能需要立即激活一次，才能生效
 * - GiveAbilityAndActivateOnce 会激活技能一次，然后技能会持续运行
 */
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

/**
 * 技能输入标签按下事件处理
 * 
 * 实现流程：
 * 1. 校验输入标签有效
 * 2. 锁定技能列表（防止并发修改）
 * 3. 遍历所有可激活技能
 * 4. 如果技能有匹配的输入标签：
 *    - 调用 AbilitySpecInputPressed（通知技能输入按下）
 *    - 如果技能已激活，调用 InvokeReplicatedEvent（网络同步输入事件）
 * 
 * @param InputTag 输入标签（如 InputTag_LMB、InputTag_RMB 等）
 * 
 * 使用场景：
 * - 玩家按下技能快捷键时调用
 * - 由 AuraPlayerController::AbilityInputTagPressed 调用
 * 
 * 注意：
 * - 使用 FScopedAbilityListLock 确保线程安全
 * - 只有已激活的技能才会同步输入事件到网络
 */
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

/**
 * 技能输入标签按住事件处理
 * 
 * 实现流程：
 * 1. 校验输入标签有效
 * 2. 锁定技能列表
 * 3. 遍历所有可激活技能
 * 4. 如果技能有匹配的输入标签：
 *    - 调用 AbilitySpecInputPressed（通知技能输入按下）
 *    - 如果技能未激活，尝试激活技能（TryActivateAbility）
 * 
 * @param InputTag 输入标签
 * 
 * 使用场景：
 * - 玩家按住技能快捷键时调用
 * - 由 AuraPlayerController::AbilityInputTagHeld 调用
 * 
 * 注意：
 * - 按住时会尝试激活技能（如果未激活）
 * - 已激活的技能会收到输入按下事件，但不会重复激活
 */
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

/**
 * 技能输入标签释放事件处理
 * 
 * 实现流程：
 * 1. 校验输入标签有效
 * 2. 锁定技能列表
 * 3. 遍历所有可激活技能
 * 4. 如果技能有匹配的输入标签且已激活：
 *    - 调用 AbilitySpecInputReleased（通知技能输入释放）
 *    - 调用 InvokeReplicatedEvent（网络同步输入事件）
 * 
 * @param InputTag 输入标签
 * 
 * 使用场景：
 * - 玩家释放技能快捷键时调用
 * - 由 AuraPlayerController::AbilityInputTagReleased 调用
 * 
 * 注意：
 * - 只有已激活的技能才会收到输入释放事件
 * - 输入释放事件会同步到网络，确保多人游戏中一致
 */
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		return FindFirstTagMatchingParent(AbilitySpec.Ability.Get()->AbilityTags, FName("Abilities"));
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	return FindFirstTagMatchingParent(AbilitySpec.DynamicAbilityTags, FName("InputTag"));
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	return FindFirstTagMatchingParent(AbilitySpec.DynamicAbilityTags, FName("Abilities.Status"));
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.DynamicAbilityTags.AddTag(Slot);
}

void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability.Get()->AbilityTags.HasTag(AbilityTag))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag,FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{

			// Handle activation/deactivation for passive abilities

			if (!SlotIsEmpty(Slot)) // There is an ability in this slot already. Deactivate and clear its slot.
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// is that ability the same as this ability? If so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearSlot(SpecWithSlot);
				}
			}

			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't yet have a slot (it's not active)
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if(UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	return Spec && AbilityHasSlot(*Spec, Slot);
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                     const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
