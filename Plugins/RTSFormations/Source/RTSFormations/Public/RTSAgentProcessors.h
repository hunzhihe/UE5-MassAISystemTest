// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "MassObserverProcessor.h"

#include "RTSAgentProcessors.generated.h"

/**
 * 
 */
 // 标记需要被RTS哈希网格管理的实体
USTRUCT()
struct FRTSAgentHashTag : public FMassTag
{
	GENERATED_BODY();
};

// 实时更新实体在RTS哈希网格中的位置
UCLASS()
class RTSFORMATIONS_API URTSUpdateHashPosition : public UMassProcessor
{
	GENERATED_BODY()

	URTSUpdateHashPosition();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

// 在实体创建或首次添加 FRTSAgentHashTag 标签时，将其初始化为哈希网格的成员（计算初始位置并加入网格）
UCLASS()
class RTSFORMATIONS_API URTSInitializeHashPosition : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSInitializeHashPosition();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

// 当实体被销毁或不再需要被哈希网格管理时（如移除 FRTSAgentHashTag 标签），将其从哈希网格中移除。
UCLASS()
class RTSFORMATIONS_API URTSRemoveHashPosition : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSRemoveHashPosition();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
