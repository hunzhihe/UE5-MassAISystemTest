// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TickSTProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSSMARTOBJECTAI_API UTickSTProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	UTickSTProcessor();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
