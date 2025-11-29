// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletHellEnemyTrait.h"

#include "MassEntityTemplateRegistry.h"

void UBulletHellEnemyTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment(FConstStructView::Make(BHEnemyFragment));
	BuildContext.AddTag<FBHEnemyTag>();
}
