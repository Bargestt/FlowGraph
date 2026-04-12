// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors
#pragma once

#include <Types/FlowIdentity.h>

#include "GameplayTagContainer.h"

#include "Nodes/FlowNode.h"
#include "FlowNode_NotifyActor.generated.h"

/**
 * Finds all Flow Components with matching Identity Tag and calls ReceiveNotify event on these components.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Notify Actor", Keywords = "event"))
class FLOW_API UFlowNode_NotifyActor : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_NotifyActor();

protected:
	UPROPERTY(EditAnywhere, Category = "Notify")
	FFlowIdentity IdentityTags;
	
	UPROPERTY(EditAnywhere, Category = "Notify")
	FGameplayTagContainer NotifyTags;

	UPROPERTY(EditAnywhere, Category = "Notify")
	EFlowNetMode NetMode;
	
private:
	// Deprecated
	UPROPERTY()
	EGameplayContainerMatchType MatchType;
	
	// Deprecated
	UPROPERTY()
	bool bExactMatch;
	
protected:
	virtual void PostLoad() override;
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};
