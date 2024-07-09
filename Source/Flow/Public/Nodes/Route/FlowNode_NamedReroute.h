// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowAsset.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_NamedReroute.generated.h"




/**
 * Executes all matching nodes
 */
UCLASS(NotBlueprintable, meta=(DisplayName = "Named Reroute"))
class FLOW_API UFlowNode_NamedReroute : public UFlowNode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Node")
	bool bIsInput;
	
	UPROPERTY(EditAnywhere, Category="Node", meta=(EditCondition = "bIsInput"))
	FLinearColor Color;
	
	UPROPERTY(EditAnywhere, Category="Node", meta=(EditCondition = "bIsInput", EditConditionHides, DisplayName = "Name"))
	FName InputName;

	UPROPERTY(EditAnywhere, Category="Node", meta=(EditCondition = "!bIsInput", EditConditionHides, DisplayName = "Name", GetOptions="GetOutputNameOptions"))
	FName OutputName;

	UPROPERTY(VisibleAnywhere, Category="Node")
	TArray<FGuid> TargetNodes;

public:
	UFlowNode_NamedReroute(const FObjectInitializer& ObjectInitializer);

protected:
	void RebuildPins();
	
	FORCEINLINE bool IsInput() const { return bIsInput; }
	FORCEINLINE bool IsOutput() const { return !bIsInput; }
	FORCEINLINE const FName& GetRerouteName() const { return IsInput() ? InputName : OutputName; }
	
protected:
	virtual void ExecuteInput(const FName& PinName) override;
#if WITH_EDITOR
	static void UpdateReroutes(UFlowNode_NamedReroute* Changed, UFlowAsset* Asset);
	
	UFUNCTION()
	TArray<FName> GetOutputNameOptions() const;
	
  	virtual FText GetNodeTitle() const override;
	virtual bool GetDynamicTitleColor(FLinearColor& OutColor) const override;
	virtual FString GetNodeDescription() const override;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

