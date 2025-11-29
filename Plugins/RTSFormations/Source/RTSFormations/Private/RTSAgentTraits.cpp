// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTraits.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassObserverRegistry.h"
#include "Engine/World.h"

/**
 * 构建实体模板，用于RTS编队代理的初始化配置
 * 
 * @param BuildContext 实体模板构建上下文，用于添加片段和共享片段到实体模板中
 * @param World 当前游戏世界对象的引用，用于获取实体子系统和实体管理器
 */
void URTSFormationAgentTraits::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 获取世界中的实体子系统实例
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	
	// 获取世界中的实体管理器引用
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	check(EntitySubsystem);

	// 添加RTS编队代理所需的片段
	BuildContext.AddFragment<FRTSFormationAgent>();

	// 获取或创建编队设置共享片段，并将其添加到构建上下文中
	auto& FormationSettingsSharedStruct = EntityManager.GetOrCreateConstSharedFragment<FRTSFormationSettings>(FormationSettings);
	BuildContext.AddConstSharedFragment(FormationSettingsSharedStruct);

	// 添加变换片段，用于存储实体的位置、旋转和缩放信息
	BuildContext.AddFragment<FTransformFragment>();
}
