// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "FlowTypes.h"
#include "FlowComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowTypes)



bool FFlowIdentity::Matches(const AActor* Actor, const UFlowComponent* Component) const
{
	return
		!IdentityTags.IsEmpty() &&
		(!ActorFilter || Actor->IsA(ActorFilter.Get())) &&
		(!ComponentFilter || Component->IsA(ComponentFilter.Get())) &&
		FlowTypes::HasMatchingTags(Component->IdentityTags, IdentityTags, IdentityMatchType);
}
