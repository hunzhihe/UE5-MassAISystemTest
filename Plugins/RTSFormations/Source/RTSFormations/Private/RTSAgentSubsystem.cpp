// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentSubsystem.h"

#include "LaunchEntityProcessor.h"
#include "MassCommandBuffer.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

/**
 * 在指定位置和半径范围内启动实体
 * 
 * @param Location 启动实体的位置坐标
 * @param Radius 搜索实体的半径范围
 */
void URTSAgentSubsystem::LaunchEntities(const FVector& Location, float Radius) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("LaunchEntities"));

	// 获取世界中的实体子系统
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	// 查询半径范围内的实体
	TArray<FMassEntityHandle> Entities;
	const FBox Bounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
	AgentHashGrid.QuerySmall(Bounds, Entities);

	if (EntitySubsystem)
	{
		// 创建启动实体片段并设置参数
		FLaunchEntityFragment LaunchEntityFragment;
		LaunchEntityFragment.Origin = Location;
		LaunchEntityFragment.Magnitude = 500.f;

		// 为查询到的每个实体添加启动片段
		for (const FMassEntityHandle& Entity : Entities)
		{
			EntitySubsystem->GetEntityManager().Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity,
				LaunchEntityFragment);
		}
		
		// 发送延迟信号给所有受影响的实体（作为观察者机制的替代方案）
		if (Entities.Num())
			GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntities(LaunchEntity, Entities, 0.1f);
	}
}
