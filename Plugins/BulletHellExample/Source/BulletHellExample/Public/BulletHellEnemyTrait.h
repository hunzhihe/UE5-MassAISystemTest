// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BulletHellSubsystem.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "BulletHellEnemyTrait.generated.h"

/**
 * 
 */
/**
 * @brief 敌人碎片结构体，用于存储敌人的基本属性信息
 * 
 * 该结构体继承自FMassFragment，包含了敌人的生命值、碰撞范围和网格位置信息，
 * 主要用于大规模敌人实体的属性管理。
 */
USTRUCT()
struct BULLETHELLEXAMPLE_API FBHEnemyFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 敌人生命值，可在编辑器中修改 */
	UPROPERTY(EditAnywhere)
	float Health;

	/** 敌人碰撞检测的范围，默认值为(100, 100, 100) */
	UPROPERTY(EditAnywhere)
	FVector CollisionExtent = FVector(100.f);

	/** 敌人在实体哈希网格中的位置信息 */
	FBHEntityHashGrid::FCellLocation CellLocation;
};

/**
 * @brief 敌人标签结构体，用于标识实体为敌人类型
 * 
 * 该结构体继承自FMassTag，作为一个标记结构体来区分敌人实体，
 * 不包含具体的数据成员，仅作为类型标识使用。
 */
USTRUCT()
struct BULLETHELLEXAMPLE_API FBHEnemyTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * @brief 子弹地狱敌人特征类，用于构建敌人实体模板
 * 
 * 该类继承自UMassEntityTraitBase，负责定义和配置敌人实体的特征模板，
 * 包含了敌人碎片数据和其他相关属性设置。
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBulletHellEnemyTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:

	/**
	 * @brief 构建实体模板的核心函数
	 * 
	 * 该函数重写了父类的虚函数，用于根据传入的构建上下文和世界信息
	 * 来配置和创建敌人实体的模板。
	 * 
	 * @param BuildContext 实体模板构建上下文，包含构建过程中需要的各种信息和工具
	 * @param World 当前的游戏世界引用，提供世界相关的上下文信息
	 */
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	/** 敌人碎片数据，包含在"Bullet Hell"分类下，可在编辑器中编辑 */
	UPROPERTY(Category = "Bullet Hell", EditAnywhere)
	FBHEnemyFragment BHEnemyFragment;
	
};
