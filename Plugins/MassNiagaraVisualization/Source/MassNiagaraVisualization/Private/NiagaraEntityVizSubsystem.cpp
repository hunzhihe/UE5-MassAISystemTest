// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraEntityVizSubsystem.h"
#include "NiagaraEntityVizActor.h"
#include "MassEntitySubsystem.h"

#include "MSRepresentationFragments.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraTickBehaviorEnum.h"



void UNiagaraEntityVizSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	auto MassSubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();

	MassManager = MassSubsystem->GetMutableEntityManager().AsShared();
}

FSharedStruct UNiagaraEntityVizSubsystem::GetOrCreateSharedNiagaraFragmentForSystemType(UNiagaraSystem* NiagaraSystem,
    UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride)
{
	/**
 * 函数功能：根据 Niagara 系统、静态网格和材质的配置，创建或复用一个共享的 Niagara 可视化 Actor，
 *           并返回对应的共享结构体片段。
 *
 * 参数说明：
 * - NiagaraSystem: 指向要使用的 Niagara 系统资产的指针，不能为空。
 * - StaticMeshOverride: 可选的静态网格资源，用于覆盖 Niagara 系统中的默认网格。
 * - MaterialOverride: 可选的材质资源，用于覆盖静态网格的默认材质。
 *
 * 返回值说明：
 * - 返回一个 FSharedNiagaraSystemFragment 结构体，包含对所创建或复用的 Niagara Actor 的引用。
 *
 * 逻辑说明：
 * 1. 根据 Niagara 系统路径名生成基础哈希；
 * 2. 若存在 StaticMeshOverride 或 MaterialOverride，则更新哈希值以区分不同配置；
 * 3. 查询是否已有相同配置的 Actor 存在，若存在则直接复用；
 * 4. 否则创建新的 Niagara 可视化 Actor，并设置相关组件参数；
 * 5. 将新创建的 Actor 缓存起来供后续复用，并通过 MassManager 获取共享结构体返回。
 */

    uint32 NiagaraAssetHash = GetTypeHash(NiagaraSystem->GetPathName());
    uint32 ParamsHash = NiagaraAssetHash;

    // 如果指定了静态网格覆盖，则将其 FName 哈希与 Niagara 资产哈希组合
    if (StaticMeshOverride)
    {
	   ParamsHash = HashCombineFast(NiagaraAssetHash, GetTypeHash(StaticMeshOverride->GetFName()));
    }

    // 如果指定了材质覆盖，则将其 FName 哈希与 Niagara 资产哈希组合
    if (MaterialOverride)
    {
       ParamsHash = HashCombineFast(NiagaraAssetHash, GetTypeHash(MaterialOverride->GetFName()));
    }

    /**
 * 函数功能：根据参数哈希值查找或创建共享的Niagara系统片段
 * 
 * 参数说明：
 *   ParamsHash - 用于标识预存在共享Niagara演员的哈希值参数
 * 
 * 返回值说明：
 *   返回一个FSharedNiagaraSystemFragment类型的共享片段对象
 * 
 * 功能描述：
 *   首先创建一个默认的共享Niagara系统片段对象，然后检查给定的参数哈希值
 *   是否在预存在的共享Niagara演员集合中。如果存在，则通过MassManager获取
 *   或创建对应的共享片段并返回。
 */

    // 创建默认的共享Niagara系统片段对象
    FSharedNiagaraSystemFragment SharedStructToReturn = FSharedNiagaraSystemFragment();

    
    // 检查参数哈希值是否存在于预存在的共享Niagara演员集合中
    if (PreexistingSharedNiagaraActors.Contains(ParamsHash))
    {
	   // 如果存在，则通过MassManager获取或创建共享片段并返回
	   return MassManager->GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>(SharedStructToReturn);
    }

    // 配置 Actor 的生成参数，标记为临时对象
       FActorSpawnParameters SpawnParams;
       SpawnParams.ObjectFlags = RF_Transient | RF_DuplicateTransient;

    // 生成新的 Niagara 可视化 Actor
       ANiagaraEntityVizActor* NewNiagaraActor = GetWorld()->SpawnActor<ANiagaraEntityVizActor>(SpawnParams);

    // 设置 Niagara 组件的更新行为和绑定的资产
       NewNiagaraActor->GetNiagaraComponent()->SetTickBehavior(ENiagaraTickBehavior::ForceTickLast);
       NewNiagaraActor->GetNiagaraComponent()->SetAsset(NiagaraSystem);

    // 如果提供了静态网格覆盖，则设置 Niagara 中对应的变量
    if (StaticMeshOverride)
    {
	   NewNiagaraActor->GetNiagaraComponent()->SetVariableStaticMesh("StaticMeshToRender", StaticMeshOverride);

	   // 设置材质变量：优先使用 MaterialOverride，否则使用 StaticMesh 的第一个材质
	   if (MaterialOverride)
	   {
		NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", MaterialOverride);
	   }
	   else
	   {
		NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", StaticMeshOverride->GetMaterial(0));
	   }
    }

    // 将新创建的 Niagara Actor 赋值给共享结构体
    SharedStructToReturn.NiagaraManagerActor = NewNiagaraActor;

    // 将该 Actor 缓存到映射表中以便后续复用
    PreexistingSharedNiagaraActors.FindOrAdd(ParamsHash, NewNiagaraActor);

    // 通过 MassManager 获取或创建共享结构体并返回
    return MassManager->GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>(SharedStructToReturn);
}
