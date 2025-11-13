// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MassStateTreeTypes.h"
#include "MW_FindRandomLocationTask.generated.h"

class UMassSignalSubsystem;
struct FTransformFragment;


/**
 * @brief 随机位置查找任务实例数据结构体
 * 
 * 该结构体用于存储随机位置查找任务的实例数据，包含搜索范围和输出位置信息。
 * 主要用于质量流浪者AI系统中的行为树任务节点。
 */
USTRUCT()
struct MASSWANDERAI_API FMW_FindRandomLocationTaskInstanceData
{
	GENERATED_BODY()

	/**
	 * @brief 搜索范围半径
	 * 
	 * 定义随机位置生成的范围，以当前位置为中心点的圆形区域内随机选择目标位置。
	 * 默认值为1000.0单位距离。
	 */
	UPROPERTY(EditAnywhere, Category = Input)
	float Range = 1000.f;

	/**
	 * @brief 输出的位置坐标
	 * 
	 * 存储随机生成的目标位置坐标，供后续任务节点使用。
	 */
	UPROPERTY(EditAnywhere, Category = Output)
	FVector OutLocation;

	/**
	 * @brief 默认构造函数
	 * 
	 * 使用默认实现初始化结构体成员变量。
	 */
	FMW_FindRandomLocationTaskInstanceData() = default;
};
/**
 * 
 */
USTRUCT(meta = (DisplayName = "MW Find Random Location"))
struct MASSWANDERAI_API FMW_FindRandomLocationTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMW_FindRandomLocationTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMW_FindRandomLocationTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
protected:
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
};
