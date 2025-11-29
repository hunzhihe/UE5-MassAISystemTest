// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MassEntityConfigAsset.h"

#include "MassEntityTraitBase.h"

#include "MassEntityTypes.h"
#include "PersistentDataTrait.generated.h"



//存储实体配置信息
USTRUCT()
struct FPersistentDataFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMassEntityConfigAsset> EntityConfig;
};

//标记需要持久化的实体
USTRUCT()
struct FPersistentDataTag : public FMassTag
{
	GENERATED_BODY()

};

//记录实体需要持久化的变换（位置）数据
USTRUCT()
struct FPersistentTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;
};
/**
 * 
 */
UCLASS()
class MASSPERSISTENCE_API UPersistentDataTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	

	UPROPERTY(EditDefaultsOnly)
	FPersistentDataFragment PersistentDataFragment;

	void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
