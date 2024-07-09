// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_MultiTimer.generated.h"




/**
 * Timer that aggregates multiple delays in one
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Timer Multi", Keywords = "delay, tick"))
class FLOW_API UFlowNode_MultiTimer : public UFlowNode
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category="Timer", meta=(UIMin="0"))
	TArray<float> Events;

	TMap<int32, FTimerHandle> TimerHandles;

	UPROPERTY(SaveGame)
	TMap<int32, float> TimeLeft;

public:
	UFlowNode_MultiTimer(const FObjectInitializer& ObjectInitializer);
protected:	
	virtual void ExecuteInput(const FName& PinName) override;

	void SetTimers();
	void Restart();
	
private:
	UFUNCTION()
	void OnCompletion(int32 Index);

	void TriggerCompleted();

protected:
	virtual void Cleanup() override;
	virtual void OnSave_Implementation() override;
	virtual void OnLoad_Implementation() override;
	
#if WITH_EDITOR
	virtual bool SupportsContextPins() const override { return true; }
	virtual TArray<FFlowPin> GetContextOutputs() override;
	
	virtual FString GetNodeDescription() const override;
	virtual FString GetStatusString() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};




