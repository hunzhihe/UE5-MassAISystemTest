// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraRepresentationProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassRepresentationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "NiagaraEntityVizActor.h"
#include "MSRepresentationFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"





/**
 * 构造函数，初始化Niagara表示处理器的执行标志、执行组和处理阶段
 * 
 * 该构造函数设置处理器在客户端、独立模式和编辑器环境下执行，
 * 并将其配置为在表示组中执行，处理阶段设置为帧结束时执行。
 */
UNiagaraRepresentationProcessor::UNiagaraRepresentationProcessor():
    NiagaraPositionChunkQuery(*this)
{
	// 设置处理器的执行标志，允许在客户端、独立模式和编辑器环境中执行
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);

	// 配置执行顺序，将处理器分配到表示处理组
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	
	// 设置处理阶段为帧结束时执行
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
}
/**
 * 配置Niagara系统的查询需求
 * 
 * 该函数用于设置Niagara表示处理器所需的实体数据查询条件，
 * 包括位置、速度和共享的Niagara系统片段的访问权限配置。
 * 
 * @param EntityManager 实体管理器的共享引用，用于管理实体数据和查询
 */
void UNiagaraRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 配置Niagara位置查询所需的数据片段访问权限
	NiagaraPositionChunkQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	NiagaraPositionChunkQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
	NiagaraPositionChunkQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
}

// todo-performance separate setup for rarely moving pieces?
void UNiagaraRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	/**
 * @brief 遍历所有实体块（Entity Chunk），收集每个实体的位置、朝向和动画索引信息，
 *        并将这些数据存储到共享的 Niagara 系统片段中。
 *
 * 此 Lambda 函数作为 NiagaraPositionChunkQuery 的 ForEachEntityChunk 方法的回调执行。
 * 它从每个实体中提取变换信息和速度信息，并将其转换为粒子系统所需的数据格式。
 *
 * @param Context FMassExecutionContext 引用，提供对当前处理实体块上下文的访问。
 */
NiagaraPositionChunkQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PositionChunkQuery);
    const int32 QueryLength = Context.GetNumEntities();

    // 获取只读的变换片段视图
    const auto& Transforms = Context.GetFragmentView<FTransformFragment>();
    // 获取可变的共享 Niagara 片段引用
    auto& SharedNiagaraFragment = Context.GetMutableSharedFragment<FSharedNiagaraSystemFragment>();
    // 获取只读的速度片段视图
    const auto& VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();

    // 遍历当前块中的每一个实体
    for (int32 i = 0; i < QueryLength; ++i)
    {
        const auto& VelocityFragment = VelocityFragments[i];
        auto& Transform = Transforms[i].GetTransform();
        
        // 将位置、旋转和动画状态推入共享 Niagara 数据结构中
        SharedNiagaraFragment.ParticlePositions.Emplace(Transform.GetTranslation());
        SharedNiagaraFragment.ParticleOrientations.Emplace((FQuat4f)Transform.GetRotation());
        SharedNiagaraFragment.AnimationIndexes.Emplace(VelocityFragment.Value.Length() > 5.f ? 1 : 0); // 根据速度切换空闲/跑步动画
    }
});

/**
 * @brief 遍历所有的 FSharedNiagaraSystemFragment 共享片段，将其中缓存的粒子数据推送至对应的 Niagara 系统组件。
 *
 * 此 Lambda 函数通过 EntityManager 调用，用于更新 Niagara 粒子系统的输入数组。
 * 推送完成后会重置共享片段中的临时数据容器以供下一帧使用。
 *
 * @param SharedNiagaraFragment 当前遍历到的 FSharedNiagaraSystemFragment 实例引用。
 */
EntityManager.ForEachSharedFragment<FSharedNiagaraSystemFragment>([](FSharedNiagaraSystemFragment& SharedNiagaraFragment)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_MassToNiagara);
    const ANiagaraEntityVizActor* NiagaraActor = SharedNiagaraFragment.NiagaraManagerActor.Get();

    if (UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent())
    {
        // 使用 Niagara 提供的接口设置粒子数组参数：位置、方向和动画索引
        UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, SharedNiagaraFragment.ParticlePositionName,
            SharedNiagaraFragment.ParticlePositions);
        UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(NiagaraComponent, SharedNiagaraFragment.ParticleOrientationsParameterName,
            SharedNiagaraFragment.ParticleOrientations);
        UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayUInt8(NiagaraComponent, SharedNiagaraFragment.AnimationIndexesParameterName,
            SharedNiagaraFragment.AnimationIndexes);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("projectile manager %s was invalid during array push!"), *NiagaraActor->GetName());
    }

    // 清除已使用的数据以便下一次填充
    // @todo optimize entities that dont change often
    SharedNiagaraFragment.ParticleOrientations.Reset();
    SharedNiagaraFragment.ParticlePositions.Reset();
    SharedNiagaraFragment.AnimationIndexes.Reset();
});
}