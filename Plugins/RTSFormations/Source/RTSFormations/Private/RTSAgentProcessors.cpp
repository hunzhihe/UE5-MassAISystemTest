// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentProcessors.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "RTSAgentTraits.h"
#include "Engine/World.h"

//----------------------------------------------------------------------//
//  URTSUpdateHashPosition
//
//  概述:
//      此类用于更新实体在哈希网格中的位置。它通过查询具有特定片段和标签的实体，
//      并根据实体当前的位置与半径信息重新计算其所在的网格单元位置，并更新到子系统中。
//
//  执行流程:
//      1. 配置需要处理的实体查询条件；
//      2. 在执行阶段遍历所有符合条件的实体块（chunk）；
//      3. 对每个实体获取其变换、当前位置及半径等数据；
//      4. 根据新位置更新该实体在哈希网格中的索引位置；
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
//  ConfigureQueries
//
//  功能描述:
//      设置 EntityQuery 查询所需的片段和标签要求，以便筛选出参与操作的实体集合.
//
//  参数说明:
//      EntityManager - 实体管理器引用，用于访问和管理系统内的实体及相关资源.
//
//  返回值说明:
//      无返回值.
//
//  备注:
//      添加了以下需求：
//          - FTransformFragment: 只读访问，用于获取实体的世界坐标；
//          - FRTSCellLocFragment: 读写访问，记录实体所在网格单元位置；
//          - FAgentRadiusFragment: 只读访问，表示实体的影响范围或碰撞半径；
//          - FRTSAgentHashTag: 必须存在此标签以标识属于 RTS 哈希系统的代理；
//          - URTSAgentSubsystem: 读写权限，用于实际进行哈希网格的操作；
//----------------------------------------------------------------------//
void URTSUpdateHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
}

//----------------------------------------------------------------------//
//  构造函数
//
//  功能描述:
//      初始化 URTSUpdateHashPosition 类实例，并将 EntityQuery 绑定至当前对象上下文.
//
//  参数说明:
//      无显式参数，使用初始化列表绑定 EntityQuery 到 *this 上下文.
//
//  返回值说明:
//      无返回值.
//----------------------------------------------------------------------//
URTSUpdateHashPosition::URTSUpdateHashPosition()
	: EntityQuery(*this)
{
}

//----------------------------------------------------------------------//
//  Execute
//
//  功能描述:
//      执行主逻辑，遍历所有匹配查询条件的实体块并逐个更新它们在哈希网格中的位置.
//
//  参数说明:
//      EntityManager - 实体管理器引用，提供对整个实体系统的访问能力；
//      Context       - 当前执行上下文，包含正在处理的一组实体及其相关数据视图；
//
//  返回值说明:
//      无返回值.
//
//  关键步骤说明:
//      1. 使用 EntityQuery 遍历所有符合要求的实体块；
//      2. 获取各片段的数据视图，包括只读和可变片段；
//      3. 循环处理每一块中的每一个实体；
//      4. 计算新的边界框，并调用 AgentSubsystem 更新哈希网格中的位置；
//----------------------------------------------------------------------//
void URTSUpdateHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历所有满足查询条件的实体块(chunk)
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取各个片段的只读/可写视图
			TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
			auto CellLocFragments = Context.GetMutableFragmentView<FRTSCellLocFragment>();
			TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
			auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();

			// 遍历当前 chunk 中的所有实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// 获取当前实体的相关数据
				auto& CellLocFragment = CellLocFragments[EntityIndex];
				const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
				const float Radius = RadiusFragments[EntityIndex].Radius;

				// 构建新的包围盒（忽略Z轴变化），仅考虑XY平面内影响范围
				const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));

				// 调用子系统接口移动实体在哈希网格中的位置
				CellLocFragment.CellLoc = AgentSubsystem.AgentHashGrid.Move(Context.GetEntity(EntityIndex), CellLocFragment.CellLoc, NewBounds);
			}
		});
}

//----------------------------------------------------------------------//
//  URTSInitializeHashPosition
//
//  功能描述:
//    此类用于初始化实体在哈希网格中的位置信息。它继承自 EntityQuery，并通过查询特定类型的实体来为其分配哈希网格坐标，
//    并将这些实体注册到 URTSAgentSubsystem 中进行统一管理。同时为实体添加 FRTSAgentHashTag 标签以标识其已处理完毕。
//
//  构造函数说明:
//    URTSInitializeHashPosition()
//      初始化 ObservedType 为 FRTSFormationAgent 类型结构体，并设置 Operation 操作类型为 Add（新增）操作，表示该处理器关注的是新加入的此类实体
//
//----------------------------------------------------------------------//
URTSInitializeHashPosition::URTSInitializeHashPosition()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}


//----------------------------------------------------------------------//
//  ConfigureQueries
//
//  功能描述:
//    配置当前处理器所使用的 EntityQuery 查询条件，包括需要读写的 Fragment 和 Subsystem 要求等
//
//  参数说明:
//    EntityManager - 实体管理器引用，用于注册和配置查询逻辑
//
//----------------------------------------------------------------------//
void URTSInitializeHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加对 FRTSCellLocFragment 的读写权限要求（用于记录网格位置）
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadWrite);

	// 添加对 FTransformFragment 的只读权限要求（获取实体世界坐标）
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	// 添加对 FAgentRadiusFragment 的只读权限要求（获取代理半径，用于构建边界框）
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);

	// 要求实体不具有 FRTSAgentHashTag 标签（避免重复处理）
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::None);

	// 添加对 URTSAgentSubsystem 子系统的读写访问权限（用于向哈希网格中添加实体）
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);

	// 将此查询与当前 Processor 关联起来以便执行时使用
	EntityQuery.RegisterWithProcessor(*this);
}

//----------------------------------------------------------------------//
//  Execute
//
//  功能描述:
//    执行主逻辑：遍历所有符合条件的实体块（chunk），计算每个实体的位置并将其插入到哈希网格中，然后打上标签防止再次处理
//
//  参数说明:
//    EntityManager - 实体管理器对象，提供对实体数据的操作接口
//    Context       - 当前执行上下文，包含当前批次的数据视图及执行环境
//
//----------------------------------------------------------------------//
void URTSInitializeHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 对每一个实体块进行迭代处理
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取变换片段、单元格位置片段以及半径片段的只读/可变视图
			TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
			auto CellLocFragments = Context.GetMutableFragmentView<FRTSCellLocFragment>();
			TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();

			// 获取 RTS Agent 子系统实例（必须存在且可修改）
			auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();

			// 遍历当前 chunk 内的所有实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// 获取当前实体对应的 CellLoc 片段引用
				auto& CellLocFragment = CellLocFragments[EntityIndex];

				// 提取实体的世界坐标
				const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();

				// 提取实体的碰撞半径
				const float Radius = RadiusFragments[EntityIndex].Radius;

				// 基于当前位置和半径构造一个二维包围盒（忽略 Z 轴变化）
				const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));

				// 日志输出当前哈希网格中的总实体数（调试用途）
				UE_LOG(LogTemp, Log, TEXT("Agents: %d"), AgentSubsystem.AgentHashGrid.GetItems().Num());

				// 将当前实体及其包围盒添加进哈希网格，并更新其所在的网格索引
				CellLocFragment.CellLoc = AgentSubsystem.AgentHashGrid.Add(Context.GetEntity(EntityIndex), NewBounds);

				// 推迟执行：给当前实体添加 FRTSAgentHashTag 标签，标记已完成初始化
				Context.Defer().AddTag<FRTSAgentHashTag>(Context.GetEntity(EntityIndex));
			}
		});
}
//----------------------------------------------------------------------//
//  URTSRemoveHashPosition
//----------------------------------------------------------------------//

/**
 * 构造函数，初始化URTSRemoveHashPosition查询对象
 * 设置观察的实体类型为FRTSFormationAgent结构体
 * 设置操作类型为移除操作
 */
URTSRemoveHashPosition::URTSRemoveHashPosition()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

/**
 * 配置查询所需的实体片段和子系统要求
 * @param EntityManager 实体管理器引用，用于注册和配置查询
 */
void URTSRemoveHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加只读的单元格位置片段需求
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadOnly);
	// 添加读写权限的RTS代理子系统需求
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	// 将查询注册到当前处理器
	EntityQuery.RegisterWithProcessor(*this);
}

/**
 * 执行移除哈希位置的操作
 * 遍历所有符合条件的实体块，从代理子系统的哈希网格中移除实体位置信息
 * @param EntityManager 实体管理器引用
 * @param Context 执行上下文，包含当前处理的实体信息
 */
void URTSRemoveHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历所有实体块
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取单元格位置片段视图
			auto CellLocFragments = Context.GetFragmentView<FRTSCellLocFragment>();
			// 获取可变的代理子系统引用
			auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();

			// 遍历当前块中的所有实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// 获取当前RTS代理的单元格位置信息
				const auto& RTSAgent = CellLocFragments[EntityIndex];

				// 从哈希网格中移除该实体及其位置信息
				AgentSubsystem.AgentHashGrid.Remove(Context.GetEntity(EntityIndex), RTSAgent.CellLoc);
			}
		});
}


