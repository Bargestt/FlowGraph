// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MovieSceneFlowEventReceiverInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UMovieSceneFlowEventReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FLOW_API IMovieSceneFlowEventReceiverInterface
{
	GENERATED_BODY()
public:
	virtual void TriggerEvent(const FString& EventName) = 0;
};
