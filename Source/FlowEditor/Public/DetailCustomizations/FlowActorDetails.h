// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorDetailsDelegates.h"

/**
 * 
 */
class FLOWEDITOR_API FFlowActorDetails : public TSharedFromThis<FFlowActorDetails>
{
public:	
	virtual ~FFlowActorDetails();
	virtual void Register();
	virtual void AddFlowCategory(class IDetailLayoutBuilder& Details, const FGetSelectedActors& GetSelectedActors);
};
