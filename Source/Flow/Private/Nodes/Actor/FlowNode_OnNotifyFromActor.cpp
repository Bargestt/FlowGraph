// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/Actor/FlowNode_OnNotifyFromActor.h"
#include "FlowComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_OnNotifyFromActor)

UFlowNode_OnNotifyFromActor::UFlowNode_OnNotifyFromActor()
	: bRetroactive(false)
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Condition;
#endif
}

void UFlowNode_OnNotifyFromActor::ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	if (!RegisteredActors.Contains(Actor))
	{
		RegisteredActors.Emplace(Actor, Component);
		Component->OnNotifyFromComponent.AddUObject(this, &UFlowNode_OnNotifyFromActor::OnNotifyFromComponent);

		if (bRetroactive && Component->GetRecentlySentNotifyTags().HasAnyExact(NotifyTags))
		{
			OnEventReceived();
		}
	}
}

void UFlowNode_OnNotifyFromActor::ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	Component->OnNotifyFromComponent.RemoveAll(this);
}

void UFlowNode_OnNotifyFromActor::OnNotifyFromComponent(UFlowComponent* Component, const FGameplayTag& Tag)
{	
	if (IdentityTags.MatchesTags(Component->IdentityTags))
	{
		if (!NotifyTags.IsValid())
		{
			OnEventReceived();
		}
		else if(bExactTag && NotifyTags.HasTagExact(Tag))
		{
			OnEventReceived();
		}		
		else if(!bExactTag)
		{
			for (FGameplayTag TestTag = Tag; TestTag.IsValid(); TestTag = TestTag.RequestDirectParent())
			{
				if (NotifyTags.HasTagExact(TestTag))
				{
					OnEventReceived();
					break;
				}
			}
		}
	}
}

#if WITH_EDITOR
FString UFlowNode_OnNotifyFromActor::GetNodeDescription() const
{
	return GetIdentityTagsDescription(IdentityTags.IdentityTags) + LINE_TERMINATOR + GetNotifyTagsDescription(NotifyTags);
}
#endif
