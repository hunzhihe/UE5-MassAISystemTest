// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "MassSignalProcessorBase.h"
#include "NiagaraRepresentationProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSNIAGARAVISUALIZATION_API UNiagaraRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	public:
    UNiagaraRepresentationProcessor();

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery PositionToNiagaraFragmentQuery;
	FMassEntityQuery NiagaraPositionChunkQuery;
	
};
