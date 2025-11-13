// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/MassMoveToSOTask.h"

#include "MassCommonFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeLinker.h"

bool FMassMoveToSOTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	return Super::Link(Linker);
}

EStateTreeRunStatus FMassMoveToSOTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	/**
    * 执行移动到智能对象槽位的逻辑
    * 
    * 该函数通过获取实例数据、移动目标、变换信息和智能对象子系统等上下文数据，
    * 计算目标位置并创建相应的移动动作。主要用于控制实体移动到指定的智能对象槽位。
    *
    * @param Context 状态树执行上下文，包含所有必要的数据句柄和系统引用
    * @return EStateTreeRunStatus 返回运行状态：
    *         - Running: 成功创建移动动作，任务正在执行中
    *         - Failed: 无法获取有效的目标位置，任务执行失败
    */

   // 获取所需的上下文数据引用
   FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
   auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
   auto& TransformFragment = Context.GetExternalData(TransformHandle);
   auto& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);

   // 创建移动动作并设置目标位置参数
   const auto& ClaimHandle = InstanceData.ClaimHandle;
   auto Destination = SmartObjectSubsystem.GetSlotLocation(ClaimHandle);
   if (!Destination.IsSet()) { return EStateTreeRunStatus::Failed; }

   MoveTarget.Center = Destination.GetValue();
   MoveTarget.SlackRadius = 50.f;
   MoveTarget.Forward = (Destination.GetValue() - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();
   MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, TransformFragment.GetTransform().GetLocation());
   MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
   MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

   return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassMoveToSOTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	/**
    * 检查是否到达目标位置
    * 
    * 该函数用于判断当前实体是否已经到达指定的目标位置。通过比较当前位置与目标位置的距离，
    * 判断是否在允许的误差范围内。如果已到达目标，则创建新的动作并返回成功状态；否则返回运行中状态。
    * 
    * @param Context 状态树执行上下文，用于获取外部数据和世界信息
    * @return EStateTreeRunStatus 状态树运行状态：
    *         - Succeeded: 已到达目标位置
    *         - Running: 仍在前往目标位置的途中
    */
    // check if goal reached
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	auto& TransformFragment = Context.GetExternalData(TransformHandle);

	TRACE_CPUPROFILER_EVENT_SCOPE(ST_FindRandomLocation)
	// 计算到目标点的距离和方向向量
	MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, TransformFragment.GetTransform().GetLocation());
	MoveTarget.Forward = (MoveTarget.Center - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();

	// 检查是否已到达目标点的允许误差范围内
	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(MoveTarget.IntentAtGoal, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
