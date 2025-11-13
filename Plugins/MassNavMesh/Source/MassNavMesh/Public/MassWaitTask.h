// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include"MassStateTreeTypes.h"

#include "MassSignalSubsystem.h"

#include "StateTreeExecutionTypes.h"

#include "MassWaitTask.generated.h"

/**
 * 
 */
USTRUCT()
struct MASSNAVMESH_API FMassWaitTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Wait Task")
	float Duration = 0.f;

	float TimePassed = 0.f;
	
};

USTRUCT(meta = (DisplayName = "Mass Wait Task"))
struct MASSNAVMESH_API FMassWaitTask : public FMassStateTreeTaskBase
{ 
	GENERATED_BODY()

	using FInstanceDataType = FMassWaitTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassWaitTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};