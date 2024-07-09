// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#pragma once

#include "Nodes/World/FlowNode_ComponentObserver.h"
#include "FlowNode_ComponentObserver_BlueprintBase.generated.h"

class UFlowComponent;

/**
 * Base class for nodes operating on actors with the Flow Component
 * Such nodes usually wait until a specific action occurs in the actor
 */
UCLASS(Abstract, Blueprintable, meta=(DisplayName="FlowNode_ComponentObserver_BP"))
class FLOW_API UFlowNode_ComponentObserver_BlueprintBase : public UFlowNode_ComponentObserver
{
	GENERATED_UCLASS_BODY()	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="FlowNode", meta=(GetOptions="GetOutputNames"))
	FName SuccessPin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="FlowNode", meta=(GetOptions="GetOutputNames"))
	FName CompletePin;

	
protected:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver", meta=(DisplayName="StartObserving"))
	virtual void BP_StartObserving();

	/** Automatically called on Cleanup */
	UFUNCTION(BlueprintCallable, Category = "ComponentObserver", meta=(DisplayName="StopObserving"))
	virtual void BP_StopObserving();


	virtual void ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;
	virtual void ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;
	virtual void OnEventReceived() override;
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ObserveActor"))
	void ReceiveObserveActor(AActor* Actor, UFlowComponent* Component, bool& bRegisterComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ForgetActor"))
	bool ReceiveForgetActor(AActor* Actor, UFlowComponent* Component);
	


	UFUNCTION(BlueprintCallable, Category = "ComponentObserver", meta = (DisplayName = "On Success"))
	void BP_OnSuccess();

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	int32 GetRegisteredNum() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ComponentObserver", meta = (DeterminesOutputType = "Class"))
	TArray<AActor*> GetRegisteredActors(TSubclassOf<AActor> Class) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ComponentObserver", meta = (DeterminesOutputType = "Class"))
	TArray<UFlowComponent*> GetRegisteredComponents(TSubclassOf<UFlowComponent> Class) const;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	FGameplayTagContainer GetIdentityTags() const;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	EFlowTagContainerMatchType GetIdentityMatchType() const;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	int32 GetSuccessCount() const;

	UFUNCTION(BlueprintCallable, Category = "ComponentObserver")
	int32 GetSuccessLimit() const;

#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
	virtual FString GetStatusString() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

