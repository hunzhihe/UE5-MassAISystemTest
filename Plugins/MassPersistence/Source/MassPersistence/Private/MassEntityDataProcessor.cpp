// Fill out your copyright notice in the Description page of Project Settings.


#include "MassEntityDataProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassPersistentDataSubsystem.h"
#include "MassSignalSubsystem.h"
#include "PersistentDataTrait.h"


/**
 * @brief 构造函数，初始化 EntityQuery 成员变量并绑定当前对象作为其拥有者。
 */
UPersistEntityDataProcessor::UPersistEntityDataProcessor()
	: EntityQuery(*this)
{
}

/**
 * @brief 配置查询条件以筛选需要处理的实体。
 * 
 * 此函数定义了用于遍历实体的查询规则：
 * - 要求实体具有只读访问权限的 FTransformFragment 片段；
 * - 要求实体必须存在 FPersistentDataFragment 共享常量片段；
 * - 要求系统中存在可读写的 UMassPersistentDataSubsystem 子系统。
 *
 * @param EntityManager 实体管理器引用，用于构建和注册查询。
 */
void UPersistEntityDataProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FPersistentDataFragment>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
}

/**
 * @brief 处理信号触发时保存实体数据的操作。
 *
 * 当接收到特定信号（如保存实体）时，该函数会遍历所有符合条件的实体块，
 * 提取每个实体的位置信息，并将其封装成持久化结构后存入子系统的保存游戏中。
 *
 * @param EntityManager 实体管理器，提供对实体数据的访问支持。
 * @param Context 执行上下文，用于获取当前批次中的实体数据。
 * @param EntitySignals 信号名称查找表，用于识别触发此操作的信号类型。
 */
void UPersistEntityDataProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// 遍历所有匹配查询条件的实体块
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取变换片段视图、共享配置片段以及持久化子系统引用
			const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			const auto PersistentDataFragment = Context.GetConstSharedFragment<FPersistentDataFragment>();
			auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();

			// 准备存储本批实体的数据容器
			TArray<FEntitySaveData> EntitySaveData;

			// 遍历当前块内的每一个实体
			const int32 NumEntities = Context.GetNumEntities();
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				// 提取实体变换信息并转换为持久化格式
				const auto& Transform = TransformFragments[EntityIdx];
				FPersistentTransformFragment PersistentTransform;
				PersistentTransform.Transform = Transform.GetTransform();

				// 创建保存数据结构并填充内容
				FEntitySaveData SaveData;
				SaveData.ConfigAsset = PersistentDataFragment.EntityConfig.LoadSynchronous();
				SaveData.EntityFragments = { FInstancedStruct::Make(PersistentTransform) };
				EntitySaveData.Emplace(SaveData);
			}

			// 将收集到的所有实体数据追加至保存游戏文件中
			PersistentDataSubsystem.FindOrCreateSaveGame()->Entities.Append(EntitySaveData);
		});
}

/**
 * @brief 初始化处理器内部状态并订阅相关信号。
 *
 * 在初始化阶段，向信号子系统注册感兴趣的信号（例如保存实体），以便在适当时候调用 SignalEntities 方法。
 *
 * @param Owner 拥有该处理器的对象（通常是世界或关卡）。
 * @param EntityManager 实体管理器的共享引用，供后续使用。
 */
void UPersistEntityDataProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::SaveEntity);
	Super::InitializeInternal(Owner, EntityManager);
}



/**
 * 构造函数，初始化UPersistentDataPostLoadProcessor对象
 * 初始化EntityQuery成员变量，绑定当前对象实例
 */
UPersistentDataPostLoadProcessor::UPersistentDataPostLoadProcessor()
	: EntityQuery(*this)
{
}

/**
 * 配置查询条件，设置实体查询所需的片段访问权限
 * @param EntityManager 实体管理器的共享引用，用于管理实体数据
 */
void UPersistentDataPostLoadProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 添加变换片段的读写权限要求
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	// 添加持久化变换片段的只读权限要求
	EntityQuery.AddRequirement<FPersistentTransformFragment>(EMassFragmentAccess::ReadOnly);
	// 添加持久化数据片段的常量共享要求，确保所有实体都包含此片段
	EntityQuery.AddConstSharedRequirement<FPersistentDataFragment>(EMassFragmentPresence::All);
}

/**
 * 处理实体信号，在实体加载完成后执行变换数据的恢复操作
 * @param EntityManager 实体管理器引用，用于管理实体生命周期
 * @param Context 执行上下文引用，提供执行环境信息
 * @param EntitySignals 实体信号查找引用，用于处理实体相关信号
 */
void UPersistentDataPostLoadProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// 遍历所有符合条件的实体块
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 获取可变的变换片段视图，用于修改实体位置信息
			const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
			// 获取持久化变换片段视图，用于读取保存的变换数据
			const auto PersistentTransformFragments = Context.GetFragmentView<FPersistentTransformFragment>();
			TArray<FEntitySaveData> EntitySaveData;

			// 获取当前块中的实体数量
			const int32 NumEntities = Context.GetNumEntities();
			// 遍历所有实体，将持久化的变换数据恢复到实际变换中
			for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
			{
				auto& Transform = TransformFragments[EntityIdx];
				auto& PersistentTransform = PersistentTransformFragments[EntityIdx];

				// 将持久化存储的变换数据赋值给当前实体的变换
				Transform.GetMutableTransform() = PersistentTransform.Transform;
			}
		});
}

/**
 * 内部初始化函数，订阅实体加载完成信号
 * @param Owner UObject引用，拥有此处理器的对象
 * @param EntityManager 实体管理器的共享引用，用于实体管理
 */
void UPersistentDataPostLoadProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 获取世界中的信号子系统
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	// 订阅实体加载完成信号，以便在实体加载后进行数据恢复
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::EntityLoaded);
	// 调用父类初始化函数
	Super::InitializeInternal(Owner, EntityManager);
}