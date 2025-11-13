// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/TickSTProcessor.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "MassStateTreeFragments.h"

UTickSTProcessor::UTickSTProcessor()
	: EntityQuery(*this)
{
	// 禁用自动注册处理阶段功能
    // 该变量控制是否自动将当前对象或模块注册到处理阶段系统中。
    // 当设置为false时，需要手动调用注册接口来完成注册过程。
    bAutoRegisterWithProcessingPhases = false;
}

void UTickSTProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
/**
 * 配置实体查询的子系统和片段需求
 * 
 * 该函数块用于设置Mass Entity系统的查询条件，包括必需的子系统、片段访问权限以及区块过滤条件。
 * 主要配置了信号子系统的读写权限、状态树实例片段的访问权限、模拟变量tick区块片段的只读权限，
 * 并设置了基于帧率的区块过滤器。
 */

// 添加信号子系统的读写权限需求
EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);

// 添加状态树实例片段的需求，不进行访问操作
EntityQuery.AddRequirement<FMassStateTreeInstanceFragment>(EMassFragmentAccess::None);

// 添加模拟变量tick区块片段的只读权限需求，该需求是可选的
EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);

// 设置区块过滤器，用于判断当前帧是否应该处理该区块
EntityQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UTickSTProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
/**
 * @brief 遍历实体查询结果中的所有区块，并为每个实体发送状态树激活信号
 * 
 * 此函数通过EntityQuery遍历所有匹配的实体区块，然后在每个区块中遍历所有实体，
 * 为每个实体向信号子系统发送StateTreeActivate信号，用于触发状态树的评估和任务执行。
 * 
 * @param Context 执行上下文，提供访问实体数据和子系统的接口
 * @note 在实际应用中应该添加节流机制，避免每帧都执行（如LOD、事件驱动等优化）
 */
// Iterate through chunks
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
		{
			/**
			 * @brief 获取信号子系统实例
			 * 
			 * 从执行上下文中获取可变的信号子系统引用，用于后续发送实体信号
			 */
			UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();

			// Iterate through entities in chunk
			/**
			 * @brief 遍历当前区块中的所有实体
			 * 
			 * 获取当前处理区块中的实体数量，并逐个处理每个实体
			 */
			const int32 NumEntities = Context.GetNumEntities();
			for (int32 EntityIndex = 0; EntityIndex < NumEntities; EntityIndex++)
			{
				/**
				 * @brief 发送状态树激活信号给实体
				 * 
				 * 向信号子系统发送StateTreeActivate信号，触发该实体的状态树进行评估和执行。
				 * 包括 evaluators、global tasks 等组件的更新逻辑。
				 * 注意：在真实环境中应实现节流机制而非每tick执行，并考虑基于事件的按需执行策略
				 */
				SignalSubsystem.SignalEntityDeferred(Context, UE::Mass::Signals::StateTreeActivate, Context.GetEntity(EntityIndex));
			}
		});
}
