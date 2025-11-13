// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/StateTreeMovementUpdateProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"

UStateTreeMovementUpdateProcessor::UStateTreeMovementUpdateProcessor() :
	EntityQuery(*this)
{
}

void UStateTreeMovementUpdateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassStateTreeInstanceFragment>(EMassFragmentAccess::None);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UStateTreeMovementUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
 /* 遍历实体块并处理移动目标逻辑 
 * @param Context 实体执行上下文，提供访问实体数据和子系统的接口
 * @param Lambda 回调函数，用于处理每个实体块中的实体
 * 
 * 此函数遍历所有实体块，检查具有移动目标的实体是否到达目标位置，
 * 如果实体距离目标位置小于100单位，则触发状态树激活信号
 */
EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取信号子系统引用，用于发送实体信号
			auto& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();

			// 获取当前块中所有实体的移动目标片段和变换片段视图
			const auto MassMoveTargetFragments = Context.GetFragmentView<FMassMoveTargetFragment>();
			const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();

			// 遍历当前块中的所有实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				const auto& MassMoveTargetFragment = MassMoveTargetFragments[EntityIdx];
				const auto& TransformFragment = TransformFragments[EntityIdx];

				// 检查实体当前是否处于移动状态
				if (MassMoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move)
				{
					// 计算实体当前位置与目标位置的2D距离
					auto Distance = FVector::Dist2D(TransformFragment.GetTransform().GetLocation(), MassMoveTargetFragment.Center);
					if (Distance < 100.f)
					{
						// 当距离小于100单位时，触发状态树激活信号
						SignalSubsystem.SignalEntityDeferred(Context, UE::Mass::Signals::StateTreeActivate, Context.GetEntity(EntityIdx));
					}
				}
			}
		});
}
