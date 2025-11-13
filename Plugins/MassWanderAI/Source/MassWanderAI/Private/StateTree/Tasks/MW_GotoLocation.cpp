// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/Tasks/MW_GotoLocation.h"

#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMW_GotoLocation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return FMassStateTreeTaskBase::Link(Linker);
}

EStateTreeRunStatus FMW_GotoLocation::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	/**
 * 执行移动到目标位置的状态逻辑
 * 
 * 该函数从实例数据中获取目标位置，设置移动参数，并创建移动动作。
 * 函数返回Running状态表示仍在执行中。
 * 
 * @return EStateTreeRunStatus::Running 表示状态仍在运行中
 */
FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FVector& Destination = InstanceData.Destination;
	auto& TransformFragment = Context.GetExternalData(TransformHandle);

	// 创建移动动作并设置相关参数
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	MoveTarget.Center = Destination;
	MoveTarget.SlackRadius = 50.f;
	MoveTarget.Forward = (Destination - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();
	MoveTarget.DistanceToGoal = FVector::Dist(Destination, TransformFragment.GetTransform().GetLocation());
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMW_GotoLocation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	/**
 * @brief 执行移动片段的逻辑更新
 * 
 * 该函数负责更新智能体的位置信息，计算到目标点的距离，并根据距离判断是否到达目标。
 * 如果到达目标范围内则创建新的动作并返回成功状态，否则继续运行。
 * 
 * @param Context 状态树执行上下文，提供数据访问和世界信息
 * @return EStateTreeRunStatus 返回运行状态：
 *         - Succeeded: 当智能体到达目标点的松弛半径内时
 *         - Running: 当智能体仍在向目标移动时
 */

    const auto& TransformFragment = Context.GetExternalData(TransformHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 更新到目标的距离和智能体位置信息
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	InstanceData.AgentLocation = TransformFragment.GetTransform().GetLocation();
	MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, InstanceData.AgentLocation);
	MoveTarget.Forward = (MoveTarget.Center - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();

	// 检查是否已到达目标点的松弛半径范围内
	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(MoveTarget.IntentAtGoal, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FMW_GotoLocation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	// 获取移动目标的外部数据引用
// 通过MoveTargetHandle句柄从Context中获取MoveTarget对象的引用
// 返回值: MoveTarget对象的引用，用于后续操作
auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);

// 为移动目标创建新的站立动作
// 调用CreateNewAction方法，传入站立动作类型和世界上下文
// 参数1: EMassMovementAction::Stand - 指定动作为站立状态
// 参数2: *Context.GetWorld() - 获取当前世界环境的引用
MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
}