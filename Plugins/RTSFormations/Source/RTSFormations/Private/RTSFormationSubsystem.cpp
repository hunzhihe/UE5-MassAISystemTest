// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "DrawDebugHelpers.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntityBuilder.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassObserverNotificationTypes.h"
#include "MassSignalSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSSignals.h"
#include "Engine/World.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "Unit/UnitFragments.h"

/**
 * @brief 获取所有单位的句柄数组
 * 
 * 该函数遍历实体管理器中的所有共享片段，提取其中的单位句柄，
 * 并将它们收集到一个数组中返回。
 * 
 * @return TArray<FUnitHandle> 包含所有单位句柄的数组
 */
TArray<FUnitHandle> URTSFormationSubsystem::GetUnits() const
{
	// 创建用于存储单位句柄的数组
	TArray<FUnitHandle> UnitArray;

	// 获取世界中的实体管理器引用
	auto& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	// 遍历所有共享片段中的单位片段，并将单位句柄添加到数组中
	EntityManager.ForEachSharedFragment<FUnitFragment>([&UnitArray](FUnitFragment& UnitFragment)
		{
			UnitArray.Emplace(UnitFragment.UnitHandle);
		});

	return UnitArray;
}

/**
 * 获取编队子系统中的第一个单位
 * 
 * @return 返回编队中的第一个单位句柄，如果编队为空则返回默认构造的单位句柄
 */
FUnitHandle URTSFormationSubsystem::GetFirstUnit() const
{
	// 如果单位列表为空，返回默认单位句柄，否则返回列表中第一个单位
	return GetUnits().IsEmpty() ? FUnitHandle() : GetUnits()[0];
}

/**
 * 销毁指定的实体对象
 * 
 * @param Entity 要销毁的Mass Agent组件指针，该组件包含了要销毁的实体信息
 */
void URTSFormationSubsystem::DestroyEntity(UMassAgentComponent* Entity)
{
	// 获取世界中的Mass实体子系统
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySubsystem);

	// 通过实体管理器延迟销毁指定的实体
	EntitySubsystem->GetEntityManager().Defer().DestroyEntity(Entity->GetEntityHandle());
}

/**
 * @brief 更新指定单位中所有实体的位置，使其符合当前的编队布局。
 *
 * 此函数会根据单位句柄查找对应的单位片段，并计算该单位下所有实体的新位置，
 * 然后将这些新位置应用到各个实体上。同时还会发送信号通知其他系统编队已更新。
 *
 * @param UnitHandle 指定要更新位置的单位句柄。
 */
void URTSFormationSubsystem::UpdateUnitPosition(const FUnitHandle& UnitHandle)
{
	auto& InEntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	// 遍历所有共享片段中的 FUnitFragment，找到与给定 UnitHandle 匹配的片段并执行处理逻辑
	InEntityManager.ForEachSharedFragmentConditional<FUnitFragment>(
		[&UnitHandle](const FUnitFragment& InUnitFragment)
		{
			return InUnitFragment.UnitHandle == UnitHandle;
		},
		[&InEntityManager, &UnitHandle](FUnitFragment& UnitFragment)
		{
			TArray<FVector3f> NewPositions;

			// 创建查询对象用于获取属于该单位的所有实体
			FMassEntityQuery EntityQuery(InEntityManager.AsShared());
			CreateQueryForUnit(UnitHandle, EntityQuery);
			EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
			FMassExecutionContext ExecutionContext(InEntityManager);

			TArray<FVector3f> RotatedNewPositions;
			TArray<FMassEntityHandle> Entities;

			{
				// 性能统计：记录 UpdateUnitPosition 的耗时
				RTS::Stats::UpdateUnitPositionTimeSec = 0.0;
				FScopedDurationTimer DurationTimer(RTS::Stats::UpdateUnitPositionTimeSec);
				TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UpdateUnitPosition"))

				// 收集所有匹配查询条件的实体句柄
				EntityQuery.ForEachEntityChunk(ExecutionContext, [&Entities](FMassExecutionContext& Context)
					{
						Entities.Append(Context.GetEntities());
					});

				// 根据实体数量计算新的编队位置
				CalculateNewPositions(UnitFragment, Entities.Num(), NewPositions);

				// 对新位置进行旋转和平移变换以适应世界坐标系
				RotatedNewPositions = NewPositions;
				for (FVector3f& RotatedNewPosition : RotatedNewPositions)
				{
					RotatedNewPosition = RotatedNewPosition.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector3f(0.f, 0.f, 1.f));
					RotatedNewPosition += UnitFragment.InterpDestination;
				}
			}

			{
				// 应用偏移到每个实体的 FormationAgent 片段中
				RTS::Stats::UpdateEntityIndexTimeSec = 0.0;
				FScopedDurationTimer DurationTimer(RTS::Stats::UpdateEntityIndexTimeSec);

				EntityQuery.ForEachEntityChunk(ExecutionContext, [&NewPositions, &RotatedNewPositions](FMassExecutionContext& Context)
					{
						auto FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
						auto TransformFragments = Context.GetFragmentView<FTransformFragment>();

						for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
						{
							FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
							auto& Transform = TransformFragments[EntityIndex].GetTransform();

							int ClosestIndex = 0;
							float DistSq = FLT_MAX;

							// 找出当前位置最接近的目标点索引
							auto Location = FVector3f(Transform.GetLocation());
							for (int i = 0; i < NewPositions.Num(); i++)
							{
								float Dist = FVector3f::DistSquared2D(RotatedNewPositions[i], Location);

								if (Dist < DistSq)
								{
									ClosestIndex = i;
									DistSq = Dist;
								}
							}

							// 将最近目标点作为该实体的新偏移量
							FormationAgent.Offset = NewPositions[ClosestIndex];

							// 移除已被分配的位置，防止重复使用
							NewPositions.RemoveAtSwap(ClosestIndex, EAllowShrinking::No);
							RotatedNewPositions.RemoveAtSwap(ClosestIndex, EAllowShrinking::No);
						}
					});

				// 发送编队更新信号给相关的实体
				auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(InEntityManager.GetWorld());
				SignalSubsystem->SignalEntities(RTS::Unit::Signals::FormationUpdated, Entities);
			}
		});
}

/**
 * 设置单位的位置，并更新其朝向和移动状态。
 *
 * @param NewPosition 新的目标位置（世界坐标）
 * @param UnitHandle  要设置位置的单位句柄
 */
void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, const FUnitHandle& UnitHandle)
{
	// 获取实体子系统及可变的实体管理器
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	// 遍历所有共享片段中的 FUnitFragment，查找匹配 UnitHandle 的单位并进行处理
	EntityManager.ForEachSharedFragmentConditional<FUnitFragment>(
		[&UnitHandle](FUnitFragment& UnitFragment)
		{
			return UnitHandle == UnitFragment.UnitHandle;
		},
		[&EntityManager, &NewPosition, &UnitHandle](FUnitFragment& UnitFragment)
		{
			// 将新位置转换为 FVector3f 类型
			auto NewPosition3f = FVector3f(NewPosition);

			// 绘制调试箭头表示目标方向
			DrawDebugDirectionalArrow(
				EntityManager.GetWorld(),
				NewPosition,
				FVector(NewPosition3f + ((NewPosition3f - UnitFragment.InterpDestination).GetSafeNormal() * 250.f)),
				150.f,
				FColor::Red,
				false,
				5.f,
				0,
				25.f
			);

			// 计算单位面向的方向
			auto ForwardDir = (NewPosition3f - UnitFragment.InterpDestination).GetSafeNormal();
			UnitFragment.ForwardDir = FVector2f(ForwardDir.X, ForwardDir.Y);

			// 根据面向方向计算旋转角度
			UnitFragment.UnitRotation = FRotator3f(UE::Math::TRotationMatrix<float>::MakeFromX(ForwardDir).Rotator());

			// 判断是否需要平滑旋转过渡
			auto UnitInterpQuat = UnitFragment.InterpRotation.Quaternion();
			auto UnitQuat = UnitFragment.UnitRotation.Quaternion();
			bool bBlendAngle = FMath::RadiansToDegrees(UnitInterpQuat.AngularDistance(UnitQuat)) < 45;

			// 若角度差较小则保持当前插值旋转，否则使用新的旋转
			UnitFragment.InterpRotation = bBlendAngle ? UnitFragment.InterpRotation : UnitFragment.UnitRotation;

			{
				// 停止单位的移动行为：创建查询以获取该单位对应的实体，并修改其移动目标动作
				FMassEntityQuery EntityQuery(EntityManager.AsShared());
				CreateQueryForUnit(UnitHandle, EntityQuery);
				EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);

				FMassExecutionContext Context(EntityManager);
				EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
				{
					auto MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

					for (auto Entity : Context.CreateEntityIterator())
					{
						MoveTargetFragments[Entity].CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
					}
				});
			}

			// 更新单位的目标位置
			UnitFragment.UnitDestination = NewPosition3f;

			// 如果不需要平滑旋转，则重新计算插值目的地
			if (!bBlendAngle)
			{
				FMassEntityQuery EntityQuery(EntityManager.AsShared());
				CreateQueryForUnit(UnitHandle, EntityQuery);

				EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
				FMassExecutionContext ExecContext(EntityManager);

				// 查找离新位置最近的实体位置作为插值目的地
				FVector ClosestLocation;
				float ClosestDistanceSq = FLT_MAX;
				EntityQuery.ForEachEntityChunk(ExecContext, [&NewPosition, &ClosestDistanceSq, &ClosestLocation](FMassExecutionContext& Context)
				{
					auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
					for (int i = 0; i < Context.GetNumEntities(); i++)
					{
						const FVector& Location = TransformFragments[i].GetTransform().GetLocation();

						auto LocationDistanceSq = FVector::DistSquared2D(Location, NewPosition);
						if (LocationDistanceSq < ClosestDistanceSq)
						{
							ClosestDistanceSq = LocationDistanceSq;
							ClosestLocation = Location;
						}
					}
				});

				UnitFragment.InterpDestination = FVector3f(ClosestLocation);
			}
		});

	// 最后调用更新单位位置的方法
	UpdateUnitPosition(UnitHandle);
}

/**
 * 为指定单位生成实体对象
 * 
 * @param UnitHandle 单位句柄，用于标识要生成实体的单位
 * @param EntityConfig 实体配置资源，包含实体的配置信息
 * @param Count 要生成的实体数量
 * 
 * 该函数根据给定的单位句柄和实体配置，批量生成指定数量的实体对象。
 * 首先验证实体配置的有效性，然后调用底层的实体生成接口完成实际的创建操作。
 */
void URTSFormationSubsystem::SpawnEntitiesForUnit(const FUnitHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count)
{
	// 检查实体配置是否有效，无效则直接返回
	if (!ensure(EntityConfig)) { return; }

	// 调用底层接口生成实体
	SpawnEntities(UnitHandle, EntityConfig->GetConfig(), Count);
}

/**
 * @brief 在指定单位下生成实体对象
 * 
 * 该函数通过实体管理器创建指定数量的实体，并将它们与指定单位关联。
 * 函数使用延迟命令执行实体创建，确保在合适的时机进行实体初始化。
 * 
 * @param UnitHandle 单位句柄，用于标识实体所属的单位
 * @param EntityConfig 实体配置信息，定义了要创建的实体的模板和初始属性
 * @param Count 要创建的实体数量
 */
void URTSFormationSubsystem::SpawnEntities(const FUnitHandle& UnitHandle, const FMassEntityConfig& EntityConfig,
	int Count)
{
	auto& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	// 手动设置单位索引需要做一些额外工作，否则直接使用SpawnEntities就足够了
	
	auto& EntityTemplate = EntityConfig.GetOrCreateEntityTemplate(*GetWorld());

	
    // 使用延迟命令来创建实体，确保在合适的时机执行实体创建逻辑
	EntityManager.Defer().PushCommand<FMassDeferredCreateCommand>([EntityTemplate, UnitHandle, Count](FMassEntityManager& InEntityManager)
		{
			FMassArchetypeSharedFragmentValues SharedFragmentValues = EntityTemplate.GetSharedFragmentValues();

			// 创建单位片段并设置单位句柄
			FUnitFragment UnitFragment = FUnitFragment();
			UnitFragment.UnitHandle = UnitHandle;

			// 获取或创建共享的单位片段，并添加到共享片段值集合中
			auto& SharedUnitFragment = InEntityManager.GetOrCreateSharedFragment<FUnitFragment>(UnitFragment);
			SharedFragmentValues.Add(SharedUnitFragment);
			SharedFragmentValues.Sort();

			// 批量创建实体
			TArray<FMassEntityHandle> Entities;

			auto CreationContext = InEntityManager.BatchCreateEntities(EntityTemplate.GetArchetype(), SharedFragmentValues, Count,Entities);

			
			// 设置实体的初始片段值
			TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate.GetInitialFragmentValues();
			InEntityManager.BatchSetEntityFragmentValues(CreationContext->GetEntityCollections(InEntityManager), FragmentInstances);
	    
		});	    
}

/**
 * @brief 生成新的单位实体
 * @param EntityConfig 实体配置资源，用于定义生成单位的属性和行为
 * @param Count 要生成的单位数量
 * @param Position 单位生成的世界坐标位置
 * @return 返回新生成单位的句柄，用于后续操作和管理
 */
FUnitHandle URTSFormationSubsystem::SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count,
	const FVector& Position)
{
	return SpawnUnit(EntityConfig->GetConfig(), Count, Position);
}

/**
 * @brief 根据实体配置生成指定数量的单位
 * @param EntityConfig 实体配置信息，包含单位的具体配置数据
 * @param Count 要生成的单位数量
 * @param Position 单位生成的世界坐标位置
 * @return 返回新生成单位的句柄，用于后续操作和管理
 */
FUnitHandle URTSFormationSubsystem::SpawnUnit(const FMassEntityConfig& EntityConfig, int Count, const FVector& Position)
{
	// 创建一个新的单位句柄
	auto UnitHandle = FUnitHandle();

	// 根据配置信息和数量生成实体
	SpawnEntities(UnitHandle, EntityConfig, Count);
	return UnitHandle;
}

void URTSFormationSubsystem::SetFormationPreset(const FUnitHandle& UnitHandle, UFormationPresets* FormationAsset)
{
	if (!ensure(FormationAsset)) { return; }

	////// @todo fix this

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	EntityManager.ForEachSharedFragmentConditional<FUnitFragment>(
		[&UnitHandle](FUnitFragment& UnitFragment)
		{
			return UnitHandle == UnitFragment.UnitHandle;
		},
		[&EntityManager, &FormationAsset, &UnitHandle](FUnitFragment& UnitFragment)
		{
            UnitFragment.UnitSettings.bHollow = FormationAsset->bHollow;
            UnitFragment.UnitSettings.FormationLength = FormationAsset->FormationLength;
            UnitFragment.UnitSettings.BufferDistance = FormationAsset->BufferDistance;
            UnitFragment.UnitSettings.Formation = FormationAsset->Formation;
            UnitFragment.UnitSettings.Rings = FormationAsset->Rings;
		});
		
	UpdateUnitPosition(UnitHandle);
	//auto& UnitSettings = EntityManager.GetSharedFragmentDataChecked<FUnitSettings>(UnitHandle);
	//auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);

	//UnitSettings.FormationLength = FormationAsset->FormationLength;
	//UnitSettings.BufferDistance = FormationAsset->BufferDistance;
	//UnitSettings.Formation = FormationAsset->Formation;
	//UnitSettings.Rings = FormationAsset->Rings;
	//UnitSettings.bHollow = FormationAsset->bHollow;

	//SetUnitPosition( UnitFragment.UnitPosition, UnitHandle);
	
}

/**
 * @brief 为指定单位计算新的编队位置
 * 
 * 根据单位的编队设置（矩形、圆形等），计算出所有单位成员应该占据的新位置。
 * 支持实心和空心编队样式，并考虑缓冲距离等因素。
 * 
 * @param UnitFragment 单位片段引用，包含编队设置参数如编队类型、长度、缓冲距离等
 * @param Count 需要计算位置的单位成员数量
 * @param OutNewPositions 输出参数，存储计算得到的新位置列表
 */
void URTSFormationSubsystem::CalculateNewPositions(FUnitFragment& UnitFragment,
	int Count, TArray<FVector3f>& OutNewPositions)
{
	// 清空旧的位置数据，为新计算结果预留空间
	OutNewPositions.Empty(Count);
	auto& UnitSettings = UnitFragment.UnitSettings;

	// 计算编队的中心偏移量，用于居中编队布局
	const FVector3f CenterOffset = FVector3f((Count / UnitSettings.FormationLength / 2) * UnitSettings.BufferDistance,
		(UnitSettings.FormationLength / 2) * UnitSettings.BufferDistance, 0.f);
	
	int PlacedUnits = 0;  // 已放置的单位计数
	int PosIndex = 0;     // 位置索引
	
	// 循环直到所有单位都被放置到编队中
	while (PlacedUnits < Count)
	{
		// 计算当前索引在网格中的行列位置
		float w = PosIndex / UnitSettings.FormationLength;  // 行号
		float l = PosIndex % UnitSettings.FormationLength;  // 列号

		// 处理空心矩形编队逻辑（两层结构）
		if (UnitSettings.bHollow && UnitSettings.Formation == EFormationType::Rectangle)
		{
			int Switch = Count - UnitSettings.FormationLength * 2;
			// 跳过内部位置，只保留外围两层
			if (w != 0 && w != 1 && !(PlacedUnits >= Switch)
				&& l != 0 && l != 1 && l != UnitSettings.FormationLength - 1 && l != UnitSettings.FormationLength - 2)
			{
				PosIndex++;
				continue;
			}
		}

		// 处理圆形编队逻辑
		if (UnitSettings.Formation == EFormationType::Circle)
		{
			int AmountPerRing = Count / UnitSettings.Rings;           // 每圈单位数量
			float Angle = PosIndex * PI * 2 / AmountPerRing;         // 当前单位的角度
			float Radius = UnitSettings.FormationLength + (PosIndex / AmountPerRing * 1.5f);  // 当前圈的半径
			w = FMath::Cos(Angle) * Radius;  // 计算X坐标
			l = FMath::Sin(Angle) * Radius;  // 计算Y坐标
		}

		PlacedUnits++;  // 增加已放置单位计数
		
		// 创建基础位置并向量并应用缓冲距离
		FVector3f Position = FVector3f(w, l, 0.f);
		Position *= UnitSettings.BufferDistance;
		
		// 对矩形编队进行位置调整以确保正确对齐
		if (UnitSettings.Formation == EFormationType::Rectangle)
		{
			FVector3f FrontOffset = CenterOffset;
			FrontOffset.X = 0.f;
			Position -= FrontOffset;
		}

		// 旋转180度以确保单位位于正确位置
		Position = Position.RotateAngleAxis(180.f, FVector3f(0.f, 0.f, 1.f));

		// 将计算出的位置添加到结果列表中
		OutNewPositions.Add(Position);
		
		PosIndex++;  // 增加位置索引
	}
}

/**
 * @brief 为指定单位创建实体查询条件
 * 
 * 配置查询以筛选出属于特定单位的所有实体，用于对该单位的实体进行批量操作。
 * 
 * @param UnitHandle 目标单位的句柄
 * @param EntityQuery 实体查询对象引用，将被配置为筛选指定单位的实体
 */
void URTSFormationSubsystem::CreateQueryForUnit(const FUnitHandle& UnitHandle, FMassEntityQuery& EntityQuery)
{
	// 添加对单位片段的只读需求，用于识别实体所属单位
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	
	// 添加对RTS编队代理片段的读写需求，用于修改编队相关信息
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
	
	// 设置块过滤器，只处理属于指定单位的实体块
	EntityQuery.SetChunkFilter([&UnitHandle](const FMassExecutionContext& Context)
		{
			// 获取当前块的单位片段
			auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();
			
			// 返回当前块是否属于目标单位
			return UnitFragment.UnitHandle == UnitHandle;
		});
}


