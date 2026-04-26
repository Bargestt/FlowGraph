// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Types/FlowIdentity.h"

#include "FlowComponent.h"
#include "FlowSettings.h"


bool FFlowIdentity::Matches(const AActor* Actor, const UFlowComponent* Component) const
{
	return
		Actor && Component && 
		!IdentityTags.IsEmpty() &&
		(!ActorFilter || Actor->IsA(ActorFilter.Get())) &&
		(!ComponentFilter || Component->IsA(ComponentFilter.Get())) &&
		FlowTypes::HasMatchingTags(Component->IdentityTags, IdentityTags, IdentityMatchType);
}

bool FFlowIdentity::MatchesChecked(const AActor* Actor, const UFlowComponent* Component) const
{
	return
		(!ActorFilter || Actor->IsA(ActorFilter.Get())) &&
		(!ComponentFilter || Component->IsA(ComponentFilter.Get())) &&
		FlowTypes::HasMatchingTags(Component->IdentityTags, IdentityTags, IdentityMatchType);;
}

bool FFlowIdentity::Matches(const UFlowComponent* Component) const
{
	if (Component && IsValid())
	{
		const AActor* Actor = Component->GetOwner();
		return Actor && Matches(Actor, Component);
	}
	
	return false;
}

bool FFlowIdentity::MatchesChecked(const UFlowComponent* Component) const
{
	const AActor* Actor = Component->GetOwner();
	return Actor && MatchesChecked(Actor, Component);
}

bool FFlowIdentity::Matches(const AActor* Actor) const
{
	if (Actor && IsValid())
	{
		TInlineComponentArray<UFlowComponent*> FlowComps(Actor);
		for (const UFlowComponent* Comp : FlowComps)
		{
			if (Matches(Actor, Comp))
			{
				return true;
			}
		}
	}
	return false;
}

bool FFlowIdentity::MatchesTags(const FGameplayTagContainer& Tags) const
{
	return !IdentityTags.IsEmpty() && FlowTypes::HasMatchingTags(Tags, IdentityTags, IdentityMatchType);
}

bool FFlowIdentity::MatchesFilters(const AActor* Actor, const UFlowComponent* Component) const
{
	return 
		(!ActorFilter || Actor->IsA(ActorFilter.Get())) &&
		(!ComponentFilter || Component->IsA(ComponentFilter.Get()));
}

FString FFlowIdentity::ToString(bool bShortNames, bool bIncludeMatchType, bool bIncludeClassFilters, FString Separator) const
{
	if (IdentityTags.IsEmpty())
	{
		return TEXT("None");
	}
	
	FString Result;
	
	TArray<FString> RemovePrefix;
	if (bShortNames)
	{
		const UFlowSettings* Settings = GetDefault<UFlowSettings>();
		for (auto& Tag : Settings->RemoveIdentityTagRoots)
		{
			RemovePrefix.Add(Tag.ToString() + TEXT("."));
		}
	}
	
	if (bIncludeMatchType)
	{
		FString MatchName = UEnum::GetValueAsString(IdentityMatchType);
		MatchName.Split(TEXT("::"), nullptr, &MatchName);
		Result += MatchName + Separator;
	}
	
	Result += FString::JoinBy(IdentityTags, *Separator, [&RemovePrefix, bShortNames](const FGameplayTag& Tag)
	{
		if (!bShortNames)
		{
			return Tag.ToString();
		}		
		
		FString TagName = Tag.ToString();
		for (auto& Prefix : RemovePrefix)
		{
			if (TagName.RemoveFromStart(Prefix))
			{
				break;
			}
		}
		return TagName;
	});

	if (bIncludeClassFilters)
	{
		if (ComponentFilter)
		{
			Result += Separator + ComponentFilter->GetName();
		}
		
		if (ActorFilter)
		{
			Result += Separator + ActorFilter->GetName();
		}
	}
	
	return Result;
}

bool FFlowIdentity::SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot)
{	
	if (Tag.GetType().IsStruct(FGameplayTagContainer::StaticStruct()->GetFName()))
	{
		IdentityTags.Serialize(Slot);
		return true;
	}
	if (Tag.GetType().IsStruct(FGameplayTag::StaticStruct()->GetFName()))
	{
		FGameplayTag GameplayTag;
		FGameplayTag::StaticStruct()->SerializeItem(Slot, &GameplayTag, nullptr);
		IdentityTags.AddTag(GameplayTag);	
		return true;
	}
	return false;
}


bool UFlowIdentityBlueprintFunctionLibrary::FlowIdentityValid(const FFlowIdentity& Identity)
{
	return Identity.IsValid();
}

bool UFlowIdentityBlueprintFunctionLibrary::FlowIdentityMatches(const FFlowIdentity& Identity, const AActor* Actor, const UFlowComponent* Component)
{
	return Identity.Matches(Actor, Component);
}

bool UFlowIdentityBlueprintFunctionLibrary::FlowIdentityMatchesActor(const FFlowIdentity& Identity, const AActor* Actor)
{	
	return Identity.Matches(Actor);
}

bool UFlowIdentityBlueprintFunctionLibrary::FlowIdentityMatchesComponent(const FFlowIdentity& Identity, const UFlowComponent* Component)
{
	return Identity.Matches(Component);
}

bool UFlowIdentityBlueprintFunctionLibrary::FlowIdentityMatchesTags(const FFlowIdentity& Identity, const FGameplayTagContainer& Tags)
{
	return Identity.MatchesTags(Tags);
}

FString UFlowIdentityBlueprintFunctionLibrary::FlowIdentityToString(const FFlowIdentity& Identity, bool bShortNames, bool bIncludeMatchType, bool bIncludeClassFilters, FString Separator)
{
	return Identity.ToString(bShortNames, bIncludeMatchType, bIncludeClassFilters, Separator);
}
