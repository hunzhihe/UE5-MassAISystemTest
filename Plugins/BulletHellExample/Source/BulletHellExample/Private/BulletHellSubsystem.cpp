// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletHellSubsystem.h"

#include "BulletTrait.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"

const FBHEntityHashGrid& UBulletHellSubsystem::GetHashGrid() const
{
	// TODO: 在此处插入 return 语句
	return EntityHashGrid;
}

FBHEntityHashGrid& UBulletHellSubsystem::GetHashGrid_Mutable()
{
	// TODO: 在此处插入 return 语句
	return EntityHashGrid;
}

/**
 * 获取玩家位置
 * @param OutLocation 输出参数，用于返回玩家的位置信息
 */
void UBulletHellSubsystem::GetPlayerLocation(FVector& OutLocation) const
{
	OutLocation = PlayerLocation;
}

/**
 * 生成子弹实体
 * @param BulletConfig 子弹配置资产，定义了子弹的属性和行为
 * @param Location 子弹生成的位置坐标
 * @param Direction 子弹移动的方向向量
 */
void UBulletHellSubsystem::SpawnBullet(UMassEntityConfigAsset* BulletConfig, const FVector& Location, const FVector& Direction)
{
	check(BulletConfig);

	// 获取必要的子系统和管理器
	auto SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	auto SpawnerSystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	// 生成子弹实体
	TArray<FMassEntityHandle> EntitiesSpawned;
	SpawnerSystem->SpawnEntities(BulletConfig->GetOrCreateEntityTemplate(*GetWorld()), 1, EntitiesSpawned);

	// 设置子弹片段数据
	auto& BulletFragment = EntityManager.GetFragmentDataChecked<FBulletFragment>(EntitiesSpawned[0]);
	BulletFragment.Direction = Direction;
	BulletFragment.SpawnLocation = Location;

	// 发送子弹生成信号
	SignalSubsystem->SignalEntity(BulletHell::Signals::BulletSpawned, EntitiesSpawned[0]);
}


/**
 * 每帧更新函数，用于更新玩家位置信息
 * @param DeltaTime 距离上一帧的时间间隔（秒）
 */
void UBulletHellSubsystem::Tick(float DeltaTime)
{
	// 如果缓存的玩家pawn存在，则获取并更新玩家当前位置
	if (CachedPlayerPawn)
	{
		PlayerLocation = CachedPlayerPawn->GetActorLocation();
	}
}

/**
 * 世界开始播放时的初始化回调函数，用于获取玩家pawn引用
 * @param InWorld 当前游戏世界的引用
 */
void UBulletHellSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	// 获取第一玩家控制器的pawn对象并缓存
	CachedPlayerPawn = InWorld.GetFirstPlayerController()->GetPawn();
}

/**
 * 获取统计ID函数，用于性能统计系统
 * @return 返回当前子系统的统计标识符
 */
TStatId UBulletHellSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UBulletHellSubsystem, STATGROUP_Tickables);
}