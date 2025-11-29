// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HierarchicalHashGrid2D.h"

#include "MassEntityTypes.h"

#include "MassSubsystemBase.h"

#include "Subsystems/WorldSubsystem.h"
#include "HashGridSubsystem.generated.h"


/**
 * @brief 定义一个二维层次化哈希网格类型的别名
 * 
 * 该类型定义了一个专门用于处理FMassEntityHandle实体的二维层次化哈希网格结构，
 * 其中模板参数指定了网格的层级结构配置。
 * 
 * 模板参数说明：
 * - 第一个参数(3)：表示网格的第一个维度配置
 * - 第二个参数(4)：表示网格的第二个维度配置
 * - FMassEntityHandle：表示存储在网格中的实体句柄类型
 */
typedef THierarchicalHashGrid2D<3, 4, FMassEntityHandle> FHashGridExample;

/**
 * 哈希网格子系统类
 * 用于管理和查询基于哈希网格的空间数据结构
 * 继承自UMassSubsystemBase基类
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()


public:
	/** 哈希网格数据实例，用于存储和管理实体的空间分布信息 */
	FHashGridExample HashGridData;

	/**
	 * 在指定区域内选择实体
	 * 根据给定的位置和半径范围，查找并选择该区域内的所有实体
	 * 
	 * @param SelectedLocation 选择区域的中心位置坐标
	 * @param Radius 选择区域的半径大小
	 */
	UFUNCTION(BlueprintCallable)
	void SelectEntitiesInArea(const FVector& SelectedLocation, float Radius);
	
};

/**
 * UHashGridSubsystem的外部子系统特性模板特化
 * 定义该子系统是否仅在游戏线程中运行
 */
template<>
struct TMassExternalSubsystemTraits<UHashGridSubsystem> final
{
	/** 子系统线程特性枚举 */
	enum
	{
		/** 是否仅在游戏线程运行的标志，false表示可以在多个线程中运行 */
		GameThreadOnly = false
	};
};
