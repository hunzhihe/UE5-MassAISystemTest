// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomizePositionProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassPersistentDataSubsystem.h"

/**
 * @brief URandomizePositionProcessor类构造函数
 * 
 * 初始化EntityQuery成员变量，将其绑定到当前实例
 */
URandomizePositionProcessor::URandomizePositionProcessor()
	: EntityQuery(*this)
{
}

/**
 * @brief 配置实体查询所需的片段要求
 * 
 * @param EntityManager 实体管理器的共享引用，用于管理实体和片段
 * 
 * 该函数设置EntityQuery需要访问FTransformFragment片段，并指定为读写权限
 */
void URandomizePositionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 处理实体信号，随机化实体位置
 * 
 * @param EntityManager 实体管理器引用，用于管理实体
 * @param Context 执行上下文引用，提供执行环境信息
 * @param EntitySignals 实体信号查找引用，用于处理信号相关操作
 * 
 * 该函数遍历所有实体块，为每个实体设置随机位置坐标
 * X和Y坐标在-2000到2000范围内随机生成，Z坐标固定为0
 */
void URandomizePositionProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取可变的变换片段视图
			const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();

			// 遍历当前块中的所有实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& TransformFragment = TransformFragments[EntityIdx];
				// 为实体设置随机位置，X和Y坐标在-2000到2000之间，Z坐标为0
				TransformFragment.GetMutableTransform().SetLocation(FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), 0.f));
			}
		});
}

/**
 * @brief 初始化处理器内部状态
 * 
 * @param Owner 拥有此处理器的UObject对象引用
 * @param EntityManager 实体管理器的共享引用
 * 
 * 该函数订阅随机化位置信号，并调用父类初始化函数
 */
void URandomizePositionProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::RandomizePositions);
	Super::InitializeInternal(Owner, EntityManager);
}