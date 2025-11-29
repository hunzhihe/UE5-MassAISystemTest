// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletTrait.h"
#include "MassEntityTemplateRegistry.h"

void UBulletTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{

	BuildContext.AddFragment(FConstStructView::Make(BulletFragment));
	BuildContext.AddTag<FBulletTag>();
}
