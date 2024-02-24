// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#pragma once

#include "GameplayTagContainer.h"

#include "Nodes/FlowNode.h"
#include "FlowNode_ComponentObserver.generated.h"

class UFlowComponent;

/**
 * Base class for nodes operating on actors with the Flow Component
 * Such nodes usually wait until a specific action occurs in the actor
 */
UCLASS(Abstract)
class FLOW_API UFlowNode_ComponentObserver : public UFlowNode
{
	GENERATED_UCLASS_BODY()
	
	friend class FFlowNode_ComponentObserverDetails;

protected:
	UPROPERTY(EditAnywhere, Category = "ObservedComponent", meta = (Categories = "Flow"))
	FGameplayTagContainer IdentityTags;

	// Container A: Identity Tags in Flow Component
	// Container B: Identity Tags listed above
	UPROPERTY(EditAnywhere, Category = "ObservedComponent")
	EFlowTagContainerMatchType IdentityMatchType;

	// This node will become Completed, if Success Limit > 0 and Success Count reaches this limit
	// Set this to zero, if you'd like receive events indefinitely (node would finish work only if explicitly Stopped)
	UPROPERTY(EditAnywhere, Category = "Lifetime", meta = (ClampMin = 0))
	int32 SuccessLimit;

	// This node will become Completed, if Success Limit > 0 and Success Count reaches this limit
	UPROPERTY(VisibleAnywhere, Category = "Lifetime", SaveGame)
	int32 SuccessCount;

	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UFlowComponent>> RegisteredActors;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	virtual void StartObserving();

	/** Automatically called on Cleanup */
	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	virtual void StopObserving();

	UFUNCTION()
	virtual void OnComponentRegistered(UFlowComponent* Component);

	UFUNCTION()
	virtual void OnComponentTagAdded(UFlowComponent* Component, const FGameplayTagContainer& AddedTags);

	UFUNCTION()
	virtual void OnComponentTagRemoved(UFlowComponent* Component, const FGameplayTagContainer& RemovedTags);

	UFUNCTION()
	virtual void OnComponentUnregistered(UFlowComponent* Component);

	virtual void ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component);
	virtual void ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component);

	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ObserveActor"))
	void ReceiveObserveActor(AActor* Actor, UFlowComponent* Component, bool& bRegisterComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ForgetActor"))
	bool ReceiveForgetActor(AActor* Actor, UFlowComponent* Component);



	UFUNCTION(BlueprintCallable, Category = "ComponentObserver", meta = (DisplayName = "On Success"))
	virtual void OnEventReceived();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ComponentObserver", meta = (DeterminesOutputType = "Class"))
	TArray<AActor*> GetRegisteredActors(TSubclassOf<AActor> Class) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ComponentObserver", meta = (DeterminesOutputType = "Class"))
	TArray<UFlowComponent*> GetRegisteredComponents(TSubclassOf<UFlowComponent> Class) const;

	virtual void Cleanup() override;

#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	virtual FString GetStatusString() const override;
#endif
};

