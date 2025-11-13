// Fill out your copyright notice in the Description page of Project Settings.


#include "MSNiagaraRepresentationTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "NiagaraEntityVizSubsystem.h"
#include "MSRepresentationFragments.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,const UWorld& World) const
{
	/**
 * 初始化Niagara可视化系统的共享片段
 * 
 * 该函数负责加载必要的资源并创建或获取Niagara系统所需的共享片段，
 * 用于在Mass Entity系统中渲染Niagara效果
 * 
 * @param World 当前的游戏世界对象，用于获取子系统和实体管理器
 * @param BuildContext 构建上下文，用于添加共享片段和检查数据状态
 * @param StaticMesh 静态网格资源，需要同步加载用于Niagara渲染
 * @param SharedNiagaraSystem 共享的Niagara系统资源，需要同步加载
 * @param MaterialOverride 材质覆盖资源，可选，需要同步加载
 */
// 获取世界中的实体管理器引用
FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

// 同步加载所有必需的资源
StaticMesh.LoadSynchronous();
SharedNiagaraSystem.LoadSynchronous();
MaterialOverride.LoadSynchronous();

// 获取Niagara实体可视化子系统
UNiagaraEntityVizSubsystem* NiagaraSubsystem = UWorld::GetSubsystem<UNiagaraEntityVizSubsystem>(&World);

// 声明需要使用变换片段
BuildContext.RequireFragment<FTransformFragment>();

// 初始化材质指针
UMaterial* Material = nullptr;

// 如果存在材质覆盖则执行相应逻辑（代码未实现）
if (MaterialOverride)
{
}

// 根据是否处于数据检查模式来决定如何处理共享片段
if (!BuildContext.IsInspectingData())
{
	// 正常模式：创建或获取与系统类型关联的共享Niagara片段
	FSharedStruct SharedFragment = NiagaraSubsystem->GetOrCreateSharedNiagaraFragmentForSystemType(SharedNiagaraSystem.Get(),
		StaticMesh.Get(), MaterialOverride.Get());
	BuildContext.AddSharedFragment(SharedFragment);
}
else
{
	// 检查模式：获取或创建通用的共享Niagara系统片段
	FSharedStruct SharedFragment = EntityManager.GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>();
	BuildContext.AddSharedFragment(SharedFragment);
}
}
