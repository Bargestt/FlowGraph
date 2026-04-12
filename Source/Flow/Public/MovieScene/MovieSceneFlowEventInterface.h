// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MovieSceneFlowEventInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UMovieSceneFlowEventInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FLOW_API IMovieSceneFlowEventInterface
{
	GENERATED_BODY()
public:
	virtual TArrayView<FString> GetAllEntryPoints() = 0;
};
