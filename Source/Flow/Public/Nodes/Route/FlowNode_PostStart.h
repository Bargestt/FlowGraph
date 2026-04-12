// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_PostStart.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Post Start", Keywords = "post start"))
class FLOW_API UFlowNode_PostStart : public UFlowNode
{
	GENERATED_BODY()
	
protected:
	FTimerHandle TimerHandle;
	
public:
	UFlowNode_PostStart();
	virtual void InitializeInstance() override;
	virtual void DeinitializeInstance() override;
protected:
	virtual void OnLoad_Implementation() override;
	
public:
#if WITH_EDITOR
	virtual FString GetStatusString() const override { return UEnum::GetValueAsString(GetActivationState()); }
#endif //
		
protected:
	void Trigger();
};
