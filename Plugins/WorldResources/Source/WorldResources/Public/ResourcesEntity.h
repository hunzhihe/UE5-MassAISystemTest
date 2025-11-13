// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"

#include "ResourcesEntity.generated.h"

/**
 * 
 */
USTRUCT()
struct WORLDRESOURCES_API FResourceUserFragment : public FMassFragment
{
	GENERATED_BODY()

	
	UPROPERTY(VisibleAnywhere)
	FGameplayTagContainer Tags;
};
