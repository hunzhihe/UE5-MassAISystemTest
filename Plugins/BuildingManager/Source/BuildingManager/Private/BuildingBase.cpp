// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingBase.h"
#include "Components/InstancedStaticMeshComponent.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Building_Constructed, "Building.Constructed")

ABuildingBase::ABuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedStaticMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMesh"));
    InstancedStaticMesh->SetupAttachment(RootComponent);
}

void ABuildingBase::ConstructBuilding()
{
	FVector InstanceTransform = InstancedStaticMesh->GetInstanceCount() * FVector(0.0f,0.0f,1.0f) * 300.f;

	InstancedStaticMesh->AddInstance(FTransform(InstanceTransform));
}

void ABuildingBase::BeginPlay()
{
    Super::BeginPlay();
}
