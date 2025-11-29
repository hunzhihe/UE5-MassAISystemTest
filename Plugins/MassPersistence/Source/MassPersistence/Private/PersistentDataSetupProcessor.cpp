// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentDataSetupProcessor.h"

#include "MassExecutionContext.h"
#include "MassPersistentDataSubsystem.h"
#include "PersistentDataTrait.h"

/**
 * 构造函数，初始化UPersistentDataInitializerProcessor对象
 * 
 * 初始化观察类型为FPersistentDataTag结构体，并设置操作类型为添加操作。
 * 同时初始化基类EntityQuery成员。
 */
UPersistentDataInitializerProcessor::UPersistentDataInitializerProcessor():EntityQuery(*this)
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

/**
 * 配置查询所需的子系统依赖
 * 
 * @param EntityManager 实体管理器的共享引用，用于配置查询条件
 * 
 * 该函数向实体查询中添加对UMassPersistentDataSubsystem子系统的读写需求，
 * 确保在执行时能够访问和修改持久化数据子系统。
 */
void UPersistentDataInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * 执行处理器的主要逻辑
 * 
 * @param EntityManager 实体管理器引用，提供实体管理功能
 * @param Context 执行上下文引用，包含当前执行环境的信息
 * 
 * 该函数遍历所有符合条件的实体块，将每个实体添加到持久化数据子系统的托管实体列表中。
 * 主要用于初始化阶段，将需要持久化管理的实体注册到相应的子系统中。
 */
void UPersistentDataInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历所有实体块_chunk_
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取可变的持久化数据子系统引用
			auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();

			// 获取当前块中的实体数量
			const int32 NumEntities = Context.GetNumEntities();
			
			// 遍历当前块中的所有实体，将其添加到托管实体列表中
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				PersistentDataSubsystem.ManagedEntities.Emplace(Context.GetEntity(EntityIdx));
			}
		});
}




/**
 * 构造函数，初始化UPersistentDataDestructorProcessor对象
 * 
 * 初始化观察类型为FPersistentDataTag的静态结构体，
 * 设置操作类型为移除操作，并初始化EntityQuery成员。
 */
UPersistentDataDestructorProcessor::UPersistentDataDestructorProcessor():EntityQuery(*this)
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

/**
 * 配置查询条件
 * 
 * @param EntityManager 共享引用的实体管理器，用于配置查询所需的子系统依赖
 * 
 * 该函数向EntityQuery添加对UMassPersistentDataSubsystem子系统的读写需求，
 * 确保在执行时能够访问和修改持久化数据子系统。
 */
void UPersistentDataDestructorProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * 执行处理器逻辑
 * 
 * @param EntityManager 实体管理器引用，提供对实体的管理功能
 * @param Context 执行上下文引用，包含当前批次执行的相关信息
 * 
 * 该函数遍历所有符合条件的实体块，收集需要移除的实体，
 * 并从PersistentDataSubsystem的ManagedEntities中移除这些实体。
 */
void UPersistentDataDestructorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历每个实体块并处理其中的实体
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取可变的持久化数据子系统引用
			auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();

			// 收集当前上下文中所有需要移除的实体
			TSet<FMassEntityHandle> EntitiesToRemove;
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				EntitiesToRemove.Add(Context.GetEntity(EntityIdx));
			}

			// 从ManagedEntities中移除所有标记为删除的实体
			PersistentDataSubsystem.ManagedEntities.RemoveAllSwap([&EntitiesToRemove](FMassEntityHandle& Entity)
				{
					return EntitiesToRemove.Contains(Entity);
				});
		});
}
