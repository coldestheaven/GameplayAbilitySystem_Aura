// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttributeInfo.generated.h"

/**
 * 单个属性的 UI 信息结构体
 * 存储属性在菜单中显示所需的所有 UI 数据
 */
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
	GENERATED_BODY()

	/** 属性唯一标识标签（如 Attributes.Primary.Strength），用于查找和匹配 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	/** 属性显示名称（如"力量"、"智力"），在属性菜单中显示 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();

	/** 属性描述文本（解释属性的作用，鼠标悬停时显示） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();

	/**
	 * 属性当前值（运行时填充）
	 * 由 AttributeMenuWidgetController 在广播时填入当前属性值
	 * 不在数据资产中配置，每次广播时动态更新
	 */
	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.f;
};

/**
 * 属性信息数据资产
 *
 * 存储游戏中所有属性的 UI 显示信息（名称、描述）
 * 在 AttributeMenuWidgetController 的 Details 面板中指定
 *
 * 使用方式：
 *   // 根据属性标签查找属性信息
 *   FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
 *   // 填入当前值后广播给 Widget
 *   Info.AttributeValue = AttributeSet->GetStrength();
 *   AttributeInfoDelegate.Broadcast(Info);
 */
UCLASS()
class AURA_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 根据属性标签查找对应的属性信息
	 * @param AttributeTag 要查找的属性标签
	 * @param bLogNotFound 未找到时是否输出警告日志
	 * @return 对应的 FAuraAttributeInfo 结构体（未找到则返回空结构体）
	 */
	FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	/** 所有属性的信息数组（在 Details 面板中配置每个属性的名称和描述） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAttributeInfo> AttributeInformation;
};
