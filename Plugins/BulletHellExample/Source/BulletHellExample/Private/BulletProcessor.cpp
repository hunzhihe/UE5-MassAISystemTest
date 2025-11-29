// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletProcessor.h"

#include "BulletTrait.h"
#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"

/**
 * @brief 构造函数，初始化 EntityQuery 并绑定到当前对象。
 */
UBulletInitializerProcessor::UBulletInitializerProcessor()
	: EntityQuery(*this)
{
}

/**
 * @brief 配置查询条件，指定处理实体所需的标签和片段要求。
 *
 * @param EntityManager 实体管理器引用，用于配置查询。
 */
void UBulletInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加必须拥有 FBulletTag 标签的实体要求
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
	
	// 添加对速度、子弹数据和变换片段的读写或只读访问权限要求
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBulletFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);

	// 添加对信号子系统的读写权限需求
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 初始化处理器内部状态，并订阅相关信号。
 *
 * @param Owner 处理器的所有者 UObject 对象。
 * @param EntityManager 实体管理器引用。
 */
void UBulletInitializerProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 调用父类初始化逻辑
	Super::InitializeInternal(Owner, EntityManager);

	// 获取世界中的 Mass 信号子系统并订阅子弹生成信号
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, BulletHell::Signals::BulletSpawned);
}

/**
 * @brief 响应信号事件，遍历符合条件的实体块并对每个实体进行初始化操作。
 *
 * @param EntityManager 实体管理器。
 * @param Context 执行上下文，提供执行环境信息。
 * @param EntitySignals 实体信号名称查找表。
 */
void UBulletInitializerProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals)
{
	// 遍历所有匹配查询条件的实体块
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取可变的信号子系统实例
			auto SignalSubsystem = Context.GetMutableSubsystem<UMassSignalSubsystem>();

			// 获取子弹相关的片段视图（只读/可写）
			auto BulletFragments = Context.GetFragmentView<FBulletFragment>();
			auto VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
			auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();

			// 遍历当前块中每一个实体
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				// 引用各个片段的数据
				auto& BulletFragment = BulletFragments[EntityIdx];
				auto& VelocityFragment = VelocityFragments[EntityIdx];
				auto& TransformFragment = TransformFragments[EntityIdx];

				// 设置实体的速度：方向标准化后乘以速度值
				VelocityFragment.Value = BulletFragment.Direction.GetSafeNormal() * BulletFragment.Speed;

				// 设置实体的位置为出生点位置
				TransformFragment.GetMutableTransform().SetLocation(BulletFragment.SpawnLocation);

				// 延迟发送销毁信号，在生命周期结束后触发
				SignalSubsystem->DelaySignalEntityDeferred(
					Context,
					BulletHell::Signals::BulletDestroy,
					Context.GetEntity(EntityIdx),
					BulletFragment.Lifetime
				);
			}
		});
}



/**
 * 配置实体查询条件
 * @param EntityManager 实体管理器引用，用于管理游戏中的实体
 */
void UBulletDestroyerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加标签要求，只处理带有FBulletTag标签的实体
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
}

/**
 * 初始化处理器内部状态
 * @param Owner 处理器的所有者对象
 * @param EntityManager 实体管理器引用，用于管理游戏中的实体
 */
void UBulletDestroyerProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 调用父类初始化方法
	Super::InitializeInternal(Owner, EntityManager);

	// 获取世界中的信号子系统并订阅子弹销毁信号
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, BulletHell::Signals::BulletDestroy);
}

/**
 * 处理接收到信号的实体，销毁所有符合条件的实体
 * @param EntityManager 实体管理器引用，用于管理游戏中的实体
 * @param Context 执行上下文，包含当前处理的实体信息
 * @param EntitySignals 实体信号名称查找器，用于识别不同的信号类型
 */
void UBulletDestroyerProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals)
{
	// 遍历所有符合条件的实体块，并销毁其中的实体
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				Context.Defer().DestroyEntity(Context.GetEntity(EntityIdx));
			}
		});
}

/**
 * 构造函数，初始化实体查询对象
 */
UBulletDestroyerProcessor::UBulletDestroyerProcessor()
	:EntityQuery(*this)
{
}




/**
 * @brief UBulletCollisionProcessor 类构造函数
 * 
 * 初始化 EntityQuery 成员变量，并将其绑定到当前实例。
 */
UBulletCollisionProcessor::UBulletCollisionProcessor()
	: EntityQuery(*this)
{
}

/**
 * @brief 配置查询条件，指定该处理器需要处理的实体类型和数据需求
 * 
 * @param EntityManager 共享引用的实体管理器，用于配置查询
 * 
 * 该函数设置 EntityQuery 查询所需的标签和片段要求：
 * - 要求实体具有 FBulletTag 标签
 * - 只读访问 FTransformFragment 片段
 * - 只读访问 UBulletHellSubsystem 子系统
 */
void UBulletCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
}

/**
 * @brief 执行碰撞检测逻辑，检查子弹是否与其它实体发生碰撞并销毁相关实体
 * 
 * @param EntityManager 实体管理器引用，用于获取实体数据
 * @param Context 执行上下文引用，提供执行环境信息
 * 
 * 函数流程如下：
 * 1. 遍历所有符合条件的实体块（Entity Chunk）
 * 2. 对于每个实体：
 *    a. 获取其位置信息
 *    b. 使用哈希网格快速筛选附近可能碰撞的实体
 *    c. 进一步精确计算距离以确认实际碰撞
 *    d. 若发现碰撞，则同时销毁子弹及被碰撞的实体
 */
void UBulletCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this, &EntityManager](FMassExecutionContext& Context)
		{
			auto BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
			auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			const int32 NumEntities = Context.GetNumEntities();
			
			// 遍历当前块中的每一个实体
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& TransformFragment = TransformFragments[EntityIdx];
				auto Location = TransformFragment.GetTransform().GetLocation();

				// 使用哈希网格进行初步范围查询，找出附近的候选实体
				TArray<FMassEntityHandle> Entities;
				BulletHellSubsystem->GetHashGrid().Query(FBox::BuildAABB(Location, FVector(50.f)), Entities);

				// 对候选实体进一步做精确的距离判断，过滤出真正发生碰撞的实体
				Entities = Entities.FilterByPredicate([&Location, &EntityManager](const FMassEntityHandle& Entity)
					{
						auto EntityLocation = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
						return FVector::Dist(Location, EntityLocation) <= 50.f;
					});

				// 如果存在碰撞实体，则将当前子弹也加入待销毁列表一并删除
				if (Entities.Num() > 0)
				{
					Entities.Add(Context.GetEntity(EntityIdx));
					Context.Defer().DestroyEntities(Entities); // Delete bullet as well
				}
			}
		});
}
