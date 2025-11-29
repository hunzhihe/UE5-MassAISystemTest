// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"

#include "MassProcessor.h"


#include "MassSignalProcessorBase.h"
#include "HashGridExampleProcessors.generated.h"

/**
 * 实体初始化时添加到哈希网格
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridInitializeProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UHashGridInitializeProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
	
};

// 实体销毁时从哈希网格中移除
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridDestroyProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UHashGridDestroyProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};

// 实体移动时更新其在哈希网格中的位置
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridProcessor : public UMassProcessor
{
	GENERATED_BODY()
	UHashGridProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};




//处理实体查询信号，对查询到的实体执行操作（比如销毁）
UCLASS()
class UHashGridQueryProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
};

//UCLASS()
//class SPATIALHASHGRIDEXAMPLE_API USimpleVisualizationProcessor : public UMassVisualizationProcessor
//{
//	GENERATED_BODY()
//    USimpleVisualizationProcessor();
//};
//
//UCLASS()
//class SPATIALHASHGRIDEXAMPLE_API USimpleVisualizationLODProcessor : public UMassVisualizationLODProcessor
//{
//	GENERATED_BODY()
//	
//	USimpleVisualizationLODProcessor();
//};