// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MSRepresentationFragments.generated.h"


class ANiagaraEntityVizActor;
/**
 * @brief FMSRepresentationFragments结构体用于管理Niagara粒子系统的可视化表示片段
 * 
 * 该结构体继承自FMassSharedFragment，用于存储和管理大规模实体的粒子位置、
 * 方向和动画索引数据，供Niagara可视化系统使用。
 * 
 * 主要功能包括：
 * - 存储粒子位置数组
 * - 存储粒子方向数组  
 * - 存储动画索引数组
 * - 维护Niagara管理器Actor引用
 * - 记录迭代偏移量
 */
USTRUCT()
struct MASSNIAGARAVISUALIZATION_API FSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	/**
	 * @brief Niagara可视化管理器Actor的弱引用指针
	 * 
	 * 用于引用和管理Niagara粒子系统的实体可视化Actor
	 */
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<ANiagaraEntityVizActor> NiagaraManagerActor;

	/**
	 * @brief 迭代偏移量，用于计算当前处理的实体批次起始位置
	 * 
	 * 在大规模实体处理中，用于分批处理实体数据的偏移索引
	 */
	int32 IterationOffset = 0;

	/**
	 * @brief 粒子位置参数在Niagara系统中的参数名称
	 * 
	 * 静态常量，定义了传递给Niagara系统的粒子位置数据参数名
	 */
	inline static FName ParticlePositionName = "MassParticlePosition";

	/**
	 * @brief 粒子位置数组
	 * 
	 * 存储所有实体粒子在世界空间中的位置坐标
	 */
	UPROPERTY()
	TArray<FVector> ParticlePositions;

	/**
	 * @brief 粒子方向参数在Niagara系统中的参数名称
	 * 
	 * 静态常量，定义了传递给Niagara系统的粒子方向数据参数名
	 */
	inline static FName ParticleOrientationsParameterName = "MassParticleOrientations";

	/**
	 * @brief 粒子方向数组
	 * 
	 * 存储所有实体粒子的四元数方向信息
	 */
    UPROPERTY()
	TArray<FQuat4f> ParticleOrientations;

	/**
	 * @brief 动画索引参数在Niagara系统中的参数名称
	 * 
	 * 静态常量，定义了传递给Niagara系统的动画索引数据参数名
	 */
	inline static FName AnimationIndexesParameterName = "MassAnimationIndexes";

	/**
	 * @brief 动画索引数组
	 * 
	 * 存储每个实体对应的动画索引值，用于控制Niagara粒子的动画状态
	 */
	UPROPERTY()
    TArray<uint8> AnimationIndexes;
};
