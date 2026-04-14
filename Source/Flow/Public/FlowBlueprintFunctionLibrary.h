// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlowBlueprintFunctionLibrary.generated.h"

class UFlowComponent;
class UFlowNode_SubGraph;
class UFlowAsset;
class UFlowNode;
/**
 * 
 */
UCLASS()
class FLOW_API UFlowBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "FlowNode", meta=(DisplayName="Get Activation State", DefaultToSelf="FlowNode"))
	static EFlowNodeState FlowNode_GetActivationState(const UFlowNode* FlowNode);

	UFUNCTION(BlueprintPure, Category = "FlowNode", meta=(DisplayName="Is Active", DefaultToSelf="FlowNode"))
	static bool FlowNode_IsActive(const UFlowNode* FlowNode)
	{
		return FlowNode_GetActivationState(FlowNode) == EFlowNodeState::Active;
	}

	UFUNCTION(BlueprintCallable, Category = "FlowNode", meta=(DisplayName="Switch Active", DefaultToSelf="FlowNode", ExpandBoolAsExecs="ReturnValue"))
	static bool FlowNode_SwitchActive(const UFlowNode* FlowNode)
	{
		return FlowNode_GetActivationState(FlowNode) == EFlowNodeState::Active;
	}
	
	
	UFUNCTION(BlueprintPure, Category = "Flow", meta=(DisplayName ="Get Node Owning This Asset Instance"))
	static UFlowNode_SubGraph* FlowAsset_GetNodeOwningThisAssetInstance(const UFlowAsset* FlowAsset);
	
	UFUNCTION(BlueprintPure, Category = "Flow", meta=(DisplayName ="Get Parent Instance"))
	static UFlowAsset* FlowAsset_GetParentInstance(const UFlowAsset* FlowAsset);
};


UCLASS(meta=(RestrictedToClasses="FlowComponent"))
class UFlowComponentFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
};