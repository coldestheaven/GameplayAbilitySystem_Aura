// Copyright Druid Mechanics


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"

/**
 * 构造函数：初始化玩家状态的核心组件
 * 
 * 初始化流程：
 * 1. 创建 AbilitySystemComponent（ASC）- 负责管理所有技能和 GameplayEffect
 * 2. 配置 ASC 的网络复制模式为 Mixed（服务端和客户端都可以应用效果）
 * 3. 创建 AttributeSet（属性集）- 存储所有角色属性（生命值、法力值、力量等）
 * 4. 设置网络更新频率为 100Hz（确保属性变化能及时同步）
 * 
 * 为什么将 ASC 放在 PlayerState 上？
 * - PlayerState 在服务端和所有客户端上都存在，且跨关卡（无缝旅行）时不会销毁
 * - 这样可以确保玩家的技能和属性数据在切换地图时不会丢失
 * - 符合 GAS 的最佳实践：将 ASC 放在 PlayerState（玩家）或 Pawn（AI）上
 */
AAuraPlayerState::AAuraPlayerState()
{
	// 创建自定义的 AbilitySystemComponent
	// 使用 CreateDefaultSubobject 确保组件在对象构造时创建，并支持序列化
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	// 启用网络复制（确保客户端的 ASC 能接收服务端的 GameplayEffect 同步）
	AbilitySystemComponent->SetIsReplicated(true);
	// 设置复制模式为 Mixed：
	// - 服务端：可以应用所有 GameplayEffect
	// - 客户端：只能应用预测的 GameplayEffect（如本地输入触发的技能）
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 创建属性集，存储所有角色属性（生命值、法力值、主属性等）
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	// 设置网络更新频率为 100Hz（每秒更新 100 次）
	// 较高的更新频率确保属性变化（如生命值、XP）能及时同步到客户端
	NetUpdateFrequency = 100.f;
}

/**
 * 注册需要网络同步的属性
 * 
 * 实现流程：
 * 1. 调用父类方法，注册基类的复制属性
 * 2. 使用 DOREPLIFETIME 宏注册本类的复制属性
 * 
 * 网络同步说明：
 * - Level、XP、AttributePoints、SpellPoints 都会从服务端同步到所有客户端
 * - 当这些属性在服务端发生变化时，会自动触发对应的 OnRep_ 函数
 * - OnRep_ 函数中会广播委托，通知 UI 更新显示
 * 
 * 为什么这些属性需要网络同步？
 * - 这些是玩家进度数据，需要在所有客户端上保持一致
 * - UI 系统需要监听这些数据的变化来更新显示
 * - 多人游戏中，其他玩家也需要看到你的等级和进度
 */
void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 调用父类方法，注册基类（APlayerState）的复制属性
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 注册玩家等级（从服务端同步到客户端）
	DOREPLIFETIME(AAuraPlayerState, Level);
	// 注册玩家 XP（从服务端同步到客户端）
	DOREPLIFETIME(AAuraPlayerState, XP);
	// 注册可用属性点数量（从服务端同步到客户端）
	DOREPLIFETIME(AAuraPlayerState, AttributePoints);
	// 注册可用技能点数量（从服务端同步到客户端）
	DOREPLIFETIME(AAuraPlayerState, SpellPoints);
}

/**
 * 实现 IAbilitySystemInterface，返回玩家的 AbilitySystemComponent
 * 
 * 实现说明：
 * - 这是 IAbilitySystemInterface 接口的必需实现
 * - 其他系统（如技能、GameplayEffect）通过此接口获取 ASC
 * - 使用接口的好处：代码解耦，不直接依赖具体类
 * 
 * @return 玩家的 AbilitySystemComponent 指针
 * 
 * 使用示例：
 *   if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PlayerState))
 *   {
 *       UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
 *       // 使用 ASC 应用 GameplayEffect 或激活技能
 *   }
 */
UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

/**
 * 增加 XP（累加到当前值）
 * 
 * 实现流程：
 * 1. 将输入的 XP 累加到当前 XP 值
 * 2. 广播 OnXPChangedDelegate 委托，通知所有监听者（如 UI 经验条）
 * 
 * @param InXP 要增加的 XP 数量（必须 >= 0）
 * 
 * 使用场景：
 * - 击杀敌人后获得 XP
 * - 完成任务后获得 XP
 * - 从存档加载时恢复 XP
 * 
 * 注意：此函数应该在服务端调用，XP 会通过网络同步到客户端
 */
void AAuraPlayerState::AddToXP(int32 InXP)
{
	// 累加 XP 值
	XP += InXP;
	// 广播委托，通知 UI 更新经验条显示
	OnXPChangedDelegate.Broadcast(XP);
}

/**
 * 增加等级（累加到当前值）
 * 
 * 实现流程：
 * 1. 将输入的等级数累加到当前等级
 * 2. 广播 OnLevelChangedDelegate 委托，bLevelUp=true 表示这是升级事件
 * 
 * @param InLevel 要增加的等级数（通常为 1）
 * 
 * 使用场景：
 * - 当玩家 XP 达到升级阈值时调用
 * - 从存档加载时恢复等级
 * 
 * 注意：
 * - bLevelUp=true 会触发升级特效（如粒子效果）
 * - 此函数应该在服务端调用，等级会通过网络同步到客户端
 */
void AAuraPlayerState::AddToLevel(int32 InLevel)
{
	// 累加等级
	Level += InLevel;
	// 广播委托，bLevelUp=true 表示这是升级事件（会触发升级特效）
	OnLevelChangedDelegate.Broadcast(Level, true);
}

/**
 * 设置 XP（直接设置值，不累加）
 * 
 * 实现流程：
 * 1. 直接将 XP 设置为指定值（覆盖当前值）
 * 2. 广播 OnXPChangedDelegate 委托，通知 UI 更新
 * 
 * @param InXP 要设置的 XP 值
 * 
 * 使用场景：
 * - 从存档加载时恢复 XP
 * - 重置玩家进度
 * 
 * 注意：此函数应该在服务端调用，XP 会通过网络同步到客户端
 */
void AAuraPlayerState::SetXP(int32 InXP)
{
	// 直接设置 XP 值（覆盖当前值）
	XP = InXP;
	// 广播委托，通知 UI 更新经验条显示
	OnXPChangedDelegate.Broadcast(XP);
}

/**
 * 设置等级（直接设置值，不累加）
 * 
 * 实现流程：
 * 1. 直接将等级设置为指定值（覆盖当前值）
 * 2. 广播 OnLevelChangedDelegate 委托，bLevelUp=false 表示这不是升级事件
 * 
 * @param InLevel 要设置的等级值
 * 
 * 使用场景：
 * - 从存档加载时恢复等级
 * - 重置玩家进度
 * - 测试时设置特定等级
 * 
 * 注意：
 * - bLevelUp=false 不会触发升级特效（因为不是升级事件）
 * - 此函数应该在服务端调用，等级会通过网络同步到客户端
 */
void AAuraPlayerState::SetLevel(int32 InLevel)
{
	// 直接设置等级值（覆盖当前值）
	Level = InLevel;
	// 广播委托，bLevelUp=false 表示这不是升级事件（不会触发升级特效）
	OnLevelChangedDelegate.Broadcast(Level, false);
}

/**
 * 设置属性点（直接设置值，不累加）
 * 
 * 实现流程：
 * 1. 直接将属性点设置为指定值（覆盖当前值）
 * 2. 广播 OnAttributePointsChangedDelegate 委托，通知 UI 更新
 * 
 * @param InPoints 要设置的属性点数量
 * 
 * 使用场景：
 * - 从存档加载时恢复属性点
 * - 重置玩家进度
 * - 升级时奖励属性点（通过 AddToAttributePoints）
 * 
 * 注意：此函数应该在服务端调用，属性点会通过网络同步到客户端
 */
void AAuraPlayerState::SetAttributePoints(int32 InPoints)
{
	// 直接设置属性点数量（覆盖当前值）
	AttributePoints = InPoints;
	// 广播委托，通知属性菜单 UI 更新可用属性点显示
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

/**
 * 设置技能点（直接设置值，不累加）
 * 
 * 实现流程：
 * 1. 直接将技能点设置为指定值（覆盖当前值）
 * 2. 广播 OnSpellPointsChangedDelegate 委托，通知 UI 更新
 * 
 * @param InPoints 要设置的技能点数量
 * 
 * 使用场景：
 * - 从存档加载时恢复技能点
 * - 重置玩家进度
 * - 升级时奖励技能点（通过 AddToSpellPoints）
 * 
 * 注意：此函数应该在服务端调用，技能点会通过网络同步到客户端
 */
void AAuraPlayerState::SetSpellPoints(int32 InPoints)
{
	// 直接设置技能点数量（覆盖当前值）
	SpellPoints = InPoints;
	// 广播委托，通知技能菜单 UI 更新可用技能点显示
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

/**
 * 网络复制回调：当 Level 从服务端同步到客户端时调用
 * 
 * 实现流程：
 * 1. 当服务端的 Level 发生变化并同步到客户端时，引擎会自动调用此函数
 * 2. 广播 OnLevelChangedDelegate 委托，bLevelUp=true 表示这是升级事件
 * 
 * @param OldLevel 同步前的旧等级值（用于比较变化）
 * 
 * 网络同步说明：
 * - 此函数只在客户端调用（服务端不会调用）
 * - 当服务端修改 Level 时，会自动同步到所有客户端
 * - 客户端收到同步后，会调用此函数来更新 UI
 * 
 * 为什么需要 OnRep_ 函数？
 * - 服务端修改属性时，不会自动触发委托广播
 * - 客户端需要知道属性变化，以便更新 UI
 * - OnRep_ 函数确保客户端也能响应属性变化
 */
void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	// 广播委托，bLevelUp=true 表示这是升级事件（会触发升级特效）
	OnLevelChangedDelegate.Broadcast(Level, true);
}

/**
 * 网络复制回调：当 XP 从服务端同步到客户端时调用
 * 
 * 实现流程：
 * 1. 当服务端的 XP 发生变化并同步到客户端时，引擎会自动调用此函数
 * 2. 广播 OnXPChangedDelegate 委托，通知 UI 更新经验条
 * 
 * @param OldXP 同步前的旧 XP 值（用于比较变化）
 * 
 * 网络同步说明：
 * - 此函数只在客户端调用（服务端不会调用）
 * - 当服务端修改 XP 时，会自动同步到所有客户端
 * - 客户端收到同步后，会调用此函数来更新 UI
 */
void AAuraPlayerState::OnRep_XP(int32 OldXP)
{
	// 广播委托，通知 UI 更新经验条显示
	OnXPChangedDelegate.Broadcast(XP);
}

/**
 * 网络复制回调：当 AttributePoints 从服务端同步到客户端时调用
 * 
 * 实现流程：
 * 1. 当服务端的 AttributePoints 发生变化并同步到客户端时，引擎会自动调用此函数
 * 2. 广播 OnAttributePointsChangedDelegate 委托，通知 UI 更新属性点显示
 * 
 * @param OldAttributePoints 同步前的旧属性点值（用于比较变化）
 * 
 * 网络同步说明：
 * - 此函数只在客户端调用（服务端不会调用）
 * - 当服务端修改 AttributePoints 时，会自动同步到所有客户端
 * - 客户端收到同步后，会调用此函数来更新 UI
 */
void AAuraPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	// 广播委托，通知属性菜单 UI 更新可用属性点显示
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

/**
 * 网络复制回调：当 SpellPoints 从服务端同步到客户端时调用
 * 
 * 实现流程：
 * 1. 当服务端的 SpellPoints 发生变化并同步到客户端时，引擎会自动调用此函数
 * 2. 广播 OnSpellPointsChangedDelegate 委托，通知 UI 更新技能点显示
 * 
 * @param OldSpellPoints 同步前的旧技能点值（用于比较变化）
 * 
 * 网络同步说明：
 * - 此函数只在客户端调用（服务端不会调用）
 * - 当服务端修改 SpellPoints 时，会自动同步到所有客户端
 * - 客户端收到同步后，会调用此函数来更新 UI
 */
void AAuraPlayerState::OnRep_SpellPoints(int32 OldSpellPoints)
{
	// 广播委托，通知技能菜单 UI 更新可用技能点显示
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

/**
 * 增加属性点（累加到当前值）
 * 
 * 实现流程：
 * 1. 将输入的属性点数累加到当前属性点值
 * 2. 广播 OnAttributePointsChangedDelegate 委托，通知 UI 更新
 * 
 * @param InPoints 要增加的属性点数量（必须 >= 0）
 * 
 * 使用场景：
 * - 升级时奖励属性点（根据 LevelUpInfo 配置）
 * - 完成任务后奖励属性点
 * - 从存档加载时恢复属性点
 * 
 * 注意：此函数应该在服务端调用，属性点会通过网络同步到客户端
 */
void AAuraPlayerState::AddToAttributePoints(int32 InPoints)
{
	// 累加属性点数量
	AttributePoints += InPoints;
	// 广播委托，通知属性菜单 UI 更新可用属性点显示
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

/**
 * 增加技能点（累加到当前值）
 * 
 * 实现流程：
 * 1. 将输入的技能点数累加到当前技能点值
 * 2. 广播 OnSpellPointsChangedDelegate 委托，通知 UI 更新
 * 
 * @param InPoints 要增加的技能点数量（必须 >= 0）
 * 
 * 使用场景：
 * - 升级时奖励技能点（根据 LevelUpInfo 配置）
 * - 完成任务后奖励技能点
 * - 从存档加载时恢复技能点
 * 
 * 注意：此函数应该在服务端调用，技能点会通过网络同步到客户端
 */
void AAuraPlayerState::AddToSpellPoints(int32 InPoints)
{
	// 累加技能点数量
	SpellPoints += InPoints;
	// 广播委托，通知技能菜单 UI 更新可用技能点显示
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}