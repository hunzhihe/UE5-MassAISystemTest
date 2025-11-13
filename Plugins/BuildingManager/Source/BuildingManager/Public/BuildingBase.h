// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericSmartObject.h"
#include "NativeGameplayTags.h"
#include "BuildingBase.generated.h"


class UInstancedStaticMeshComponent;
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Building_Constructed);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "Building")
class BUILDINGMANAGER_API ABuildingBase : public AGenericSmartObject
{
	GENERATED_BODY()
	
	public:
        ABuildingBase();

        UFUNCTION(BlueprintCallable, Category = "Building")
        void ConstructBuilding();
	protected:
        virtual void BeginPlay() override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
        TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMesh;

	    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
        int Floors = 1;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
		int CurrentFloor = 0;
};
