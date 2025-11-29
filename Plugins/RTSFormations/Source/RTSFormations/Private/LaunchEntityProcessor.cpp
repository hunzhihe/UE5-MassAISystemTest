// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchEntityProcessor.h"

#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "TimerManager.h"
#include "Engine/World.h"


//----------------------------------------------------------------------//
//  ULaunchEntityProcessor
//
//  概述:
//    此类用于处理与发射实体相关的逻辑。它配置查询以筛选具有特定片段的实体，并在初始化时订阅信号系统，以便响应发射事件。当接收到信号时，会对符合条件的实体执行移动目标设置操作，通常用于模拟发射行为（如导弹、投射物等）的初始运动方向和位置计算。
//----------------------------------------------------------------------//

/**
 * @brief 配置实体查询条件，指定需要访问哪些片段以及它们的访问权限
 *
 * 添加以下要求到 EntityQuery：
 * - FLaunchEntityFragment：读写权限，用于获取发射相关数据
 * - FMassMoveTargetFragment：读写权限，用于设定移动目标
 * - FTransformFragment：只读权限，用于获取当前变换信息
 * - 不允许存在 FInitLaunchFragment 标签的实体参与处理
 *
 * @param EntityManager 引擎提供的实体管理器引用，用于构建查询
 */
void ULaunchEntityProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FInitLaunchFragment>(EMassFragmentPresence::None);
}

/**
 * @brief 初始化处理器内部状态，包括获取子系统并注册信号回调
 *
 * 调用父类初始化方法后，从世界中获取 Mass 信号子系统和 RTS Formation 子系统实例，
 * 并将 LaunchEntity 方法绑定为信号回调函数，使其能够响应外部触发的发射信号
 *
 * @param Owner 处理器的所有者 UObject 对象
 * @param EntityManager 实体管理器的共享引用
 */
void ULaunchEntityProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, LaunchEntity);
}

/**
 * @brief 响应发射信号，批量更新实体的移动目标片段
 *
 * 使用 EntityQuery 并行遍历所有匹配的实体块，在每个线程上下文中：
 * - 获取发射实体片段、移动目标片段和变换片段的数据视图；
 * - 遍历该块中的每一个实体，根据发射源点和当前位置计算新的移动目标；
 * - 设置移动动作类型为 Move，并更新目标中心点、前进方向及距离；
 * - 最终给实体添加 FInitLaunchFragment 标签标记其已被初始化发射状态
 *
 * @param EntityManager 实体管理器引用
 * @param Context 执行上下文，提供对当前批次实体的操作接口
 * @param EntitySignals 实体信号名称查找表（未在此函数中使用）
 */
void ULaunchEntityProcessor::SignalEntities(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals)
{
	// 并行遍历所有满足查询条件的实体块
	EntityQuery.ParallelForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取各片段数组视图
			TArrayView<FLaunchEntityFragment> LaunchEntityFragments = Context.GetMutableFragmentView<FLaunchEntityFragment>();
			TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

			// 遍历当前块内的所有实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				const FLaunchEntityFragment& LaunchEntityFragment = LaunchEntityFragments[EntityIndex];
				FMassMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIndex];
				const FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

				// 创建一个新的移动动作
				MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *GetWorld());

				// 计算目标点：基于发射原点与当前位置的方向向量进行延伸
				MoveTargetFragment.Center = TransformFragment.GetTransform().GetLocation() +
					(TransformFragment.GetTransform().GetTranslation() - LaunchEntityFragment.Origin).GetSafeNormal() * LaunchEntityFragment.Magnitude;

				// 将 Z 轴设为零以保持水平面移动
				MoveTargetFragment.Center.Z = 0.f;

				// 设置朝向为目标方向
				MoveTargetFragment.Forward = (TransformFragment.GetTransform().GetTranslation() - LaunchEntityFragment.Origin).GetSafeNormal();

				// 计算到目标的距离
				MoveTargetFragment.DistanceToGoal = (TransformFragment.GetTransform().GetTranslation() - LaunchEntityFragment.Origin).Length();

				// 给实体打上已发射标签，防止重复处理
				Context.Defer().AddTag<FInitLaunchFragment>(Context.GetEntity(EntityIndex));
			}
		});
}


//----------------------------------------------------------------------//
//  UMoveForceProcessor
//----------------------------------------------------------------------//

/**
 * 配置查询条件，用于筛选需要处理的实体集合
 *
 * @param EntityManager 实体管理器引用，用于访问和操作实体数据
 */
void UMoveForceProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加对FLaunchEntityFragment片段的只读需求
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::ReadOnly);
	
	// 添加对FMassForceFragment片段的读写需求
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	
	// 添加对FTransformFragment片段的读写需求
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	// 添加对FMassMoveTargetFragment片段的读写需求
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	
	// 要求实体必须拥有FInitLaunchFragment标签
	EntityQuery.AddTagRequirement<FInitLaunchFragment>(EMassFragmentPresence::All);
}

/**
 * 执行处理器逻辑，遍历符合条件的实体并更新其状态或执行相关操作
 *
 * @param EntityManager 实体管理器，提供对实体数据的访问接口
 * @param Context 执行上下文，包含当前批次处理的相关信息
 */
void UMoveForceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 并行遍历所有匹配的实体块（chunk）
	EntityQuery.ParallelForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取各个片段的数据视图
			TConstArrayView<FLaunchEntityFragment> LaunchEntityFragments = Context.GetFragmentView<FLaunchEntityFragment>();
			TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
			TArrayView<FMassForceFragment> ForceFragments = Context.GetMutableFragmentView<FMassForceFragment>();
			TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();

			// 遍历当前块中的每一个实体
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// 获取各片段实例的引用
				const FLaunchEntityFragment& LaunchEntityFragment = LaunchEntityFragments[EntityIndex];
				FMassMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIndex];
				FMassForceFragment& ForceFragment = ForceFragments[EntityIndex];
				FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

				// 计算当前位置到目标中心的距离
				MoveTargetFragment.DistanceToGoal = (TransformFragment.GetTransform().GetTranslation() - MoveTargetFragment.Center).Length();

				// 判断是否接近目标点（距离小于50单位）
				if (MoveTargetFragment.DistanceToGoal < 50.f)
				{
					// 当前处于移动动作时进行销毁处理
					if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move)
					{
						// 延迟销毁该实体，并切换动作为站立状态
						Context.Defer().DestroyEntity(Context.GetEntity(EntityIndex));
						MoveTargetFragment.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
					}
				}
				else
				{
					// 在世界中绘制红色球形调试可视化
					DrawDebugSphere(GetWorld(), TransformFragment.GetTransform().GetLocation(), 40.f, 5, FColor::Red);
				}
			}
		});
}
