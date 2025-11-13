// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomMovementTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "RandomMovementFragment.h"

/**
 * 构建随机移动特征的实体模板
 * 
 * @param BuildContext 实体模板构建上下文，用于添加片段和共享片段到实体模板中
 * @param World 当前的游戏世界引用，用于获取实体管理器
 * 
 * 此函数负责为具有随机移动特征的实体配置所需的片段和共享片段。
 * 它会添加随机移动片段以及相关的常量共享片段配置。
 */
void URandomMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 获取世界对应的实体管理器实例
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// 为实体添加随机移动功能所需的片段
	BuildContext.AddFragment<FRandomMovementFragment>();

	// 获取或创建随机移动设置的常量共享片段，并将其添加到构建上下文中
	auto SharedFragment = EntityManager.GetOrCreateConstSharedFragment(RandomMovementSettings);
	BuildContext.AddConstSharedFragment(SharedFragment);
}
