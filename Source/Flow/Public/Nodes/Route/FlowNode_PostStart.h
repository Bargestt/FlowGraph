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
	GENERATED_UCLASS_BODY()
protected:
	virtual void InitializeInstance() override;
	virtual void ExecuteInput(const FName& PinName) override;
public:
	virtual void OnLoad_Implementation() override;

protected:
	void Trigger();

#if WITH_EDITOR
public:
	virtual FString GetStatusString() const override
	{
		return StaticEnum<EFlowNodeState>()->GetNameStringByValue((int64)GetActivationState());
	}
#endif //
};
