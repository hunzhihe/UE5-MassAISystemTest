// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalProcessorBase.h"
#include "MassEntityDataProcessor.generated.h"

/**
 * 该处理器用于响应SaveEntity信号，序列化实体碎片数据（如 FPersistentTransformFragment）到 FEntitySaveData。
 */
UCLASS()
class MASSPERSISTENCE_API UPersistEntityDataProcessor : public UMassSignalProcessorBase
{
	UPersistEntityDataProcessor();
	FMassEntityQuery EntityQuery;
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;



private:
	GENERATED_BODY()
};

//响应 EntityLoaded 信号，加载后恢复实体变换、配置等状态。
UCLASS()
class MASSPERSISTENCE_API UPersistentDataPostLoadProcessor : public UMassSignalProcessorBase
{
	UPersistentDataPostLoadProcessor();
	FMassEntityQuery EntityQuery;
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;



private:
	GENERATED_BODY()
};