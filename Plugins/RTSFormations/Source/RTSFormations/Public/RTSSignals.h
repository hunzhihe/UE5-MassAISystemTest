// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace RTS::Unit::Signals
{
	// 信号名称常量定义命名空间
	// 用于定义单位相关的信号标识符
	
	// 更新单位位置信号
	// 当单位位置发生变化时触发此信号
	const FName UpdateUnitPosition = FName(TEXT("UpdateUnitPosition"));
	
	// 编队更新信号
	// 当单位编队信息发生变更时触发此信号
	const FName FormationUpdated = FName(TEXT("FormationUpdated"));
	
	// 更新索引信号
	// 当需要更新单位索引信息时触发此信号
	const FName UpdateIndex = FName(TEXT("UpdateIndex"));
}

