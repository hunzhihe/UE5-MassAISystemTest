// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "FormationPresets.h"
#include "MassEntityElementTypes.h"

#include <MassEntityHandle.h>
#include "UnitFragments.generated.h"

/**
 * 
 */


//单位ID的计数器，用于生成唯一的UnitID.
static int16 UnitNum = 0;

//单位句柄，用于唯一标识游戏中的单位，每次创建FUnitHandle实例时，UnitNum会自增，确保每个单位的 ID 唯一。
USTRUCT(BlueprintType)
struct FUnitHandle
{
	GENERATED_BODY()

	FUnitHandle()
	{
		UnitID = UnitNum++;
	};

	UPROPERTY()
	int16 UnitID;

   /* UPROPERTY()
	TArray<FMassEntityHandle> Entities;*/

	//运行符重载，用于比较两个FUnitHandle是否指向同一单位
	bool operator==(const FUnitHandle Other) const
	{
		return UnitID == Other.UnitID;
	}

	bool operator!=(const FUnitHandle Other) const
	{
		return !operator==(Other);
	}
	//提供哈希值计算（基于UnitID），使FUnitHandle可用于哈希表
	friend uint32 GetTypeHash(const FUnitHandle Entity)
	{
		return Entity.UnitID;
	}
};

//单位配置，存储单位编队和移动的配置参数
USTRUCT()
struct FUnitSettings
{
	GENERATED_BODY()

	float InterpolationSpeed = 5.f;
	int FormationLength = 8; 
	float BufferDistance = 100.f;
	int Rings = 2; 
	EFormationType Formation = EFormationType::Rectangle; 
	bool bHollow = false;
};
/**
 * @brief 单位片段结构体，用于存储单位相关的信息和状态
 * 
 * 继承自FMassSharedFragment，包含单位句柄、目标位置、旋转信息、
 * 插值移动数据以及单位设置等属性。主要用于Mass Entity系统中
 * 表示和管理游戏中的单位实体。
 */
USTRUCT()
struct FUnitFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	FUnitFragment() = default;

	UPROPERTY()
	FUnitHandle UnitHandle;

	/** 实体在单位中的目标位置 */
	FVector3f UnitDestination = FVector3f::ZeroVector;
	
	/** 单位的旋转方向 */
	FRotator3f UnitRotation;

	/** 插值移动的目标位置 */
	FVector3f InterpDestination = FVector3f::ZeroVector;
	
	/** 插值移动的旋转方向 */
	FRotator3f InterpRotation;

	/** 单位的前进方向（XZ平面） */
	FVector2f ForwardDir;

	/** 单位的配置设置 */
	FUnitSettings UnitSettings;

	/**
	 * @brief 比较两个单位片段是否相等
	 * @param OtherUnitFragment 要比较的另一个单位片段
	 * @return bool 如果两个单位片段的UnitHandle相等则返回true，否则返回false
	 */
	bool operator==(const FUnitFragment& OtherUnitFragment) const
	{
		return OtherUnitFragment.UnitHandle == UnitHandle;
	}

	/**
	 * @brief 比较单位片段与单位句柄是否相等
	 * @param OtherUnitHandle 要比较的单位句柄
	 * @return bool 如果单位句柄相等则返回true，否则返回false
	 */
	bool operator==(const FUnitHandle& OtherUnitHandle) const
	{
		return OtherUnitHandle == UnitHandle;
	}
};
