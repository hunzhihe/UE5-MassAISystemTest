// Fill out your copyright notice in the Description page of Project Settings.


#include "BHEnemyProcessor.h"

#include "BulletHellEnemyTrait.h"

#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"


/**
 * @brief UBHEnemyProcessor类构造函数
 * 
 * 初始化EntityQuery和UpdateHashGridQuery查询对象，将当前处理器实例作为参数传入
 */
UBHEnemyProcessor::UBHEnemyProcessor()
	: EntityQuery(*this),
	UpdateHashGridQuery(*this)
{
}

/**
 * @brief 配置实体查询条件
 * 
 * 设置两个主要查询(EntityQuery和UpdateHashGridQuery)所需的片段、子系统和标签要求，
 * 用于后续在Bullet Hell游戏系统中处理敌人实体
 * 
 * @param EntityManager 共享引用的实体管理器，提供实体数据访问和管理功能
 */
void UBHEnemyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 配置主实体查询条件：用于处理敌人的移动和行为逻辑
	// 要求实体具有移动目标片段(读写权限)、变换片段(只读权限)和BulletHell子系统(只读权限)
	// 必须包含敌人标签，可选择性包含模拟变量刻度块片段用于性能优化
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBHEnemyTag>(EMassFragmentPresence::All);

	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);

	// 配置更新哈希网格查询条件：用于维护敌人在空间中的位置信息
	// 要求实体具有敌人片段(读写权限)、变换片段(只读权限)和BulletHell子系统(读写权限)
	// 可选择性包含模拟变量刻度块片段用于性能优化
	UpdateHashGridQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateHashGridQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);

	UpdateHashGridQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	UpdateHashGridQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

/**
 * @brief 执行敌人的行为逻辑更新，包括移动目标的更新与哈希网格位置的更新。
 *
 * 此函数通过遍历实体块（Entity Chunk）的方式处理大量敌人实体的行为逻辑。主要完成以下两个任务：
 * 1. 更新每个敌人的移动目标，使其朝向玩家，并根据距离决定是否需要开始移动或停止。
 * 2. 更新敌人在哈希网格中的位置信息，用于后续的空间查询和碰撞检测优化。
 *
 * @param EntityManager 实体管理器引用，提供对所有实体及其组件的访问能力。
 * @param Context 当前执行上下文，包含当前批次处理的实体以及相关子系统等信息。
 */
void UBHEnemyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历实体块以更新敌人的移动目标
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			SCOPED_NAMED_EVENT(STAT_UpdateMoveTarget, FColor::Red);

			// 获取所需子系统及片段视图
			auto BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
			auto MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			auto TransformFragments = Context.GetFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();

			// 遍历当前块中每一个实体
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& MoveTargetFragment = MoveTargetFragments[EntityIdx];
				auto& TransformFragment = TransformFragments[EntityIdx];

				// 设置移动目标为中心点（即玩家位置）
				BulletHellSubsystem->GetPlayerLocation(MoveTargetFragment.Center);

				// 计算当前位置到目标的距离和方向
				auto EntityLocation = TransformFragment.GetTransform().GetLocation();
				MoveTargetFragment.DistanceToGoal = FVector::Dist(EntityLocation, MoveTargetFragment.Center);
				MoveTargetFragment.Forward = (MoveTargetFragment.Center - EntityLocation).GetSafeNormal();

				// 根据当前动作状态和距离切换动作：站立时若远离则改为移动；移动时接近则改为站立
				if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Stand && MoveTargetFragment.DistanceToGoal > 50.f)
				{
					MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
					MoveTargetFragment.IntentAtGoal = EMassMovementAction::Stand;
				}
				else if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move && MoveTargetFragment.DistanceToGoal <= 50.f)
				{
					MoveTargetFragment.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
				}
			}
		});

	// 遍历实体块以更新敌人在哈希网格中的位置
	UpdateHashGridQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			SCOPED_NAMED_EVENT(STAT_UpdateHashGrid, FColor::Red);

			// 获取可变子系统及片段视图
			auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
			auto BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
			auto TransformFragments = Context.GetFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();

			// 遍历当前块中每一个实体
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto TransformFragment = TransformFragments[EntityIdx];
				auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];

				// 获取实体的世界坐标并更新其在哈希网格中的格子位置
				auto Location = TransformFragment.GetTransform().GetLocation();
				BHEnemyFragment.CellLocation = BulletHellSubsystem->GetHashGrid_Mutable().Move(
					Context.GetEntity(EntityIdx),
					BHEnemyFragment.CellLocation,
					FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent)
				);
			}
		});
}



/**
 * 构造函数，初始化UBHEnemyInitializer对象
 * 
 * 初始化EntityQuery成员变量，并设置观察类型为FBHEnemyTag结构体，
 * 操作类型为添加操作
 */
UBHEnemyInitializer::UBHEnemyInitializer()
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

/**
 * 配置实体查询所需的组件要求
 * 
 * @param EntityManager 共享引用的实体管理器，用于管理实体和组件
 * 
 * 该函数为EntityQuery添加必要的组件访问要求：
 * - FTransformFragment：只读访问，用于获取实体变换信息
 * - FBHEnemyFragment：读写访问，用于处理敌人相关数据
 * - UBulletHellSubsystem：读写访问，用于访问子弹地狱子系统
 */
void UBHEnemyInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 敌人初始化执行函数，在每个实体块中初始化敌人的哈希网格位置信息
 * 
 * @param EntityManager 实体管理器引用，用于管理游戏中的实体
 * @param Context 执行上下文引用，提供当前执行环境的信息和操作接口
 */
void UBHEnemyInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 遍历所有实体块，为每个敌人实体初始化其在哈希网格中的位置信息
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取变换片段、敌人片段和子弹地狱子系统的视图
			auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			auto BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
			auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
			auto& HashGrid = BulletHellSubsystem->GetHashGrid_Mutable();

			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				// 获取当前实体的敌人片段和位置信息，并将其添加到哈希网格中
				auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];
				auto TransformFragment = TransformFragments[EntityIdx];
				auto Location = TransformFragment.GetTransform().GetLocation();

				BHEnemyFragment.CellLocation = HashGrid.Add(Context.GetEntity(EntityIdx), FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent));
			}
		});
}


/**
 * 构造函数，初始化UBHEnemyDestructor对象
 * 
 * 初始化EntityQuery，并设置观察类型为FBHEnemyFragment结构体，
 * 操作类型为移除操作
 */
UBHEnemyDestructor::UBHEnemyDestructor()
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

/**
 * 配置查询条件
 * 
 * @param EntityManager 实体管理器的共享引用，用于配置查询所需的片段和子系统要求
 * 
 * 添加对FBHEnemyFragment片段的只读需求和对UBulletHellSubsystem子系统的读写需求
 */
void UBHEnemyDestructor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * 执行销毁逻辑
 * 
 * @param EntityManager 实体管理器引用，用于管理实体
 * @param Context 执行上下文引用，包含当前执行环境的信息
 * 
 * 遍历所有实体块，从哈希网格中移除敌人的位置信息
 */
void UBHEnemyDestructor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取敌人类片段视图和子弹地狱子系统
			auto BHEnemyFragments = Context.GetFragmentView<FBHEnemyFragment>();
			auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
			auto& HashGrid = BulletHellSubsystem->GetHashGrid_Mutable();

			// 遍历当前块中的所有实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];

				// 从哈希网格中移除实体及其位置信息
				HashGrid.Remove(Context.GetEntity(EntityIdx), BHEnemyFragment.CellLocation);
			}
		});
}