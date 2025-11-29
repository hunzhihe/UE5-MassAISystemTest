// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGridExampleProcessors.h"

#include "HashGridFragments.h"
#include "HashGridSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"


/**
 * 静态变量定义，表示哈希网格的一半范围值，用于空间划分计算
 */
static float HalfRange = 25.f;

/**
 * UHashGridInitializeProcessor类的构造函数
 * 初始化实体查询对象，并设置观察的片段类型和操作类型
 * 
 * 构造函数初始化列表：
 * - EntityQuery(*this): 使用当前对象初始化实体查询
 * 
 * 成员变量初始化：
 * - ObservedType: 设置为FHashGridFragment的静态结构信息
 * - Operation: 设置为添加操作类型
 */
UHashGridInitializeProcessor::UHashGridInitializeProcessor() :
	EntityQuery(*this)
{
    
	ObservedType = FHashGridFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

/**
 * 配置实体查询所需的片段和子系统要求
 * 
 * @param EntityManager 实体管理器的共享引用，用于管理实体和片段
 * 
 * 该函数为EntityQuery添加以下要求：
 * - FHashGridFragment片段的读写访问权限
 * - FTransformFragment片段的只读访问权限  
 * - UHashGridSubsystem子系统的读写访问权限
 */
void UHashGridInitializeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
}
/**
 * @brief 执行哈希网格初始化处理器，为实体分配哈希网格单元位置
 * 
 * 该函数遍历所有符合条件的实体块，为每个实体计算其在哈希网格中的位置，
 * 并将实体添加到对应的网格单元中。
 * 
 * @param EntityManager 实体管理器引用，用于管理实体数据
 * @param Context 执行上下文引用，包含当前处理的实体信息和子系统访问接口
 */
void UHashGridInitializeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // 遍历所有实体块，在每个块中处理实体的哈希网格初始化
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
        {
            // 获取可变的哈希网格子系统引用
            auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();

            // 获取变换片段和哈希网格片段的只读视图
            const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
            const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();

            // 遍历当前块中的所有实体，为每个实体分配哈希网格单元位置
            const int32 NumEntities = Context.GetNumEntities();
            for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
            {
                auto& HashGridFragment = HashGridFragments[EntityIdx];
                auto& TransformFragment = TransformFragments[EntityIdx];
                auto Location = TransformFragment.GetTransform().GetLocation();

                // 根据实体位置计算边界框，并将实体添加到哈希网格数据结构中
                FBox Bounds = { Location - HalfRange, Location + HalfRange };
                HashGridFragment.CellLocation = HashGridSubsystem.HashGridData.Add(Context.GetEntity(EntityIdx), Bounds);
            }
        });
}

/**
 * 构造函数，初始化UHashGridDestroyProcessor对象
 * 
 * 初始化EntityQuery成员，并设置观察的实体类型为FHashGridFragment结构体，
 * 操作类型设置为移除操作
 */
UHashGridDestroyProcessor::UHashGridDestroyProcessor() :
    EntityQuery(*this)
{
    ObservedType = FHashGridFragment::StaticStruct();
    Operation = EMassObservedOperation::Remove;
}

/**
 * 配置查询条件，为实体查询添加必要的片段和子系统要求
 * 
 * @param EntityManager 共享引用的实体管理器，用于配置查询
 * 
 * 此函数为EntityQuery添加了两个要求：
 * 1. 只读访问FHashGridFragment片段
 * 2. 读写访问UHashGridSubsystem子系统
 */
void UHashGridDestroyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 执行哈希网格销毁处理器，从哈希网格数据结构中移除实体
 * 
 * 该函数遍历所有符合条件的实体块，从哈希网格子系统中移除每个实体的哈希网格数据。
 * 主要用于实体销毁时清理其在空间分区哈希网格中的引用。
 * 
 * @param EntityManager 实体管理器引用，提供对实体数据的访问
 * @param Context 执行上下文引用，包含当前处理的实体信息和子系统访问接口
 */
void UHashGridDestroyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // 遍历所有实体块，对每个块中的实体执行哈希网格数据清理操作
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
        {
            // 获取可变的哈希网格子系统引用，用于修改哈希网格数据
            auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();

            // 获取当前块中所有实体的哈希网格片段视图
            const auto HashGridFragments = Context.GetFragmentView<FHashGridFragment>();

            // 遍历当前块中的所有实体，从哈希网格中移除它们
            const int32 NumEntities = Context.GetNumEntities();
            for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
            {
                // 获取当前实体的哈希网格片段信息
                const auto& HashGridFragment = HashGridFragments[EntityIdx];

                // 从哈希网格数据中移除实体及其对应的网格位置信息
                HashGridSubsystem.HashGridData.Remove(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation);
            }
        });
}


/**
 * 构造函数，初始化UHashGridProcessor对象
 * 通过初始化列表初始化基类EntityQuery
 */
UHashGridProcessor::UHashGridProcessor() :
    EntityQuery(*this)
{
}

/**
 * 配置查询所需的片段和子系统要求
 * @param EntityManager 实体管理器的共享引用，用于管理实体和片段
 */
void UHashGridProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    // 添加哈希网格片段的读写权限要求
    EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadWrite);
    // 添加变换片段的只读权限要求
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    // 添加哈希网格子系统的读写权限要求
    EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * 执行处理器逻辑，处理实体在哈希网格中的位置更新
 * @param EntityManager 实体管理器引用，用于管理实体
 * @param Context 执行上下文引用，提供执行环境信息
 */
void UHashGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // 遍历每个实体块并更新实体在哈希网格中的位置
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
        {
            // 获取可变的哈希网格子系统引用
            auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();

            // 获取变换片段和哈希网格片段的视图
            const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
            const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();

            // 获取当前块中实体的数量
            const int32 NumEntities = Context.GetNumEntities();
            
            // 遍历所有实体，更新它们在哈希网格中的位置
            for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
            {
                auto& HashGridFragment = HashGridFragments[EntityIdx];
                auto& TransformFragment = TransformFragments[EntityIdx];
                const auto& Location = TransformFragment.GetTransform().GetLocation();

                // 根据实体的新位置计算边界框，并移动实体到新的网格单元
                FBox Bounds = { Location - HalfRange, Location + HalfRange };
                auto NewCellLocation = HashGridSubsystem.HashGridData.Move(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation, Bounds);
                HashGridFragment.CellLocation = NewCellLocation;
            }
        });
}



/**
 * 初始化处理器内部状态
 * @param Owner 处理器的所有者对象引用
 * @param EntityManager 实体管理器的共享引用
 */
void UHashGridQueryProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
    Super::InitializeInternal(Owner, EntityManager);
    auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
    SubscribeToSignal(*SignalSubsystem, HashGridExample::Signals::EntityQueried);
}

/**
 * 配置查询所需的片段要求
 * @param EntityManager 实体管理器的共享引用
 */
void UHashGridQueryProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

/**
 * 处理信号并执行实体操作
 * @param EntityManager 实体管理器引用
 * @param Context 执行上下文引用
 * @param EntitySignals 实体信号名称查找引用
 */
void UHashGridQueryProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
    FMassSignalNameLookup& EntitySignals)
{
    // 遍历所有实体块并处理其中的实体
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
        {
            const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();

            const int32 NumEntities = Context.GetNumEntities();
            for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
            {
                auto& TransformFragment = TransformFragments[EntityIdx];

                // 销毁接收到信号的实体并在世界中绘制调试点
                Context.Defer().DestroyEntity(Context.GetEntity(EntityIdx));

                DrawDebugPoint(Context.GetWorld(), TransformFragment.GetTransform().GetLocation(), 50.f, FColor::Red, true);
            }
        });
}

