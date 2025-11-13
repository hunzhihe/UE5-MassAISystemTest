// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartObjects/BenchSOMassBehavior.h"
#include "MassSmartObjectAI.h"


/**
 * @brief 激活基准测试智能对象行为
 * 
 * 当实体与智能对象交互时执行基准测试行为逻辑。
 * 此函数在实体激活智能对象行为时被调用。
 * 
 * @param CommandBuffer 命令缓冲区，用于存储和执行批量操作命令
 * @param EntityContext 实体行为上下文，包含实体视图和相关行为信息
 */
DEFINE_LOG_CATEGORY(LogMassSmartObjectAI);

void UBenchSOMassBehavior::Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	// 执行智能对象交互时的基准测试逻辑
	// 记录日志信息，显示在指定实体上运行了基准测试行为
	UE_LOG(LogMassSmartObjectAI, Verbose, TEXT("Ran Bench behavior on %llu"), EntityContext.EntityView.GetEntity().AsNumber());
}
