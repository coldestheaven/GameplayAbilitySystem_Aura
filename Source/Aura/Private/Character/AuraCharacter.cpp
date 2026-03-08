// Copyright Druid Mechanics


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/AuraHUD.h"

/**
 * 构造函数：初始化玩家角色的摄像机、移动和升级特效
 * 
 * 实现流程：
 * 1. 创建弹簧臂组件（CameraBoom），挂载到根组件，使用绝对旋转，禁用碰撞测试
 * 2. 创建俯视摄像机组件，挂载到弹簧臂末端，不随角色旋转
 * 3. 创建升级粒子 Niagara 组件，挂载到根组件，默认不自动激活
 * 4. 配置角色移动：朝向移动方向、400°/s 旋转、约束在平面内、启动时吸附到平面
 * 5. 禁用控制器旋转对角色姿态的影响（俯视角不需要）
 * 6. 设置默认职业为 Elementalist（元素师）
 * 
 * 使用场景：
 * - 玩家角色在游戏开始时由引擎自动构造
 * - 摄像机配置适用于俯视角 ARPG 玩法
 */
AAuraCharacter::AAuraCharacter()
{
	// 创建弹簧臂：控制摄像机与角色的距离和角度
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;

	// 创建俯视摄像机，挂载在弹簧臂末端
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	// 创建升级特效组件（升级时手动激活）
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	// 配置角色移动：朝向移动方向、平面内移动
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// 俯视角下角色不随控制器俯仰/翻滚/偏航旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::Elementalist;
}

/**
 * 服务端：角色被控制器接管时调用
 * 
 * 实现流程：
 * 1. 调用父类 PossessedBy，执行基类逻辑
 * 2. 初始化 ASC ActorInfo（服务端侧）
 * 3. 从存档加载玩家进度（属性、技能、等级等）
 * 4. 加载关卡世界状态（实现 SaveInterface 的 Actor 状态恢复）
 * 
 * @param NewController 接管此角色的控制器（通常为 AuraPlayerController）
 * 
 * 网络同步说明：
 * - 此函数仅在服务端调用，客户端不会执行
 * - ASC 和 AttributeSet 在 PlayerState 上，PlayerState 会复制到客户端
 * - 客户端通过 OnRep_PlayerState 完成 ASC 初始化
 * 
 * 注意：
 * - 必须在 PossessedBy 中完成 ASC 初始化，否则技能无法在服务端工作
 * - LoadWorldState 会恢复关卡中实现 SaveInterface 的 Actor 状态
 */
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 服务端：初始化 ASC ActorInfo
	InitAbilityActorInfo();
	LoadProgress();

	// 加载关卡世界状态（恢复保存的 Actor 状态）
	if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		AuraGameMode->LoadWorldState(GetWorld());
	}
}

/**
 * 从存档加载玩家进度（属性、技能、等级、XP 等）
 * 
 * 实现流程：
 * 1. 获取 GameMode 并调用 RetrieveInGameSaveData 获取存档数据
 * 2. 如果存档无效，直接返回
 * 3. 如果是首次加载（bFirstTimeLoadIn=true）：
 *    - 初始化默认属性（主属性、次属性、生命/法力）
 *    - 添加角色初始技能（主动 + 被动）
 * 4. 否则从存档恢复：
 *    - 通过 ASC 从存档恢复技能状态（AddCharacterAbilitiesFromSaveData）
 *    - 恢复 PlayerState 的等级、XP、属性点、技能点
 *    - 使用 InitializeDefaultAttributesFromSaveData 恢复主属性值
 * 
 * 使用场景：
 * - 在 PossessedBy 中调用，服务端接管角色后加载进度
 * - 新游戏：初始化默认属性和技能
 * - 读档：恢复上次保存的进度
 * 
 * 注意：
 * - 此函数在服务端调用，存档数据从 GameInstance 的 LoadSlot 获取
 * - 技能恢复依赖 ASC 的 AddCharacterAbilitiesFromSaveData
 */
void AAuraCharacter::LoadProgress()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (AuraGameMode)
	{
		ULoadScreenSaveGame* SaveData = AuraGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		if (SaveData->bFirstTimeLoadIn)
		{
			// 首次加载：初始化默认属性和技能
			InitializeDefaultAttributes();
			AddCharacterAbilities();
		}
		else
		{
			// 从存档恢复技能状态
			if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
			{
				AuraASC->AddCharacterAbilitiesFromSaveData(SaveData);
			}
			
			// 从存档恢复等级、XP、属性点、技能点
			if (AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(GetPlayerState()))
			{
				AuraPlayerState->SetLevel(SaveData->PlayerLevel);
				AuraPlayerState->SetXP(SaveData->XP);
				AuraPlayerState->SetAttributePoints(SaveData->AttributePoints);
				AuraPlayerState->SetSpellPoints(SaveData->SpellPoints);
			}
			
			// 从存档恢复主属性值（力量、智力、韧性、活力）
			UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
		}
	}
}

/**
 * 客户端：PlayerState 从服务端复制完成后调用
 * 
 * 实现流程：
 * 1. 调用父类 OnRep_PlayerState
 * 2. 初始化 ASC ActorInfo（客户端侧）
 * 
 * 网络同步说明：
 * - 此函数仅在客户端调用（服务端已有 PlayerState，不会触发 OnRep）
 * - 当 PlayerState 复制到客户端后，引擎自动调用此函数
 * - 客户端必须在此完成 ASC 初始化，否则技能、属性、UI 无法正常工作
 * 
 * 注意：
 * - 客户端不调用 LoadProgress，进度数据由 PlayerState 的复制属性同步
 * - 客户端不调用 LoadWorldState，世界状态由服务端权威
 */
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 客户端：初始化 ASC ActorInfo
	InitAbilityActorInfo();
}

/**
 * 增加经验值（IPlayerInterface 实现）
 * 
 * 实现流程：
 * 1. 获取 PlayerState 并校验非空
 * 2. 调用 PlayerState::AddToXP 累加 XP
 * 3. PlayerState 内部会广播 OnXPChangedDelegate，触发升级检测
 * 
 * @param InXP 要增加的 XP 数量（必须 >= 0）
 * 
 * 使用场景：
 * - 击杀敌人后获得 XP
 * - 完成任务后获得 XP
 * - 由 PlayerState 的升级逻辑间接调用
 * 
 * 网络同步说明：
 * - 此函数为 RPC（_Implementation 表示服务端执行体）
 * - 应在服务端调用，XP 通过 PlayerState 的 DOREPLIFETIME 同步到客户端
 */
void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToXP(InXP);
}

/**
 * 触发升级逻辑（IPlayerInterface 实现）
 * 
 * 实现流程：
 * 1. 调用 MulticastLevelUpParticles 多播 RPC，在所有客户端播放升级粒子特效
 * 
 * 使用场景：
 * - 由 PlayerState 的 OnRep_Level 或升级逻辑调用
 * - 当玩家 XP 达到升级阈值时触发
 * 
 * 网络同步说明：
 * - 此函数为 RPC，在服务端执行
 * - MulticastLevelUpParticles 会复制到所有客户端，确保所有人看到升级特效
 * - 属性点和技能点的增加由 PlayerState 的 AddToLevel 等函数处理
 */
void AAuraCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

/**
 * 多播 RPC：在所有客户端播放升级粒子特效
 * 
 * 实现流程：
 * 1. 校验 LevelUpNiagaraComponent 有效
 * 2. 计算特效朝向：从特效位置指向摄像机，使特效面向玩家视角
 * 3. 设置组件旋转并激活粒子系统
 * 
 * 网络同步说明：
 * - NetMulticast + Reliable：服务端调用后复制到所有客户端执行
 * - 确保所有玩家都能看到升级特效
 * 
 * 注意：
 * - 特效朝向摄像机可提升视觉效果
 * - 仅在客户端执行视觉逻辑，不修改权威数据
 */
void AAuraCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetXP();
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToLevel(InPlayerLevel);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
	}
}

void AAuraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToAttributePoints(InAttributePoints);
}

void AAuraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetSpellPoints();
}

void AAuraCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		AuraPlayerController->ShowMagicCircle(DecalMaterial);
		AuraPlayerController->bShowMouseCursor = false;
	}
}

void AAuraCharacter::HideMagicCircle_Implementation()
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		AuraPlayerController->HideMagicCircle();
		AuraPlayerController->bShowMouseCursor = true;
	}
}

/**
 * 保存游戏进度（IPlayerInterface 实现）
 * 
 * 实现流程：
 * 1. 获取 GameMode 和当前存档数据
 * 2. 设置 PlayerStartTag（检查点标签，用于重生位置）
 * 3. 从 PlayerState 保存等级、XP、属性点、技能点
 * 4. 从 AttributeSet 保存主属性值（力量、智力、韧性、活力）
 * 5. 设置 bFirstTimeLoadIn = false
 * 6. 仅服务端：遍历 ASC 技能，将技能状态保存到 SavedAbilities
 * 7. 调用 GameMode::SaveInGameProgressData 写入磁盘
 * 
 * @param CheckpointTag 触发保存的检查点标签（对应 PlayerStart 的 PlayerStartTag）
 * 
 * 使用场景：
 * - 玩家到达检查点时自动保存
 * - 手动保存
 * 
 * 网络同步说明：
 * - 只有服务端会实际写入存档（HasAuthority 检查）
 * - 客户端调用此函数不会保存技能数据，但会填充 SaveData 的基础字段
 * 
 * 注意：
 * - 技能数据（SavedAbilities）仅在服务端保存
 * - 关卡世界状态需通过 GameMode::SaveWorldState 单独保存
 */
void AAuraCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (AuraGameMode)
	{
		ULoadScreenSaveGame* SaveData = AuraGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		SaveData->PlayerStartTag = CheckpointTag;

		if (AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(GetPlayerState()))
		{
			SaveData->PlayerLevel = AuraPlayerState->GetPlayerLevel();
			SaveData->XP = AuraPlayerState->GetXP();
			SaveData->AttributePoints = AuraPlayerState->GetAttributePoints();
			SaveData->SpellPoints = AuraPlayerState->GetSpellPoints();
		}
		SaveData->Strength = UAuraAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Intelligence = UAuraAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Resilience = UAuraAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Vigor = UAuraAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

		SaveData->bFirstTimeLoadIn = false;

		if (!HasAuthority()) return;

		UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
		FForEachAbility SaveAbilityDelegate;
		SaveData->SavedAbilities.Empty();
		SaveAbilityDelegate.BindLambda([this, AuraASC, SaveData](const FGameplayAbilitySpec& AbilitySpec)
		{
			const FGameplayTag AbilityTag = AuraASC->GetAbilityTagFromSpec(AbilitySpec);
			UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(this);
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

			FSavedAbility SavedAbility;
			SavedAbility.GameplayAbility = Info.Ability;
			SavedAbility.AbilityLevel = AbilitySpec.Level;
			SavedAbility.AbilitySlot = AuraASC->GetSlotFromAbilityTag(AbilityTag);
			SavedAbility.AbilityStatus = AuraASC->GetStatusFromAbilityTag(AbilityTag);
			SavedAbility.AbilityTag = AbilityTag;
			SavedAbility.AbilityType = Info.AbilityType;

			SaveData->SavedAbilities.AddUnique(SavedAbility);

		});
		AuraASC->ForEachAbility(SaveAbilityDelegate);
		
		AuraGameMode->SaveInGameProgressData(SaveData);
	}
}

int32 AAuraCharacter::GetPlayerLevel_Implementation()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

/**
 * 玩家死亡处理（重写基类）
 * 
 * 实现流程：
 * 1. 调用父类 Die，执行基类死亡逻辑
 * 2. 创建定时器，DeathTime 秒后调用 GameMode::PlayerDied
 * 3. 将摄像机从角色分离，保持世界位置（便于观察死亡动画）
 * 
 * @param DeathImpulse 死亡冲量（用于布娃娃物理击飞）
 * 
 * 使用场景：
 * - 玩家生命值归零时由 GAS 或伤害系统调用
 * 
 * 注意：
 * - PlayerDied 会重新加载关卡，玩家在存档记录的地图出生点重生
 * - 摄像机分离后保持世界位置，不会跟随角色下落
 */
void AAuraCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindLambda([this]()
	{
		AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (AuraGM)
		{
			AuraGM->PlayerDied(this);
		}
	});
	GetWorldTimerManager().SetTimer(DeathTimer, DeathTimerDelegate, DeathTime, false);
	TopDownCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

/**
 * 眩晕状态同步回调（重写基类）
 * 
 * 实现流程：
 * 1. 获取 ASC 和 GameplayTags
 * 2. 构建输入阻止标签容器（光标追踪、输入按下/按住/释放）
 * 3. 若 bIsStunned 为 true：添加阻止标签、激活眩晕特效
 * 4. 若 bIsStunned 为 false：移除阻止标签、停用眩晕特效
 * 
 * 网络同步说明：
 * - 此函数在客户端调用（bIsStunned 通过 ReplicatedUsing 同步）
 * - 服务端通过 StunTagChanged 设置 bIsStunned，变化后复制到客户端
 * 
 * 注意：
 * - 眩晕期间玩家无法移动、释放技能（通过阻止标签实现）
 */
void AAuraCharacter::OnRep_Stunned()
{
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		if (bIsStunned)
		{
			AuraASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Activate();
		}
		else
		{
			AuraASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Deactivate();
		}
	}
}

/**
 * 燃烧状态同步回调（重写基类）
 * 
 * 实现流程：
 * 1. 若 bIsBurned 为 true：激活燃烧特效组件
 * 2. 否则：停用燃烧特效组件
 * 
 * 网络同步说明：
 * - 此函数在客户端调用（bIsBurned 通过 ReplicatedUsing 同步）
 * - 服务端由燃烧 GameplayEffect 设置 bIsBurned
 */
void AAuraCharacter::OnRep_Burned()
{
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}
	else
	{
		BurnDebuffComponent->Deactivate();
	}
}

/**
 * 初始化 ASC ActorInfo（重写基类）
 * 
 * 实现流程：
 * 1. 从 PlayerState 获取 ASC 和 AttributeSet
 * 2. 调用 ASC::InitAbilityActorInfo（Owner=PlayerState, Avatar=this）
 * 3. 调用 AuraASC::AbilityActorInfoSet（自定义初始化）
 * 4. 将 ASC 和 AttributeSet 缓存到本类成员
 * 5. 广播 OnAscRegistered 委托
 * 6. 注册眩晕标签变化回调（StunTagChanged）
 * 7. 若有 PlayerController 和 HUD，初始化 Overlay（绑定 WidgetController）
 * 
 * 使用场景：
 * - 服务端：PossessedBy 中调用
 * - 客户端：OnRep_PlayerState 中调用
 * 
 * 注意：
 * - 玩家角色的 ASC 在 PlayerState 上，需从 PlayerState 获取
 * - HUD 初始化依赖 Controller 和 PlayerState 已就绪
 */
void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	OnAscRegistered.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraCharacter::StunTagChanged);

	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
