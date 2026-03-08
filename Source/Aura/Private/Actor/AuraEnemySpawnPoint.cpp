// Copyright Druid Mechanics


#include "Actor/AuraEnemySpawnPoint.h"

#include "Character/AuraEnemy.h"

/**
 * 生成敌人
 * 
 * 实现流程：
 * 1. 配置生成参数（碰撞处理：尽可能调整但始终生成）
 * 2. 延迟生成敌人（SpawnActorDeferred）
 * 3. 设置敌人等级（SetLevel）
 * 4. 设置敌人职业（SetCharacterClass）
 * 5. 完成生成（FinishSpawning）
 * 6. 生成默认 AI 控制器（SpawnDefaultController）
 * 
 * 使用场景：
 * - 玩家进入敌人生成区域时调用
 * - 由 AuraEnemySpawnVolume 调用
 * 
 * 注意：
 * - 使用延迟生成以便在生成前设置属性
 * - 生成后会自动创建 AI 控制器
 */
void AAuraEnemySpawnPoint::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 延迟生成敌人
	AAuraEnemy* Enemy = GetWorld()->SpawnActorDeferred<AAuraEnemy>(EnemyClass, GetActorTransform());
	
	// 设置敌人属性
	Enemy->SetLevel(EnemyLevel);
	Enemy->SetCharacterClass(CharacterClass);
	
	// 完成生成
	Enemy->FinishSpawning(GetActorTransform());
	
	// 生成 AI 控制器
	Enemy->SpawnDefaultController();
}
