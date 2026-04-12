// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "FlowTypes.h"
#include "FlowIdentity.generated.h"


USTRUCT(BlueprintType)
struct FLOW_API FFlowIdentity
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FGameplayTagContainer IdentityTags;

	// Container A: Identity Tags in Flow Component
	// Container B: Identity Tags listed above
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EFlowTagContainerMatchType IdentityMatchType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Identity")
	TSubclassOf<class UFlowComponent> ComponentFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Identity")
	TSubclassOf<class AActor> ActorFilter;

	FFlowIdentity()
		: IdentityMatchType(EFlowTagContainerMatchType::HasAnyExact)
	{		
	}
	
	FORCEINLINE bool IsValid() const { return !IdentityTags.IsEmpty(); }	
	FORCEINLINE bool IsExactMatch() const { return IdentityMatchType == EFlowTagContainerMatchType::HasAnyExact || IdentityMatchType == EFlowTagContainerMatchType::HasAllExact; }
	FORCEINLINE EGameplayContainerMatchType GetContainerMatchType() const
	{
		return (IdentityMatchType == EFlowTagContainerMatchType::HasAny || IdentityMatchType == EFlowTagContainerMatchType::HasAnyExact) ? EGameplayContainerMatchType::Any : EGameplayContainerMatchType::All;
	}
	
	bool Matches(const AActor* Actor, const UFlowComponent* Component) const;
	bool MatchesChecked(const AActor* Actor, const UFlowComponent* Component) const;
	bool Matches(const UFlowComponent* Component) const;
	bool MatchesChecked(const UFlowComponent* Component) const;
	bool Matches(const AActor* Actor) const;
	
	bool MatchesTags(const FGameplayTagContainer& Tags) const;
	bool MatchesFilters(const AActor* Actor, const UFlowComponent* Component) const;
	
	FString ToString(bool bShortNames = false, bool bIncludeMatchType = false, bool bIncludeClassFilters = false, FString Separator = TEXT("\n")) const;
	
	
	bool SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot);
};

template<>
struct TStructOpsTypeTraits< FFlowIdentity > : public TStructOpsTypeTraitsBase2< FFlowIdentity >
{
	enum
	{		
		WithStructuredSerializeFromMismatchedTag = true,
	};
};


UCLASS()
class FLOW_API UFlowIdentityBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category=Flow)
	static bool FlowIdentityValid(const FFlowIdentity& Identity);	

	/** Identity Valid and matches Actor and Flow component */
	UFUNCTION(BlueprintPure, Category=Flow)
	static bool FlowIdentityMatches(const FFlowIdentity& Identity, const AActor* Actor, const UFlowComponent* Component);

	/** Identity Valid and matches Actor and any Flow component in actor */
	UFUNCTION(BlueprintPure, Category=Flow)
	static bool FlowIdentityMatchesActor(const FFlowIdentity& Identity, const AActor* Actor);

	/** Identity Valid and matches Owning Actor and Flow component */
	UFUNCTION(BlueprintPure, Category=Flow)
	static bool FlowIdentityMatchesComponent(const FFlowIdentity& Identity, const UFlowComponent* Component);

	/** Identity Valid and matches tags */
	UFUNCTION(BlueprintPure, Category=Flow)
	static bool FlowIdentityMatchesTags(const FFlowIdentity& Identity, const FGameplayTagContainer& Tags);

	UFUNCTION(BlueprintPure, Category=Flow, meta=(DisplayName = "ToString(Identity)"))
	static FString FlowIdentityToString(const FFlowIdentity& Identity, bool bShortNames = false, bool bIncludeMatchType = false, bool bIncludeClassFilters = false, FString Separator = TEXT("\n"));
	
};
