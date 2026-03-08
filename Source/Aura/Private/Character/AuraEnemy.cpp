// Copyright Druid Mechanics


#include "Character/AuraEnemy.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Aura/Aura.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/EnemyHealthBarWidgetController.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AI/AuraAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

/**
 * 构造函数：初始化敌人角色的组件和配置
 * 
 * 实现流程：
 * 1. 设置网格体碰撞响应（阻挡可见性通道，用于光标追踪）
 * 2. 创建 ASC，启用网络复制，使用 Minimal 模式（仅复制到拥有者）
 * 3. 禁用控制器旋转对角色姿态的影响（AI 控制）
 * 4. 启用控制器期望旋转（AI 会朝向目标）
 * 5. 创建 AttributeSet
 * 6. 创建血条 Widget 组件，挂载到根组件
 * 7. 设置自定义深度模板值（CUSTOM_DEPTH_RED，用于高亮显示）
 * 8. 设置基础移动速度
 * 
 * 使用场景：
 * - 敌人角色在游戏开始时由引擎自动构造
 * 
 * 注意：
 * - ASC 使用 Minimal 复制模式（敌人不需要完整复制到所有客户端）
 * - 自定义深度用于高亮显示（鼠标悬停时）
 */
AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 创建 ASC，使用 Minimal 复制模式（仅复制到拥有者）
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// AI 控制：禁用控制器旋转对角色姿态的影响，启用控制器期望旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	// 创建血条 Widget 组件
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	// 设置自定义深度模板值（用于高亮显示）
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty();
	
	BaseWalkSpeed = 250.f;
}

/**
 * 服务端：敌人被 AI 控制器接管时调用
 * 
 * 实现流程：
 * 1. 调用父类 PossessedBy
 * 2. 仅服务端：初始化 AI 行为树
 *    - 将 NewController 转换为 AuraAIController
 *    - 初始化黑板（使用 BehaviorTree 的黑板资产）
 *    - 运行行为树
 *    - 设置初始黑板值：
 *      * HitReacting = false（未受击）
 *      * RangedAttacker = true/false（根据职业判断是否为远程攻击者）
 * 
 * @param NewController AI 控制器（AuraAIController）
 * 
 * 网络同步说明：
 * - 此函数仅在服务端调用
 * - AI 行为树只在服务端运行，客户端不需要
 * 
 * 注意：
 * - RangedAttacker 用于 AI 行为树判断攻击方式（近战/远程）
 * - Warrior 职业为近战，其他职业为远程
 */
void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	AuraAIController = Cast<AAuraAIController>(NewController);
	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AuraAIController->RunBehaviorTree(BehaviorTree);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

void AAuraEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	// Do not change OutDestination
}

int32 AAuraEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AAuraEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	if (AuraAIController) AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	SpawnLoot();
	Super::Die(DeathImpulse);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

/**
 * 游戏开始时初始化敌人
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 设置移动速度为 BaseWalkSpeed
 * 3. 初始化 ASC ActorInfo
 * 4. 仅服务端：赋予初始技能（通用技能 + 职业专属技能）
 * 5. 初始化血条 Widget
 * 6. 绑定属性变化委托（生命值、最大生命值）
 * 7. 注册受击标签变化回调（HitReactTagChanged）
 * 8. 广播初始值（确保血条显示正确）
 * 
 * 使用场景：
 * - 敌人角色生成后自动调用
 * 
 * 注意：
 * - 技能赋予仅在服务端执行
 * - 血条 Widget 会在敌人头顶显示
 * - 受击标签变化会影响移动速度（受击时停止移动）
 */
void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);	
	}

	// 初始化头顶 HUD Widget
	InitializeHealthBarWidget();
	
	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		// 绑定属性变化委托（用于旧的委托系统兼容）
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AAuraEnemy::HitReactTagChanged
		);

		// 广播初始值
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
}

/**
 * 受击标签变化回调
 * 
 * 实现流程：
 * 1. 根据 NewCount 设置 bHitReacting（>0 为受击中）
 * 2. 受击时 MaxWalkSpeed=0，否则恢复为 BaseWalkSpeed
 * 3. 在黑板中设置 HitReacting 值（通知 AI 行为树）
 * 
 * @param CallbackTag 触发回调的标签（Effects_HitReact）
 * @param NewCount 当前标签数量
 * 
 * 使用场景：
 * - 敌人受到伤害并触发受击动画时调用
 * - 由 ASC 的 RegisterGameplayTagEvent 在受击 GE 应用/移除时触发
 * 
 * 注意：
 * - 受击时敌人会停止移动，播放受击动画
 * - AI 行为树会根据 HitReacting 状态调整行为
 */
void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

/**
 * 初始化 ASC ActorInfo（重写基类）
 * 
 * 实现流程：
 * 1. 调用 ASC::InitAbilityActorInfo（Owner=this, Avatar=this）
 * 2. 调用 AuraASC::AbilityActorInfoSet（自定义初始化）
 * 3. 注册眩晕标签变化回调（StunTagChanged）
 * 4. 仅服务端：初始化默认属性
 * 5. 广播 OnAscRegistered 委托
 * 
 * 使用场景：
 * - 在 BeginPlay 中调用
 * 
 * 注意：
 * - 敌人角色的 ASC 在自身 Pawn 上（与玩家不同，玩家在 PlayerState 上）
 * - 属性初始化仅在服务端执行
 */
void AAuraEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraEnemy::StunTagChanged);


	if (HasAuthority())
	{
		InitializeDefaultAttributes();		
	}
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

/**
 * 初始化默认属性（重写基类）
 * 
 * 实现流程：
 * 1. 调用 AuraAbilitySystemLibrary::InitializeDefaultAttributes
 * 2. 传入职业类型和等级，初始化属性
 * 
 * 使用场景：
 * - 在 InitAbilityActorInfo 中调用（仅服务端）
 * 
 * 注意：
 * - 敌人属性根据职业和等级初始化（在 CharacterClassInfo 中配置）
 */
void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

/**
 * 眩晕标签变化回调（重写基类）
 * 
 * 实现流程：
 * 1. 调用父类 StunTagChanged（处理移动速度）
 * 2. 在黑板中设置 Stunned 值（通知 AI 行为树）
 * 
 * @param CallbackTag 触发回调的标签（Debuff_Stun）
 * @param NewCount 当前标签数量
 * 
 * 使用场景：
 * - 敌人被眩晕时调用
 * - AI 行为树会根据 Stunned 状态调整行为
 */
void AAuraEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);
	
	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

/**
 * 初始化血条 Widget（敌人头顶血条）
 * 
 * 实现流程：
 * 1. 校验 HealthBar Widget 组件有效
 * 2. 获取 Widget 对象并转换为 AuraUserWidget
 * 3. 创建 EnemyHealthBarWidgetController
 * 4. 设置 WidgetControllerParams（ASC、AttributeSet，无 PC/PS）
 * 5. 设置敌人引用到 Controller
 * 6. 将 Controller 设置到 Widget
 * 7. 绑定回调并广播初始值
 * 
 * 使用场景：
 * - 在 BeginPlay 中调用
 * 
 * 注意：
 * - 敌人血条使用独立的 WidgetController（EnemyHealthBarWidgetController）
 * - WidgetControllerParams 中 PC 和 PS 为 nullptr（敌人不需要）
 * - 血条会显示在敌人头顶，跟随敌人移动
 */
void AAuraEnemy::InitializeHealthBarWidget()
{
	if (!IsValid(HealthBar)) return;

	// 获取 Widget 对象
	UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject());
	if (!IsValid(AuraUserWidget)) return;

	// 创建 Widget Controller
	if (HealthBarWidgetControllerClass)
	{
		HealthBarWidgetController = NewObject<UEnemyHealthBarWidgetController>(this, HealthBarWidgetControllerClass);
	}
	else
	{
		HealthBarWidgetController = NewObject<UEnemyHealthBarWidgetController>(this);
	}

	// 设置 Widget Controller 参数（敌人不需要 PC 和 PS）
	FWidgetControllerParams WCParams;
	WCParams.AttributeSet = AttributeSet;
	WCParams.AbilitySystemComponent = AbilitySystemComponent;
	WCParams.PlayerState = nullptr;
	WCParams.PlayerController = nullptr;

	HealthBarWidgetController->SetWidgetControllerParams(WCParams);
	HealthBarWidgetController->SetEnemy(this);

	// 将 Controller 设置到 Widget
	AuraUserWidget->SetWidgetController(HealthBarWidgetController);

	// 绑定回调并广播初始值
	HealthBarWidgetController->BindCallbacksToDependencies();
	HealthBarWidgetController->BroadcastInitialValues();
}
