// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MassEntityQuery.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "RTSFormationProcessors.generated.h"

//编队初始化处理器,监听单位（Unit）生成事件，将新实体添加到子系统的单位数组中，并缓存实体索引，为后续编队位置计算（URTSFormationUpdate）提供基础数据。
UCLASS()
class RTSFORMATIONS_API URTSFormationInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationInitializer();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

// 编队初始化处理器, 监听实体销毁事件，负责清理子系统中单位数组的关联数据（如移除已销毁实体），并调整剩余实体的索引（例如让最后一个实体填补被销毁实体的位置，维持数组连续性）。
UCLASS()
class RTSFORMATIONS_API URTSFormationDestroyer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationDestroyer();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};


// 移动处理器，负责将单位（Unit）的移动目标设置为编队位置实现实体的基础移动逻辑，负责将实体从当前位置平滑移动到目标位置（如编队中计算出的位置）。
UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery = FMassEntityQuery(*this);

	FMassEntityQuery FormationQuery = FMassEntityQuery(*this);
};

// 编队更新处理器，编队系统的核心逻辑处理器，负责计算编队中每个实体的目标位置，并将位置信息同步到移动组件（FMassMoveTargetFragment），驱动实体按编队排列。
UCLASS()
class RTSFORMATIONS_API URTSFormationUpdate : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
};