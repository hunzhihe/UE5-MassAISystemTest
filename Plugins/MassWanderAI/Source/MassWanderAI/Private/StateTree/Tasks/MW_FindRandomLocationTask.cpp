// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/Tasks/MW_FindRandomLocationTask.h"

#include "MassCommonFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMW_FindRandomLocationTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);

	return FMassStateTreeTaskBase::Link(Linker);
}

EStateTreeRunStatus FMW_FindRandomLocationTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	/**
 * 执行状态节点逻辑，用于查找随机位置
 * 
 * @param Context 状态树执行上下文，包含实例数据和外部数据的访问接口
 * @param Transition 状态转换信息，包含状态改变类型
 * @return EStateTreeRunStatus::Running 返回运行状态，表示节点仍在执行中
 * 
 * 该函数首先检查状态是否发生改变，如果没有改变则直接返回运行状态。
 * 如果状态发生改变，则计算一个随机偏移位置，并将其存储在实例数据的输出位置中。
 */
if (Transition.ChangeType != EStateTreeStateChangeType::Changed) { return EStateTreeRunStatus::Running; }
// 开始CPU性能分析器事件范围，用于性能监控

	TRACE_CPUPROFILER_EVENT_SCOPE(ST_FindRandomLocation)

	// 获取实例数据引用，包含范围参数
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float Range = InstanceData.Range;
	
	// 获取外部变换数据，用于计算当前位置
	FTransformFragment& Transform = Context.GetExternalData(TransformHandle);
	
	// 计算随机偏移位置，X和Y坐标在范围的一半内随机生成，Z坐标为0
	const auto NewOffset = FVector(FMath::RandRange(Range / 2 * -1, Range / 2), FMath::RandRange(Range / 2 * -1, Range / 2), 0.f);

	// 将当前位置与随机偏移相加，得到新的目标位置并存储到输出变量中
	InstanceData.OutLocation = Transform.GetTransform().GetLocation() + NewOffset;

	return EStateTreeRunStatus::Running;
}