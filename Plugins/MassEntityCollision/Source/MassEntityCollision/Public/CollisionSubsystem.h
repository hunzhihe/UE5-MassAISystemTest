// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HierarchicalHashGrid2D.h"
#include "MassEntityHandle.h"
#include "MassEntityTypes.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "CollisionSubsystem.generated.h"



class UMassEntityConfigAsset;

/*
* 基于模板THierarchicalHashGrid2D，通过typedef定义了一个分层哈希网格
* 2 表示哈希网格的维度（层级），
4 表示哈希网格的层数，即上层网格的单元格尺寸是下层的 4 倍（如下层单元格大小为 100 单位时，上层为 400 单位）。
FMassEntityHandle 表示哈希网格中存储的实体句柄。
* 
*/
typedef THierarchicalHashGrid2D<2, 4, FMassEntityHandle> FHashGridExample;

/**
 * 继承自UMassSubsystemBase，表明这是 Mass 框架下的子系统，会随世界（World）生命周期管理，且可被 Mass 处理器访问。
 */
UCLASS()
class MASSENTITYCOLLISION_API UCollisionSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()
	
public:
	//实例化的哈希网格对象，构造参数100表示底层网格的单元格大小为 100 单位（如厘米），用于实体的空间分区存储。
	FHashGridExample HashGridData = FHashGridExample(100);

	UFUNCTION(BlueprintCallable)
	void SpawnEntities(const FVector& Location, int Count, UMassEntityConfigAsset* EntityConfig);
};


template<>
struct TMassExternalSubsystemTraits<UCollisionSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};