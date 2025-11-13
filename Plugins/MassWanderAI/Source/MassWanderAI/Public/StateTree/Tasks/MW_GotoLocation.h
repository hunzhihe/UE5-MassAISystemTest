// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MW_GotoLocation.generated.h"

class UMassSignalSubsystem;
struct FTransformFragment;
struct FMassMoveTargetFragment;

/**
 * @brief GotoLocation行为节点的实例数据结构体
 * 
 * 该结构体用于存储和传递GotoLocation行为节点在执行过程中的数据，
 * 包含目标位置和智能体当前位置等信息。
 */
USTRUCT()
struct MASSWANDERAI_API FMW_GotoLocationInstanceData
{
	GENERATED_BODY()

	/**
	 * @brief 目标位置
	 * 
	 * 存储智能体需要前往的目标世界坐标位置
	 */
	UPROPERTY(EditAnywhere, Category = Input)
	FVector Destination;

	/**
	 * @brief 智能体当前位置
	 * 
	 * 存储智能体当前的世界坐标位置，通常作为输出数据使用
	 */
	UPROPERTY(EditAnywhere, Category = Output)
	FVector AgentLocation;

	/**
	 * @brief 默认构造函数
	 * 
	 * 使用默认实现初始化结构体成员
	 */
	FMW_GotoLocationInstanceData() = default;
};
/**
 * 
 */



USTRUCT(meta = (DisplayName = "MW Goto Location"))
struct MASSWANDERAI_API FMW_GotoLocation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMW_GotoLocationInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMW_GotoLocationInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
protected:
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
