// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGridSubsystem.h"

#include "HashGridFragments.h"
#include "MassEntityUtils.h"
#include "MassSignalSubsystem.h"

/**
 * 在指定区域中选择实体
 * 
 * @param SelectedLocation 选择区域的中心位置
 * @param Radius 选择区域的半径范围
 */
void UHashGridSubsystem::SelectEntitiesInArea(const FVector& SelectedLocation, float Radius)
{
	// 计算查询边界框
	FBox Bounds = { SelectedLocation - Radius, SelectedLocation + Radius };
	TArray<FMassEntityHandle> EntitiesQueried;
	
	// 在哈希网格中查询指定边界内的实体
	HashGridData.Query(Bounds, EntitiesQueried);

	// 获取实体管理器和信号子系统
	auto EntityManager = UE::Mass::Utils::GetEntityManager(GetWorld());
	auto EntitySignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	
	// 如果没有查询到实体则直接返回
	if (EntitiesQueried.IsEmpty()) { return; } 

	// 向查询到的实体发送信号
	EntitySignalSubsystem->SignalEntities(HashGridExample::Signals::EntityQueried, EntitiesQueried);
}
