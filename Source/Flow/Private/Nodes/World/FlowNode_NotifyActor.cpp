// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/World/FlowNode_NotifyActor.h"
#include "FlowComponent.h"
#include "FlowSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_NotifyActor)

UFlowNode_NotifyActor::UFlowNode_NotifyActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NetMode(EFlowNetMode::Authority)
{
#if WITH_EDITOR
	Category = TEXT("Notifies");
#endif

	Identity.IdentityMatchType = EFlowTagContainerMatchType::HasAllExact;
}

void UFlowNode_NotifyActor::ExecuteInput(const FName& PinName)
{
	if (const UFlowSubsystem* FlowSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFlowSubsystem>())
	{
		TSet<UFlowComponent*> FlowComponents = FlowSubsystem->GetFlowComponentsByIdentity(Identity);
		for (const TWeakObjectPtr<UFlowComponent>& Component : FlowComponents)
		{
			Component->NotifyFromGraph(NotifyTags, NetMode);
		}
	}

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFlowNode_NotifyActor::GetNodeDescription() const
{
	return GetIdentityTagsDescription(Identity.IdentityTags) + LINE_TERMINATOR + GetNotifyTagsDescription(NotifyTags);
}

EDataValidationResult UFlowNode_NotifyActor::ValidateNode()
{
	if (Identity.IdentityTags.IsEmpty())
	{
		ValidationLog.Error<UFlowNode>(*UFlowNode::MissingIdentityTag, this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
