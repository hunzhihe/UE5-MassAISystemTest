// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HashGridSubsystem.h"

#include "HashGridFragments.generated.h"

/**
 * 
 */
/**
 * @brief 命名空间HashGridExample::Signals定义了哈希网格示例中使用的信号名称
 * 
 * 该命名空间包含了用于实体查询事件的信号标识符，供系统中的事件通信机制使用
 */
namespace HashGridExample::Signals
{
	/**
	 * @brief 实体查询事件的信号名称
	 * 
	 * 当实体被查询时触发的事件标识符，用于在系统组件间传递实体查询消息
	 */
	const FName EntityQueried = FName(TEXT("EntityQueried"));
}

/**
 * @brief 哈希网格片段结构体
 * 
 * 继承自FMassFragment的结构体，用于存储实体在哈希网格中的位置信息。
 * 该结构体作为ECS架构中的片段组件，保存实体所在的网格单元位置
 */
USTRUCT()
struct SPATIALHASHGRIDEXAMPLE_API FHashGridFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	/** 实体在哈希网格中的单元格位置信息 */
	FHashGridExample::FCellLocation CellLocation;
};
