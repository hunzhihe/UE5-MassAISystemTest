// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassMovementTypes.h"
#include "RTSAgentSubsystem.h"
#include "Unit/UnitFragments.h"
#include "RTSAgentTraits.generated.h"



class URTSFormationSubsystem;
/**
 * 
 */


USTRUCT()
struct RTSFORMATIONS_API FRTSFormationAgent : public FMassFragment
{
	GENERATED_BODY()

	FRTSFormationAgent() = default;

	//实体在单位（编队）中的偏移
	FVector3f Offset;
};

USTRUCT()
struct FRTSCellLocFragment : public FMassFragment
{
	GENERATED_BODY()

	FRTSCellLocFragment() = default;
	
	//存储实体当前所在的网格单元索引，用于快速空间查询
	RTSAgentHashGrid2D::FCellLocation CellLoc;
};

//单位中实体的运动配置
USTRUCT()
struct RTSFORMATIONS_API FRTSFormationSettings : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Formation")
	FMassMovementStyleRef WalkMovement;

	UPROPERTY(EditAnywhere, Category = "Formation")
	FMassMovementStyleRef RunMovement;
};

UCLASS()
class RTSFORMATIONS_API URTSFormationAgentTraits : public UMassEntityTraitBase
{
	GENERATED_BODY()
	

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	FRTSFormationSettings FormationSettings;
};
