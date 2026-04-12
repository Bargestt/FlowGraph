// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/Route/FlowNode_PostStart.h"

#include "FlowAsset.h"


UFlowNode_PostStart::UFlowNode_PostStart()
{
#if WITH_EDITOR
	Category = TEXT("Route");
	NodeDisplayStyle = FlowNodeStyle::InOut;
#endif

	InputPins.Reset();
	OutputPins = { UFlowNode::DefaultOutputPin };
}

void UFlowNode_PostStart::InitializeInstance()
{
	Super::InitializeInstance();
	
	if (const UWorld* World = GetWorld())
	{
		TimerHandle = World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::Trigger);
	}
}

void UFlowNode_PostStart::DeinitializeInstance()
{
	Super::DeinitializeInstance();
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UFlowNode_PostStart::OnLoad_Implementation()
{
	//Must be saved to avoid retrigger on load
	ActivationState = EFlowNodeState::Active;	
}

void UFlowNode_PostStart::Trigger()
{
	if (const UFlowAsset* FlowAsset = GetFlowAsset())
	{
		if (const UFlowNode* DefaultEntryNode = FlowAsset->GetDefaultEntryNode())
		{
			if (DefaultEntryNode->GetActivationState() == EFlowNodeState::NeverActivated)
			{
				if (const UWorld* World = GetWorld())
				{
					TimerHandle = World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::Trigger);
				}
				return;
			}
		}
	}
	
	if (GetActivationState() == EFlowNodeState::NeverActivated)
	{
		ActivationState = EFlowNodeState::Active;
		TriggerFirstOutput(false);
	}	
}
