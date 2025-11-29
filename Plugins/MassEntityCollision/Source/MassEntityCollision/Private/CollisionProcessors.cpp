// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionProcessors.h"

#include "CollisionFragments.h"
#include "CollisionSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassMovementFragments.h"

//网格单元的碰撞范围半径，用于计算碰撞边界
static float HalfRange = 25.f;


/**
 * @brief ollision初始化处理器构造函数
 * 
 * 初始化实体查询对象，并设置观察的片段类型为FCollisionFragment，
 * 操作类型为添加操作。
 */
UCollisionInitializerProcessor::UCollisionInitializerProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

/**
 * @brief 配置查询需求
 * 
 * @param EntityManager 实体管理器引用，用于配置查询所需的片段和子系统
 * 
 * 为实体查询添加必要的片段访问需求：
 * - FCollisionFragment：读写访问权限
 * - FTransformFragment：只读访问权限  
 * - UCollisionSubsystem：读写访问权限
 */
void UCollisionInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 执行碰撞初始化处理逻辑
 * 
 * @param EntityManager 实体管理器引用
 * @param Context 执行上下文，包含当前处理的实体块信息
 * 
 * 遍历所有符合条件的实体块，为每个实体：
 * 1. 获取变换和碰撞片段数据
 * 2. 根据实体位置计算碰撞边界框
 * 3. 将实体添加到碰撞子系统的哈希网格中并更新片段的位置信息
 */
void UCollisionInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取碰撞子系统引用
			auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();

			// 获取当前块中的变换片段和碰撞片段视图
			const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			const auto HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();

			// 遍历块中的所有实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& HashGridFragment = HashGridFragments[EntityIdx];
				auto& TransformFragment = TransformFragments[EntityIdx];
				auto Location = TransformFragment.GetTransform().GetLocation();

				// 计算实体的碰撞边界框
				FBox Bounds = { Location - HalfRange, Location + HalfRange };
				
				// 将实体添加到哈希网格并更新片段中的位置信息
				HashGridFragment.CellLocation = HashGridSubsystem.HashGridData.Add(Context.GetEntity(EntityIdx), Bounds);
			}
		});
}


/**
 * @brief 构造函数，初始化碰撞销毁处理器
 * 
 * 设置观察的实体类型为FCollisionFragment结构体，并指定操作为移除操作。
 * 同时初始化EntityQuery成员变量。
 */
UCollisionDestroyProcessor::UCollisionDestroyProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

/**
 * @brief 配置查询条件
 * 
 * @param EntityManager 实体管理器的共享引用，用于管理实体和片段
 * 
 * 为EntityQuery添加FCollisionFragment只读需求和UCollisionSubsystem读写需求，
 * 确保在执行时能够访问到所需的片段和子系统。
 */
void UCollisionDestroyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 执行碰撞销毁处理逻辑
 * 
 * @param EntityManager 实体管理器引用，用于管理实体和片段
 * @param Context 执行上下文引用，提供执行环境和数据访问接口
 * 
 * 遍历所有符合条件的实体块，从哈希网格中移除实体的碰撞信息。
 * 主要功能是清理被销毁实体在碰撞子系统中的相关数据。
 */
void UCollisionDestroyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取碰撞子系统引用
			auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();

			// 获取当前块中所有碰撞片段的视图
			const auto HashGridFragments = Context.GetFragmentView<FCollisionFragment>();

			// 遍历当前块中的所有实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& HashGridFragment = HashGridFragments[EntityIdx];

				// 从哈希网格数据中移除实体及其位置信息
				HashGridSubsystem.HashGridData.Remove(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation);
			}
		});
}



/**
 * @brief 构造函数，初始化碰撞处理器。
 *
 * 设置执行顺序：在 Movement 组之前执行，并属于 Avoidance 处理组。
 */
UCollisionProcessor::UCollisionProcessor() :
	EntityQuery(*this),
	CollisionQuery(*this)
{
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Avoidance;
}

/**
 * @brief 配置查询条件以供后续处理使用。
 *
 * @param EntityManager 实体管理器引用，用于访问实体数据。
 */
void UCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 配置 EntityQuery 查询所需的片段和子系统要求
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);

	// 配置 CollisionQuery 查询所需的片段、标签及子系统要求
	CollisionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	CollisionQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	CollisionQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadOnly);
	CollisionQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	CollisionQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::None); // 仅用作过滤实体
	CollisionQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
}

/**
 * @brief 执行碰撞检测与响应逻辑。
 *
 * 包括更新哈希网格中的位置信息以及解决与其他实体之间的碰撞问题。
 *
 * @param EntityManager 实体管理器，提供对所有实体及其组件的访问。
 * @param Context 当前执行上下文，包含当前批次的数据。
 */
void UCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 更新每个实体在哈希网格中的位置
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UpdateCollisionHashGrid)

			auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();

			const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			const auto HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();

			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& HashGridFragment = HashGridFragments[EntityIdx];
				auto& TransformFragment = TransformFragments[EntityIdx];
				const auto& Location = TransformFragment.GetTransform().GetLocation();

				// 计算包围盒并移动到新的格子中
				FBox Bounds = { Location - HalfRange, Location + HalfRange };
				auto NewCellLocation = HashGridSubsystem.HashGridData.Move(
					Context.GetEntity(EntityIdx),
					HashGridFragment.CellLocation,
					Bounds
				);
				HashGridFragment.CellLocation = NewCellLocation;
			}
		});

	// 检测并处理与其他实体的碰撞
	CollisionQuery.ParallelForEachEntityChunk(Context, [this, &EntityManager](FMassExecutionContext& Context)
		{
			const auto& HashGridSubsystem = Context.GetSubsystemChecked<UCollisionSubsystem>();

			const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
			const auto RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
			const auto VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();

			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& TransformFragment = TransformFragments[EntityIdx];
				const auto& RadiusFragment = RadiusFragments[EntityIdx];
				auto& Velocity = VelocityFragments[EntityIdx];

				const auto Radius = RadiusFragment.Radius;
				auto& Transform = TransformFragment.GetMutableTransform();

				// 查询附近可能相交的实体
				FBox Bounds = {
					Transform.GetLocation() - HalfRange / 2,
					Transform.GetLocation() + HalfRange / 2
				};

				TArray<FMassEntityHandle> Entities;
				HashGridSubsystem.HashGridData.QuerySmall(Bounds, Entities);

				// 过滤掉自身
				Entities = Entities.FilterByPredicate([&Context, EntityIdx](const FMassEntityHandle& OtherEntity)
					{
						return OtherEntity != Context.GetEntity(EntityIdx);
					});

				// 解决碰撞并将速度投影到法线平面上
				FVector HitNormal = ResolveCollisions(Entities, EntityManager, Radius, Transform);
				Velocity.Value = FVector::VectorPlaneProject(Velocity.Value, HitNormal);
			}
		});
}

/**
 * @brief 根据给定的一组实体计算碰撞反应并向外推移重叠部分。
 *
 * @param Entities 可能发生碰撞的其他实体列表。
 * @param EntityManager 实体管理器，用于获取其他实体的位置等信息。
 * @param Radius 当前实体半径。
 * @param EntityTransform 当前实体变换信息（将被修改）。
 * @return 返回碰撞方向的单位法向量。
 */
FVector UCollisionProcessor::ResolveCollisions(
	const TArray<FMassEntityHandle>& Entities,
	FMassEntityManager& EntityManager,
	float Radius,
	FTransform& EntityTransform)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CalculateCollision)

	FVector HitNormal(FVector::ZeroVector);

	for (auto& Entity : Entities)
	{
		auto OtherEntityTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		auto OtherLocation = OtherEntityTransform->GetTransform().GetLocation();
		auto DistSq = FVector::DistSquared(EntityTransform.GetLocation(), OtherLocation);

		// 若距离小于两倍半径，则认为发生了重叠
		if (DistSq < FMath::Square(Radius * 2))
		{
			auto Direction = (EntityTransform.GetLocation() - OtherLocation).GetSafeNormal();
			Direction.Z = 0.f;

			auto Radii = Radius * 2;
			auto Depth = Radii - FMath::Sqrt(DistSq) + 0.01f;

			// 将当前位置沿分离方向偏移一半深度
			EntityTransform.SetLocation(EntityTransform.GetLocation() + Depth / 2 * Direction);
			HitNormal = Direction;
		}
	}

	return HitNormal;
}