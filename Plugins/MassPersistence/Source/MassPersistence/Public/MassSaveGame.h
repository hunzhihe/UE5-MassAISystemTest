// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MassSaveGame.generated.h"

class UMassEntityConfigAsset;
struct FInstancedStruct;

//存储单个Mass实体的持久化数据，其中包括实体的片段数据和关联的配置资产
USTRUCT()
struct FEntitySaveData
{
	GENERATED_BODY()

	//用于存储实体的多个片段数据
	UPROPERTY()
	TArray<FInstancedStruct> EntityFragments;

	//存储实体的配置资产
	UPROPERTY()
	UMassEntityConfigAsset* ConfigAsset;
};
/**
 * 继承自 Unreal 的USaveGame（保存游戏基类），作为整个实体持久化数据的根容器，
 用于存储所有需要持久化的实体数据。
 */
UCLASS()
class MASSPERSISTENCE_API UMassSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	//存储所有被持久化的实体数据集合。每个元素对应一个实体的FEntitySaveData。
	UPROPERTY()
	TArray<FEntitySaveData> Entities;
	
};
