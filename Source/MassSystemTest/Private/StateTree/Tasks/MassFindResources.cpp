// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/Tasks/MassFindResources.h"
#include "MassCommonFragments.h"
#include "MassSmartObjectHandler.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#include "ResourcesEntity.h"

#include "SmartObjectSubsystem.h"

bool FMassFindResource::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(ResourceUserHandle);
	return true;
}

EStateTreeRunStatus FMassFindResource::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
/**
 * 执行状态树节点逻辑，用于寻找建造房屋所需的资源智能对象
 * 
 * @param Context 状态树执行上下文，提供数据访问和执行环境
 * @return EStateTreeRunStatus 返回运行状态：
 *         - Running: 成功发起异步查找请求，节点正在运行中
 *         - Failed: 不需要任何资源（已拥有所有必需资源），节点执行失败
 */
   FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
   const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
   USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
   UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
   FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
   FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);

  // 创建智能对象处理器，用于处理大规模实体的智能对象交互
  const FMassSmartObjectHandler MassSmartObjectHandler(
    MassContext.GetMassEntityExecutionContext(),
    SmartObjectSubsystem,
    SignalSubsystem);

  // 构建房屋所需的资源标签容器
  FGameplayTagContainer ResourcesToBuildHouse;
  ResourcesToBuildHouse.AddTag(InstanceData.RockResourceTag);
  ResourcesToBuildHouse.AddTag(InstanceData.WoodResourceTag);

  // 确定当前最需要的资源类型（优先获取岩石，然后是木材）
  FGameplayTag NeedTag = FGameplayTag();
  NeedTag = ResourceUserFragment.Tags.HasTag(InstanceData.RockResourceTag) ? NeedTag : InstanceData.RockResourceTag;
  NeedTag = ResourceUserFragment.Tags.HasTag(InstanceData.WoodResourceTag) ? NeedTag : InstanceData.WoodResourceTag;

  // 如果已经拥有所有必需资源，则不需要继续执行
  if (!NeedTag.IsValid()) { return EStateTreeRunStatus::Failed; }

  // 构造查询条件并异步查找符合条件的智能对象候选者
  FGameplayTagQuery Query = FGameplayTagQuery::MakeQuery_MatchTag(NeedTag);
  InstanceData.RequestID = MassSmartObjectHandler.FindCandidatesAsync(MassContext.GetEntity(), FGameplayTagContainer(), Query, TransformFragment.GetTransform().GetLocation());
  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassFindResource::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	/**
 * 处理智能对象请求并更新实例数据状态
 * 
 * 该函数主要完成以下功能：
 * 1. 获取执行上下文和相关子系统引用
 * 2. 查询智能对象候选槽位
 * 3. 更新实例数据中的槽位信息
 * 4. 清理请求并发送完成信号
 * 
 * @param Context 状态树执行上下文，提供数据访问和执行环境
 * @return EStateTreeRunStatus::Running 返回运行状态，表示任务仍在执行中
 */
{
	// 获取实例数据引用和各子系统/片段的引用
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	
	// 创建智能对象处理器实例
	const FMassSmartObjectHandler MassSmartObjectHandler(
		MassContext.GetMassEntityExecutionContext(),
		SmartObjectSubsystem,
		SignalSubsystem);

	// 检查是否存在指定请求ID的候选槽位，如果存在则更新实例数据
	if (auto CandidateSlots = MassSmartObjectHandler.GetRequestCandidates(InstanceData.RequestID))
	{
		// 保存找到的候选槽位并标记是否找到智能对象
		InstanceData.FoundSlots = *CandidateSlots;
		InstanceData.bFoundSmartObject = InstanceData.FoundSlots.NumSlots > 0;

		// 清理已完成的请求并重置请求ID
		MassSmartObjectHandler.RemoveRequest(InstanceData.RequestID);
		InstanceData.RequestID.Reset();

		// 向实体发送LookAt完成信号
		SignalSubsystem.SignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity());
	}
	
	// 返回运行状态，表示该任务仍在执行过程中
	return EStateTreeRunStatus::Running;
}
}

void FMassFindResource::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	/**
     * 退出当前状态的处理函数
     * 
     * 该函数负责清理当前状态的相关资源，包括移除智能对象请求、重置请求ID，
     * 并调用基类的退出状态处理逻辑
     * 
     * @param Context 状态树执行上下文，包含实例数据和各种子系统引用
     * @param Transition 状态转换信息，描述如何从当前状态退出
    */
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	
	// 创建智能对象处理器，用于管理智能对象相关的请求操作
	const FMassSmartObjectHandler MassSmartObjectHandler(
		MassContext.GetMassEntityExecutionContext(),
		SmartObjectSubsystem,
		SignalSubsystem);

	// 如果存在有效的请求ID，则移除对应的智能对象请求并重置请求ID
	if (InstanceData.RequestID.IsSet())
	{
		MassSmartObjectHandler.RemoveRequest(InstanceData.RequestID);
		InstanceData.RequestID.Reset();
	}
	
	// 调用基类的退出状态处理逻辑，完成状态转换
	FMassStateTreeTaskBase::ExitState(Context, Transition);
}

void FMassFindResource::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const
{
	/**
    * @brief 重置实例数据并标记状态树任务完成
    * 
    * @param Context 状态树执行上下文，包含任务执行所需的状态和数据
    * @param CompletionStatus 任务完成状态，指示任务是成功、失败还是其他状态
    * @param CompletedActiveStates 已完成的活动状态集合，用于状态跟踪和管理
    * 
    * 该函数首先获取当前实例的数据引用，然后重置请求ID并清除智能对象查找标志，
    * 最后调用基类方法标记状态树任务完成。
    */
   FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
   // 重置实例数据中的请求标识和智能对象查找状态
   InstanceData.RequestID.Reset();
   InstanceData.bFoundSmartObject = false;

   // 调用基类方法标记状态树任务完成
   FMassStateTreeTaskBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}
