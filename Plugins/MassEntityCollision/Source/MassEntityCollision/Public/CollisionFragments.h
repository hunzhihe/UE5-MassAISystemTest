// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CollisionSubsystem.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "CollisionFragments.generated.h"

/**
 * 
 */
namespace HashGridExample::Signals
{
	const FName EntityQueried = FName(TEXT("EntityQueried"));
}

//该片段记录实体在哈希网格中所处的单元格位置
USTRUCT()
struct MASSENTITYCOLLISION_API FCollisionFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	FHashGridExample::FCellLocation CellLocation;
};


//该特性记录实体在哈希网格中所处的单元格位置
UCLASS()
class MASSENTITYCOLLISION_API UCollisionFragments : public UMassEntityTraitBase
{
	GENERATED_BODY()

	public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,const UWorld& World) const override;
	
};
