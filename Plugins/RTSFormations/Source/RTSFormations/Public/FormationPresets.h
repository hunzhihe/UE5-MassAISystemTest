// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FormationPresets.generated.h"

//枚举，定义了两种编队类型 ，矩形和圆形
UENUM()
enum class EFormationType : uint8
{
	Rectangle,
	Circle
};
/**
 * 
 */

//数据资产，用于存储可配置的编队参数
UCLASS(Blueprintable)
class RTSFORMATIONS_API UFormationPresets : public UDataAsset
{
	GENERATED_BODY()
	
public:

	//编队类型（默认矩形）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFormationType Formation = EFormationType::Rectangle;

	// 编队长度（默认8）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FormationLength = 8;

	//编队单位间距（默认100）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BufferDistance = 100.f;

	// 圆形编队环数（仅对圆形编队有效，默认2）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Rings = 2;

	//圆形编队是否为空心
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHollow = false;
};
