// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowBlueprintFunctionLibrary.h"

#include "FlowAsset.h"
#include "FlowComponent.h"
#include "Nodes/FlowNode.h"

EFlowNodeState UFlowBlueprintFunctionLibrary::FlowNode_GetActivationState(const UFlowNode* FlowNode)
{
	if (FlowNode)
	{
		return FlowNode->GetActivationState();
	}
	return EFlowNodeState::Invalid;
}

UFlowNode_SubGraph* UFlowBlueprintFunctionLibrary::FlowAsset_GetNodeOwningThisAssetInstance(const UFlowAsset* FlowAsset)
{
	if (FlowAsset)
	{
		return FlowAsset->GetNodeOwningThisAssetInstance();
	}
	return nullptr;
}

UFlowAsset* UFlowBlueprintFunctionLibrary::FlowAsset_GetParentInstance(const UFlowAsset* FlowAsset)
{
	if (FlowAsset)
	{
		return FlowAsset->GetParentInstance();
	}
	return nullptr;
}

void UFlowComponentFunctionLibrary::NotifySelf(UFlowComponent* Component, const FGameplayTag NotifyTag)
{
	if (Component)
	{
		if (NotifyTag.IsValid() && Component->HasBegunPlay())
		{
			Component->OnNotify(Component, NotifyTag);
		}
	}
}
