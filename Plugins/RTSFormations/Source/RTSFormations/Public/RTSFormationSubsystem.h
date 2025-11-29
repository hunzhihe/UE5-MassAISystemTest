// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FormationPresets.h"

#include "MassEntityHandle.h"
#include "MassSubsystemBase.h"

#include "Unit/UnitFragments.h"
#include "RTSFormationSubsystem.generated.h"



/**
 * @brief 前向声明FMassEntityQuery结构体
 * 用于定义实体查询相关的数据结构
 */
struct FMassEntityQuery;

/**
 * @brief 前向声明FMassExecutionContext结构体
 * 用于定义Mass执行上下文相关的数据结构
 */
struct FMassExecutionContext;

/**
 * @brief 前向声明FMassEntityConfig结构体
 * 用于定义实体配置相关的数据结构
 */
struct FMassEntityConfig;

/**
 * @brief 前向声明UMassEntityConfigAsset类
 * 用于定义实体配置资源相关的类
 */
class UMassEntityConfigAsset;

/**
 * @brief 前向声明UMassAgentComponent类
 * 用于定义Mass代理组件相关的类
 */
class UMassAgentComponent;

/**
 * @brief 前向声明FMassEntityHandle结构体
 * 用于定义实体句柄相关的数据结构
 */
struct FMassEntityHandle;

/**
 * @brief RTS命名空间下的Stats子命名空间
 * 用于存储RTS系统中的统计信息
 */
namespace RTS::Stats
{
	/**
	 * @brief 实体索引更新时间统计变量
	 * 记录更新实体索引所花费的时间(秒)
	 */
	inline double UpdateEntityIndexTimeSec = 0.0;

	/**
	 * @brief 单位位置更新时间统计变量
	 * 记录更新单位位置所花费的时间(秒)
	 */
	inline double UpdateUnitPositionTimeSec = 0.0;
}
/**
 * 
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

public:
    

	/**
    * 获取所有单位的句柄数组
    * @return 包含所有单位句柄的TArray数组
    */
    TArray<FUnitHandle> GetUnits() const;

/**
 * 获取第一个单位的句柄
 * @return 第一个单位的句柄
 */
UFUNCTION(BlueprintCallable)
FUnitHandle GetFirstUnit() const;

/**
 * 销毁指定的实体
 * @param Entity 要销毁的MassAgent组件实体
 */
UFUNCTION(BlueprintCallable)
void DestroyEntity(UMassAgentComponent* Entity);

/**
 * 更新指定单位的位置信息
 * @param UnitHandle 要更新位置的单位句柄
 */
void UpdateUnitPosition(const FUnitHandle& UnitHandle);

/**
 * 设置指定单位的位置
 * @param NewPosition 新的位置坐标
 * @param UnitHandle 要设置位置的单位句柄
 */
UFUNCTION(BlueprintCallable)
void SetUnitPosition(const FVector& NewPosition, const FUnitHandle& UnitHandle);

/**
 * 为指定单位生成实体
 * @param UnitHandle 目标单位句柄
 * @param EntityConfig 实体配置资源
 * @param Count 要生成的实体数量
 */
UFUNCTION(BlueprintCallable)
void SpawnEntitiesForUnit(const FUnitHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count);

/**
 * 生成实体
 * @param UnitHandle 单位句柄
 * @param EntityConfig 实体配置
 * @param Count 要生成的实体数量
 */
void SpawnEntities(const FUnitHandle& UnitHandle, const FMassEntityConfig& EntityConfig, int Count);

/**
 * 生成新的单位
 * @param EntityConfig 实体配置资源
 * @param Count 要生成的实体数量
 * @param Position 生成位置
 * @return 新生成单位的句柄
 */
UFUNCTION(BlueprintCallable)
FUnitHandle SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position);

/**
 * 生成单位
 * @param EntityConfig 实体配置
 * @param Count 要生成的实体数量
 * @param Position 生成位置
 * @return 新生成单位的句柄
 */
FUnitHandle SpawnUnit(const FMassEntityConfig& EntityConfig, int Count, const FVector& Position);

/**
 * 为指定单位设置编队预设
 * @param UnitHandle 目标单位句柄
 * @param FormationAsset 编队预设资源
 */
UFUNCTION(BlueprintCallable)
void SetFormationPreset(const FUnitHandle& UnitHandle, UFormationPresets* FormationAsset);

/**
 * 计算单位的新位置
 * @param UnitFragment 单位片段引用
 * @param Count 实体数量
 * @param OutNewPositions 输出的新位置数组
 */
static void CalculateNewPositions(FUnitFragment& UnitFragment, int Count, TArray<FVector3f>& OutNewPositions);

/**
 * 为单位创建查询
 * @param UnitHandle 单位句柄
 * @param EntityQuery 实体查询引用
 */
static void CreateQueryForUnit(const FUnitHandle& UnitHandle, FMassEntityQuery& EntityQuery);
	
};

/**
 * @brief RTSFormationSubsystem的外部子系统特性模板特化
 * 
 * 该结构体为URTSFormationSubsystem类型特化了TMassExternalSubsystemTraits模板，
 * 定义了该子系统在Mass框架中的线程安全特性。
 * 
 * @tparam URTSFormationSubsystem 被特化的子系统类型
 */
template<>
struct TMassExternalSubsystemTraits<URTSFormationSubsystem> final
{
	/**
	 * @brief 子系统特性枚举定义
	 * 
	 * 定义了URTSFormationSubsystem子系统的线程访问特性：
	 * - GameThreadOnly: 指示该子系统是否只能在游戏线程中访问
	 * - ThreadSafeWrite: 指示该子系统是否支持线程安全的写操作
	 */
	enum
	{
		/** @brief 是否仅限游戏线程访问 - 设置为false表示可在其他线程访问 */
		GameThreadOnly = false,
		/** @brief 是否支持线程安全写入 - 设置为false表示不支持线程安全写入 */
		ThreadSafeWrite = false
	};
};