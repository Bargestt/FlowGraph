// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/Actor/FlowNode_NotifyActor.h"
#include "FlowComponent.h"
#include "FlowSubsystem.h"
#include "FlowVersion.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_NotifyActor)

UFlowNode_NotifyActor::UFlowNode_NotifyActor()
	: NetMode(EFlowNetMode::Authority)
	, MatchType(EGameplayContainerMatchType::All)
	, bExactMatch(true)
{
#if WITH_EDITOR
	Category = TEXT("Actor");
#endif
	
	IdentityTags.IdentityMatchType = EFlowTagContainerMatchType::HasAllExact;
}

void UFlowNode_NotifyActor::PostLoad()
{
	Super::PostLoad();
	
	if (GetLinkerCustomVersion(FFlowVersion::GUID) < FFlowVersion::FlowIdentity)
	{		
		if (MatchType == EGameplayContainerMatchType::All)
		{
			IdentityTags.IdentityMatchType = bExactMatch ? EFlowTagContainerMatchType::HasAllExact : EFlowTagContainerMatchType::HasAll;
		}
		else
		{
			IdentityTags.IdentityMatchType = bExactMatch ? EFlowTagContainerMatchType::HasAnyExact : EFlowTagContainerMatchType::HasAny;
		}
		
		MatchType = EGameplayContainerMatchType::All;
		bExactMatch = true;
	}
}

void UFlowNode_NotifyActor::ExecuteInput(const FName& PinName)
{
	if (const UFlowSubsystem* FlowSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFlowSubsystem>())
	{
		for (const TWeakObjectPtr<UFlowComponent>& Component : FlowSubsystem->GetFlowComponentsByIdentity(IdentityTags))
		{
			Component->NotifyFromGraph(NotifyTags, NetMode);
		}
	}

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFlowNode_NotifyActor::GetNodeDescription() const
{
	return GetIdentityTagsDescription(IdentityTags.IdentityTags) + LINE_TERMINATOR + GetNotifyTagsDescription(NotifyTags);
}

EDataValidationResult UFlowNode_NotifyActor::ValidateNode()
{
	if (!IdentityTags.IsValid())
	{
		ValidationLog.Error<UFlowNode>(*UFlowNode::MissingIdentityTag, this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
