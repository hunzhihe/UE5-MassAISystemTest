// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * 声明一个外部日志分类变量，用于质量智能对象AI模块的日志记录
 * 
 * 该宏声明了一个名为LogMassSmartObjectAI的日志分类，其日志级别为Log，
 * 并且在All配置下可用。这个日志分类通常用于UE4/UE5项目中，专门记录
 * 与质量智能对象AI相关的日志信息。
 * 
 * 参数说明：
 * - LogMassSmartObjectAI: 日志分类的名称
 * - Log: 日志默认级别
 * - All: 在所有构建配置中都启用此日志分类
 */
DECLARE_LOG_CATEGORY_EXTERN(LogMassSmartObjectAI, Log, All);

class FMassSmartObjectAIModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
