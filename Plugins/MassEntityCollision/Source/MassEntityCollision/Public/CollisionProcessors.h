// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "CollisionProcessors.generated.h"

/**
 * 碰撞初始化处理器
 */
UCLASS()
class MASSENTITYCOLLISION_API UCollisionInitializerProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UCollisionInitializerProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
	
};


//碰撞销毁处理器，当实体的碰撞相关碎片被移除时，执行清理操作，将实体从哈希网格中移除，停止对其的碰撞跟踪。
UCLASS()
class MASSENTITYCOLLISION_API UCollisionDestroyProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UCollisionDestroyProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};

//碰撞处理器，周期性更新哈希网格中实体的位置，并检测和处理实体间的碰撞，确保碰撞查询的准确性（尤其是实体移动后）
UCLASS()
class MASSENTITYCOLLISION_API UCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()
	UCollisionProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
	FMassEntityQuery CollisionQuery;

	FVector ResolveCollisions(const TArray<FMassEntityHandle>& Entities, FMassEntityManager& EntityManager, float Radius,
		FTransform
		& EntityTransform);
};