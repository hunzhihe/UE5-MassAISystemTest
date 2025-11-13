// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "RandomMovementFragment.generated.h"

/**
 * RandomMovement命名空间
 * 包含随机移动相关的常量定义
 */
namespace RandomMovement
{
	/**
	 * 定义位置到达事件的名称常量
	 * 用于标识实体到达目标位置的事件
	 */
	const FName LocationReached = FName(TEXT("LocationReached"));
}

/**
 * 随机移动片段结构体
 * 继承自FMassFragment，用于存储随机移动相关的片段数据
 * 作为Mass Entity系统中的数据组件
 */
USTRUCT()
struct  FRandomMovementFragment : public FMassFragment
{
	GENERATED_BODY()
};

/**
 * 随机移动设置片段结构体
 * 继承自FMassConstSharedFragment，用于存储随机移动的配置参数
 * 作为Mass Entity系统中的共享常量片段，可在蓝图中编辑和读取
 */
USTRUCT(BlueprintType)
struct FRandomMovementSettingsFragment :public FMassConstSharedFragment
{
    GENERATED_BODY()

    /**
     * 新位置生成延迟时间（秒）
     * 控制实体在当前位置停留多长时间后寻找新位置
     * 可在编辑器中编辑，蓝图可读写
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NewLocationDelay = 1.f;

    /**
     * 新位置生成半径
     * 控制实体在当前位置周围多大范围内随机选择新位置
     * 可在编辑器中编辑，蓝图只读
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float NewLocationRadius = 500.f;
};