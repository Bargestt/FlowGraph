// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/Route/FlowNode_PostStart.h"

#include "FlowAsset.h"


UFlowNode_PostStart::UFlowNode_PostStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("Route");
	NodeStyle = EFlowNodeStyle::InOut;
#endif

	InputPins.Reset();
	OutputPins = { UFlowNode::DefaultOutputPin };
}

void UFlowNode_PostStart::InitializeInstance()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::Trigger);
	}
}

void UFlowNode_PostStart::ExecuteInput(const FName& PinName)
{
	
}

void UFlowNode_PostStart::OnLoad_Implementation()
{
	ActivationState = EFlowNodeState::Active;
}

void UFlowNode_PostStart::Trigger()
{
	if (GetActivationState() == EFlowNodeState::NeverActivated)
	{
		ActivationState = EFlowNodeState::Active;
		TriggerFirstOutput(false);
	}	
}
