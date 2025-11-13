// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectHandler.h"
#include "MassSmartObjectRequest.h"
#include "MassStateTreeTypes.h"

#include "MassFindResources.generated.h"

/**
 * 
 */
struct FTransformFragment;
struct FResourceUserFragment;

/**
 * FMassFindResourceInstanceData 结构体
 * 
 * 用于在大规模生态系统中查找资源实例的数据结构。
 * 包含查找结果信息、候选插槽以及资源配置参数。
 */
USTRUCT()
struct MASSSYSTEMTEST_API FMassFindResourceInstanceData
{
	GENERATED_BODY()

	
	/** 查找智能对象的结果标志，表示是否找到了符合条件的资源 */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bFoundSmartObject = false;

	/** 找到的智能对象候选插槽集合，存储可用的资源位置信息 */
	UPROPERTY(EditAnywhere, Category = Output)
	FMassSmartObjectCandidateSlots FoundSlots;

	/** 木材资源标签，用于标识和筛选木材类型的资源 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag WoodResourceTag;

	/** 石头资源标签，用于标识和筛选石头类型的资源 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag RockResourceTag;

	/** 智能对象请求ID，用于跟踪和管理资源查找请求 */
	FMassSmartObjectRequestID RequestID;
};

USTRUCT(meta = (DisplayName = "Mass Find Resources"))
struct MASSSYSTEMTEST_API FMassFindResource : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassFindResourceInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct();}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;

protected:
	//TStateTreeExternalDataHandle<FResourceUserFragment> MassResourceUserHandle;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FResourceUserFragment> ResourceUserHandle;
};