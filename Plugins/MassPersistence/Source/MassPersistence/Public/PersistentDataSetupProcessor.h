// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "PersistentDataSetupProcessor.generated.h"

/**
 * 
 */
 // 监听实体创建事件，为带 FPersistentDataTag 的实体添加到子系统的管理列表。
UCLASS()
class UPersistentDataInitializerProcessor : public UMassObserverProcessor
{
public:
	UPersistentDataInitializerProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};

// 监听实体销毁事件，从管理列表移除实体。
UCLASS()
class UPersistentDataDestructorProcessor : public UMassObserverProcessor
{
public:
	UPersistentDataDestructorProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};
