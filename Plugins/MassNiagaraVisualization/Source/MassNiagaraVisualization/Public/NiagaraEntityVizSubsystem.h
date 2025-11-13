// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MassArchetypeTypes.h"
#include "NiagaraEntityVizActor.h"
#include "NiagaraEntityVizSubsystem.generated.h"

/**
 * 
 */
/**
 * @brief Niagara实体可视化子系统类
 * 
 * 该类负责管理Mass Entity框架中的Niagara可视化系统，提供共享的Niagara片段管理和预创建的Niagara演员管理功能。
 * 继承自UWorldSubsystem，作为世界子系统在游戏运行时提供可视化服务。
 */
UCLASS()
class MASSNIAGARAVISUALIZATION_API UNiagaraEntityVizSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	protected:
		/** Mass实体管理器的智能指针 */
		TSharedPtr<FMassEntityManager> MassManager;

		/**
		 * @brief 初始化子系统
		 * 
		 * 重写父类的初始化方法，在子系统创建时进行必要的初始化操作。
		 * 
		 * @param Collection 子系统集合基础引用，用于管理子系统的依赖关系
		 */
		virtual void Initialize(FSubsystemCollectionBase& Collection) override;

		/**
		 * @brief 反初始化子系统
		 * 
		 * 重写父类的反初始化方法，在子系统销毁时清理资源。
		 * 重置Mass管理器并清空预创建的共享Niagara演员映射表。
		 */
		virtual void Deinitialize() override
		{
			MassManager.Reset();
			PreexistingSharedNiagaraActors.Empty();
		};
	public:
		/**
		 * @brief 获取或创建指定系统类型的共享Niagara片段
		 * 
		 * 根据传入的Niagara系统类型、静态网格覆盖和材质覆盖参数，
		 * 查找已存在的共享片段或创建新的共享片段用于实体可视化。
		 * 
		 * @param NiagaraSystem Niagara系统对象指针，定义了粒子系统的外观和行为
		 * @param StaticMeshOverride 静态网格覆盖，用于替换默认的网格渲染
		 * @param MaterialOverride 材质覆盖，用于替换默认材质，可选参数，默认为nullptr
		 * @return FSharedStruct 返回共享结构体，包含Niagara可视化相关的数据
		 */
		FSharedStruct GetOrCreateSharedNiagaraFragmentForSystemType(class UNiagaraSystem* NiagaraSystem, 
			UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride = nullptr);


		/** 预创建的共享Niagara演员映射表，使用uint32作为键，ANiagaraEntityVizActor指针作为值 */
		UPROPERTY()
		TMap<uint32, ANiagaraEntityVizActor*> PreexistingSharedNiagaraActors;
};
