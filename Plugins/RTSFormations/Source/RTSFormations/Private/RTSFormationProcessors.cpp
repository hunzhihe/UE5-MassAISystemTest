// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationProcessors.h"

#include "RTSFormationSubsystem.h"
#include "LaunchEntityProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "RTSAgentTraits.h"
#include "RTSSignals.h"
#include "Engine/World.h"
#include "Unit/UnitFragments.h"


//----------------------------------------------------------------------//
//  URTSFormationInitializer
//----------------------------------------------------------------------//

/**
 * @brief 构造函数，初始化实体查询配置
 * 
 * 初始化观察类型为FRTSFormationAgent结构体，并设置操作为添加模式
 */
URTSFormationInitializer::URTSFormationInitializer()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

/**
 * @brief 配置实体查询所需的组件要求
 * 
 * @param EntityManager 实体管理器的共享引用，用于管理游戏中的实体
 * 
 * 该函数设置查询需要的三种组件要求：
 * - FRTSFormationAgent片段（无访问权限）
 * - FUnitFragment共享片段（只读权限）
 * - URTSFormationSubsystem子系统（读写权限）
 */
void URTSFormationInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::None);
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 执行初始化逻辑，处理实体并更新单位位置信息
 * 
 * @param EntityManager 实体管理器引用，用于管理游戏实体
 * @param Context 执行上下文引用，提供执行环境信息
 * 
 * 函数主要功能：
 * 1. 收集所有单位的句柄信息
 * 2. 调用阵型子系统更新每个单位的位置
 */
void URTSFormationInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FUnitHandle> UnitHandles;

	// 遍历所有实体块，收集单位句柄信息
	EntityQuery.ForEachEntityChunk(Context, [&UnitHandles](FMassExecutionContext& Context)
		{
			auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();

			UnitHandles.Emplace(UnitFragment.UnitHandle);
		});

	// 获取阵型子系统并更新所有单位的位置信息
	auto FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(EntityManager.GetWorld());
	for (const FUnitHandle& UnitHandle : UnitHandles)
	{
		FormationSubsystem->UpdateUnitPosition(UnitHandle);
	}
}



/**
 * @brief 构造函数，初始化URTSFormationDestroyer对象
 * 
 * 初始化EntityQuery，并设置观察类型为FRTSFormationAgent结构体，
 * 操作类型设置为移除操作。
 */
URTSFormationDestroyer::URTSFormationDestroyer()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

/**
 * @brief 配置实体查询所需的片段和子系统要求
 * 
 * @param EntityManager 实体管理器的共享引用，用于配置查询条件
 * 
 * 添加FRTSFormationAgent片段、FUnitFragment共享片段和URTSFormationSubsystem子系统的访问要求
 */
void URTSFormationDestroyer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::None);
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 执行销毁formation的逻辑处理
 * 
 * @param EntityManager 实体管理器引用
 * @param Context 执行上下文引用
 * 
 * 遍历符合条件的实体块，收集需要更新的单位句柄，
 * 最后通知formation子系统更新相关单位的位置信息。
 */
void URTSFormationDestroyer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();
	TArray<FUnitHandle> UnitSignals;

	// 遍历所有符合条件的实体块，收集单位句柄
	EntityQuery.ForEachEntityChunk(Context, [&UnitSignals](FMassExecutionContext& Context)
		{
			auto UnitFragment = Context.GetSharedFragment<FUnitFragment>();

			// 收集受影响的单位/实体句柄，避免重复添加
			UnitSignals.AddUnique(UnitFragment.UnitHandle);
		});

	// 通知受影响的单位/实体进行位置更新
	for (const auto& Unit : UnitSignals)
	{
		// 只有当中心点发生变化时才需要通知单位中的每个实体
		// 其他情况下只需要通知替换被销毁实体的那个实体
		FormationSubsystem.UpdateUnitPosition(Unit);
	}
}



//----------------------------------------------------------------------//
//  URTSAgentMovement::ConfigureQueries
//
//  配置用于查询实体的条件集合。该函数定义了哪些 Fragment 和 Shared Fragment 是必需的、可选的，以及它们的访问权限，
//  并将这些查询注册到当前处理器中以供后续执行使用。
//
//  参数:
//      EntityManager - 实体管理器引用，用于与 Mass 框架交互（虽然本函数未直接使用）
//
//----------------------------------------------------------------------//
void URTSAgentMovement::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加 LaunchEntityFragment 的要求：不需要访问也不需要存在（占位用途）
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::None, EMassFragmentPresence::None);

	// 要求具有只读访问权限的 RTSFormationAgent Fragment
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);

	// 要求具有读写访问权限的移动目标 Fragment
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);

	// 要求具有只读访问权限的变换 Fragment
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	// 添加必须存在的常量共享 Fragment：移动参数
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);

	// 添加 RTSFormationSettings 常量共享 Fragment
	EntityQuery.AddConstSharedRequirement<FRTSFormationSettings>();

	// 添加 UnitFragment 共享 Fragment，并设置为只读
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);

	// 可选地添加模拟变量时间片 Chunk Fragment，并设置为只读
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);

	// 设置 Chunk 过滤器，仅处理应该在当前帧更新的 Chunk
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);

	// 将此查询注册给当前 Processor 使用
	EntityQuery.RegisterWithProcessor(*this);
}

//----------------------------------------------------------------------//
//  URTSAgentMovement::Execute
//
//  执行主逻辑，在每个 Tick 中遍历所有匹配查询条件的实体 Chunk，计算并更新其移动目标信息，包括位置偏移、方向和速度控制等行为逻辑.
//
//  参数:
//      EntityManager - 实体管理器，提供对实体数据的操作接口
//      Context       - 当前执行上下文，包含当前批次的数据视图及运行时状态
//
//----------------------------------------------------------------------//
void URTSAgentMovement::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历所有符合条件的实体 Chunk
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
		{
			// 获取可变的移动目标片段数组视图（允许修改）
			TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

			// 获取只读的变换片段数组视图
			TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

			// 获取只读的 RTS Formation Agent 片段数组视图
			TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();

			// 获取常量共享片段中的 Formation Settings 数据
			const FRTSFormationSettings& FormationSettings = Context.GetConstSharedFragment<FRTSFormationSettings>();

			// 获取常量共享片段中的 Movement Parameters 数据
			const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

			// 获取共享片段中的单位信息
			auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();

			// 对当前 Chunk 中的所有实体进行迭代处理
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// 引用当前实体的移动目标片段
				FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];

				// 获取当前实体的世界变换信息
				const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

				// 获取当前实体对应的 Formation Agent 信息
				const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];

				// 根据 Formation Agent 的偏移量和单位朝向旋转得到最终的目标点偏移
				auto Offset = RTSFormationAgent.Offset;
				Offset = Offset.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector3f(0.f, 0.f, 1.f));

				// 计算并设定新的移动中心点（基于插值目的地加上偏移）
				MoveTarget.Center = FVector(UnitFragment.InterpDestination + Offset);

				// 更新距离目标的距离和前进方向
				auto DiffToGoal = MoveTarget.Center - Transform.GetLocation();
				MoveTarget.DistanceToGoal = DiffToGoal.Length();
				MoveTarget.Forward = DiffToGoal.GetSafeNormal();

				// 如果已经足够接近目标点，则切换至步行模式并调整期望速度
				if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
				{
					// TODO: 若启用 Stand 动作需取消注释以下行
					// MoveTarget.CreateNewAction(EMassMovementAction::Stand, *GetWorld());

					// 设置行走速度，依据 FormationSettings 中 WalkMovement 参数生成具体数值
					MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.WalkMovement, Context.GetEntity(EntityIndex).Index));
				}
			}
		});
}

//----------------------------------------------------------------------//
//  URTSFormationUpdate
//
//  描述：该类用于处理RTS单位编队更新逻辑。它订阅了编队更新信号，并在接收到信号时计算每个实体的目标移动位置，以维持编队结构。
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
//  InitializeInternal
//
//  描述：初始化内部状态，在拥有者对象上下文中注册对“编队更新”信号的监听。
//
//  参数：
//      Owner           - 拥有此系统的 UObject 对象，通常是一个 World 或 Actor 实例
//      EntityManager   - 共享引用到 FMassEntityManager，用于管理 Mass Entities 的生命周期与数据访问
//----------------------------------------------------------------------//
void URTSFormationUpdate::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, RTS::Unit::Signals::FormationUpdated);
}

//----------------------------------------------------------------------//
//  ConfigureQueries
//
//  描述：配置查询规则，定义哪些 Fragment 和 Shared Fragment 是必需的以便后续遍历实体使用。这些要求包括只读或读写的访问权限设置等信息.
//
//  参数:
//      EntityManager   - 共享引用到 FMassEntityManager，用于构建和注册 Entity 查询条件
//----------------------------------------------------------------------//
void URTSFormationUpdate::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FRTSFormationSettings>();
}

//----------------------------------------------------------------------//
//  SignalEntities
//
//  描述：响应编队更新信号，针对所有匹配查询条件的实体块进行迭代处理。根据当前实体的位置、编队设定以及运动参数来重新计算其目标移动点并更新相关片段数据.
//
//  参数:
//      EntityManager    - 引用至 FMassEntityManager，提供实体管理系统接口
//      Context          - 执行上下文，包含当前批次执行所需的信息如世界指针、实体列表等
//      EntitySignals    - 信号名称查找表，可用于调试或进一步扩展信号机制（本函数未直接使用）
//----------------------------------------------------------------------//
void URTSFormationUpdate::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// 遍历符合查询条件的所有实体块，逐个处理其中每一个实体
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
		{
			// 获取可变视图用于修改 MoveTarget 片段
			TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			// 获取只读 Transform 片段视图
			TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

			// 获取共享常量片段中的编队设置和移动参数
			const FRTSFormationSettings& FormationSettings = Context.GetConstSharedFragment<FRTSFormationSettings>();
			const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

			// 遍历当前 Chunk 中的所有实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
				const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

				// 创建新的移动动作，并基于当前位置与中心点方向设置基本属性
				MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
				MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
				MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
				MoveTarget.SlackRadius = 10.f;
				MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

				// 根据运行模式及实体索引生成期望速度并赋值给 DesiredSpeed 字段
				MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.RunMovement, Context.GetEntity(EntityIndex).Index));
			}
		});
}
