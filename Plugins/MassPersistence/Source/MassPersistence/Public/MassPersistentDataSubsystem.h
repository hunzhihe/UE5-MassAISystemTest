// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MassExternalSubsystemTraits.h"
#include "MassSaveGame.h"
#include "MassSubsystemBase.h"
#include "MassPersistentDataSubsystem.generated.h"

//三个信号名称
namespace PersistentData::Signals
{
	//实体数据保存的信号
	const FName SaveEntity = FName(TEXT("SaveEntity"));
	//实体位置随机化的信号
	const FName RandomizePositions = FName(TEXT("RandomizePositions"));
	//实体从保存数据加载完成后出发的信号
	const FName EntityLoaded = FName(TEXT("EntityLoaded"));
}

struct FMassEntityHandle;
class USaveGame;
/**
 * 持久化管理子系统
 */
UCLASS()
class MASSPERSISTENCE_API UMassPersistentDataSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

public:
	/**
 * @brief 将实体保存到指定的存档槽位中
 * 
 * @param SlotName 存档槽位的名称，用于标识要保存到哪个存档文件
 */
UFUNCTION(BlueprintCallable)
	void SaveEntities(const FString& SlotName);

/**
 * @brief 保存实体数据到存档系统
 * 
 * @param Entities 要保存的实体句柄数组
 */
void SaveEntityData(TArray<FMassEntityHandle>& Entities);

/**
 * @brief 查找或创建存档游戏对象
 * 
 * @return 返回找到的或新创建的UMassSaveGame对象指针
 */
UFUNCTION(BlueprintCallable)
	UMassSaveGame* FindOrCreateSaveGame();

/**
 * @brief 从存档文件中加载实体数据
 * 
 * @param SaveGameFile 包含实体数据的存档游戏对象指针
 */
UFUNCTION(BlueprintCallable)
	void LoadEntitiesFromSave(UMassSaveGame* SaveGameFile);

/**
 * @brief 根据配置资产生成指定数量的实体
 * 
 * @param ConfigAsset 实体配置资产，定义了要生成的实体类型和属性
 * @param Amount 要生成的实体数量
 */
UFUNCTION(BlueprintCallable)
	void SpawnEntities(UMassEntityConfigAsset* ConfigAsset, int Amount);

/**
 * @brief 随机化所有管理实体的位置
 * 
 * 此函数会遍历所有受管理的实体，并将它们的位置设置为随机值
 */
UFUNCTION(BlueprintCallable)
	void RandomizePositions();

/**
 * @brief 清除所有持久化的实体数据
 * 
 * 此函数会移除所有已保存的实体记录，重置管理系统状态
 */
UFUNCTION(BlueprintCallable)
	void ClearPersistedEntities();

/**
 * @brief 指向当前存档游戏对象的指针
 * 
 * 用于存储和管理实体的序列化数据
 */
UPROPERTY(BlueprintReadWrite)
	UMassSaveGame* SaveGame;

/**
 * @brief 存储所有受管理的实体句柄数组
 * 
 * 包含当前系统正在跟踪的所有实体的句柄列表
 */
UPROPERTY()
	TArray<FMassEntityHandle> ManagedEntities;
	
};

//支持多线程处理实体数据
template<>
struct TMassExternalSubsystemTraits<UMassPersistentDataSubsystem>
{
	enum
	{
		GameThreadOnly = false
	};
};