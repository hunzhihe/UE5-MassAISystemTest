// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentDataTrait.h"

#include "MassEntityTemplateRegistry.h"

/**
 * @brief 构建实体模板，添加持久化数据相关的片段和标签
 * 
 * 该函数用于在实体模板构建过程中添加持久化数据所需的共享片段、
 * 标签和变换片段，确保实体具备持久化存储能力
 * 
 * @param BuildContext 实体模板构建上下文，用于添加各种片段和标签
 * @param World 当前世界对象，用于获取实体管理器
 */
void UPersistentDataTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 获取世界对应的实体管理器实例
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// 创建或获取持久化数据共享片段，并添加到构建上下文中
	FConstSharedStruct ParamsFragment = EntityManager.GetOrCreateConstSharedFragment(PersistentDataFragment);
	BuildContext.AddConstSharedFragment(ParamsFragment);

	// 添加持久化数据标签和变换片段到实体模板中
	BuildContext.AddTag<FPersistentDataTag>();
	BuildContext.AddFragment<FPersistentTransformFragment>();

}
