// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityQuery.h"
#include "MassEntityTypes.h"
#include "MassSignalProcessorBase.h"
#include "MassSignalSubsystem.h"
#include "RTSFormationSubsystem.h"
#include "LaunchEntityProcessor.generated.h"

/**
 * 
 */
const FName LaunchEntity = FName(TEXT("LaunchEntity"));

// 用于响应 LaunchEntity信号，当 LaunchEntity 信号触发时，对符合条件的实体执行发射逻辑
UCLASS()
class RTSFORMATIONS_API ULaunchEntityProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
	TObjectPtr<URTSFormationSubsystem> FormationSubsystem;
};

// 用于周期性处理实体的运动力逻辑，筛选出需要处理的实体，并对实体施加力，更新其运动状态。
UCLASS()
class RTSFORMATIONS_API UMoveForceProcessor : public UMassProcessor
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery = FMassEntityQuery(*this);
};

// 用于存储发射实体的参数，发射起点，发射力度
USTRUCT()
struct RTSFORMATIONS_API FLaunchEntityFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float Magnitude = 500.f;
};

//处理器可通过该标签快速筛选出刚触发发射的实体，执行初始化逻辑（如设置初始速度、绑定后续运动处理器）。
USTRUCT()
struct RTSFORMATIONS_API FInitLaunchFragment : public FMassTag
{
	GENERATED_BODY()
};
