// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"

#include "NiagaraSystem.h"
#include "MSNiagaraRepresentationTrait.generated.h"

/**
 * 
 */
/**
 * @brief Niagara表示特征类，用于定义实体的Niagara视觉表现
 * 
 * 该类继承自UMassEntityTraitBase，提供了基于Niagara系统的实体可视化功能，
 * 允许配置共享的Niagara系统、静态网格和材质覆盖等视觉属性。
 */
UCLASS(meta = (DisplayName = "Niagara Representation"))
class MASSNIAGARAVISUALIZATION_API UMSNiagaraRepresentationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	public:
	/**
	 * @brief 构建实体模板的方法
	 * 
	 * 重写基类方法，用于构建具有Niagara表示功能的实体模板。
	 * 在构建过程中会添加必要的片段和处理器来支持Niagara系统的渲染。
	 * 
	 * @param BuildContext 实体模板构建上下文，包含构建过程中需要的各种信息和工具
	 * @param World 当前的游戏世界引用
	 */
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,const UWorld& World) const override;

	/** 可编辑的Niagara系统资源引用，用于定义实体的粒子效果表现 */
	UPROPERTY(EditAnywhere, Category = "Config")
	TSoftObjectPtr<UNiagaraSystem> SharedNiagaraSystem;

	/** 可编辑的静态网格资源引用，用于定义实体的基础网格形状 */
	UPROPERTY(EditAnywhere, Category = "Config")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** 可编辑的材质接口资源引用，用于覆盖默认材质表现 */
	UPROPERTY(EditAnywhere, Category = "Config")
	TSoftObjectPtr<UMaterialInterface> MaterialOverride;
};
