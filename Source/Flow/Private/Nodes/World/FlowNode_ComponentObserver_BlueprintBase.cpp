// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/World/FlowNode_ComponentObserver_BlueprintBase.h"
#include "FlowSubsystem.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_ComponentObserver_BlueprintBase)

UFlowNode_ComponentObserver_BlueprintBase::UFlowNode_ComponentObserver_BlueprintBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SuccessPin = TEXT("Success");
	CompletePin = TEXT("Completed");
	
}

void UFlowNode_ComponentObserver_BlueprintBase::ExecuteInput(const FName& PinName)
{
	K2_ExecuteInput(PinName);

	if (Identity.IsValid())
	{
		if (PinName == TEXT("Start"))
		{
			StartObserving();
		}
		else if (PinName == TEXT("Stop"))
		{
			TriggerOutput(TEXT("Stopped"), true);
		}
	}
	else
	{
		LogError(MissingIdentityTag);
	}
}

void UFlowNode_ComponentObserver_BlueprintBase::Cleanup()
{
	Super::Cleanup();
	K2_Cleanup();
}

void UFlowNode_ComponentObserver_BlueprintBase::BP_StartObserving()
{
	StartObserving();
}

void UFlowNode_ComponentObserver_BlueprintBase::BP_StopObserving()
{
	StopObserving();
}


void UFlowNode_ComponentObserver_BlueprintBase::ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		bool bRegisterComponent = false;
		ReceiveObserveActor(Actor.Get(), Component.Get(), bRegisterComponent);
		if (bRegisterComponent)
		{
			RegisteredActors.Add(Actor, Component);
		}
	}
}

void UFlowNode_ComponentObserver_BlueprintBase::ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	if (!HasAnyFlags(RF_BeginDestroyed) && !IsUnreachable() && (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native)))
	{
		ReceiveForgetActor(Actor.Get(), Component.Get());
	}
}


void UFlowNode_ComponentObserver_BlueprintBase::OnEventReceived()
{
	if (!SuccessPin.IsNone())
	{
		TriggerOutput(SuccessPin, false);
	}
	

	SuccessCount++;
	if (SuccessLimit > 0 && SuccessCount == SuccessLimit)
	{
		TriggerOutput(CompletePin, true);
	}
}

void UFlowNode_ComponentObserver_BlueprintBase::BP_OnSuccess()
{
	OnEventReceived();
}

int32 UFlowNode_ComponentObserver_BlueprintBase::GetRegisteredNum() const
{
	return RegisteredActors.Num();
}

TArray<AActor*> UFlowNode_ComponentObserver_BlueprintBase::GetRegisteredActors(TSubclassOf<AActor> Class) const
{
	TArray<AActor*> Array;
	for (const auto& Pair : RegisteredActors)
	{
		AActor* Actor = Pair.Key.Get();
		if (Actor && (!Class || Actor->IsA(Class)))
		{
			Array.Add(Actor);
		}
	}
	return Array;
}

TArray<UFlowComponent*> UFlowNode_ComponentObserver_BlueprintBase::GetRegisteredComponents(TSubclassOf<UFlowComponent> Class) const
{
	TArray<UFlowComponent*> Array;
	for (const auto& Pair : RegisteredActors)
	{
		UFlowComponent* Comp = Pair.Value.Get();
		if (Comp && (!Comp || Comp->IsA(Class)))
		{
			Array.Add(Comp);
		}
	}
	return Array;
}

FGameplayTagContainer UFlowNode_ComponentObserver_BlueprintBase::GetIdentityTags() const
{
	return Identity.IdentityTags;
}

EFlowTagContainerMatchType UFlowNode_ComponentObserver_BlueprintBase::GetIdentityMatchType() const
{
	return Identity.IdentityMatchType;
}

int32 UFlowNode_ComponentObserver_BlueprintBase::GetSuccessCount() const
{
	return SuccessCount;
}

int32 UFlowNode_ComponentObserver_BlueprintBase::GetSuccessLimit() const
{
	return SuccessLimit;
}

#if WITH_EDITOR
FString UFlowNode_ComponentObserver_BlueprintBase::GetNodeDescription() const
{
	FString Description = K2_GetNodeDescription();
	if (Description.IsEmpty())
	{
		Description += (SuccessLimit > 0) ? *FString::Printf(TEXT("%d times"), SuccessLimit) : TEXT("Infinite");
		Description += TEXT("\n") + GetIdentityTagsDescription(Identity.IdentityTags);		
	}
	return Description;
}

FString UFlowNode_ComponentObserver_BlueprintBase::GetStatusString() const
{
	FString Status = K2_GetStatusString();
	if (Status.IsEmpty())
	{
		if (ActivationState == EFlowNodeState::Active)
		{
			if (RegisteredActors.Num() == 0)
			{
				return NoActorsFound;
			}
			
			if (SuccessLimit > 0)
			{
				Status += FString::Printf(TEXT("%d / %d"), SuccessCount, SuccessLimit);
			}			
		}
	}
	return Status;
}

EDataValidationResult UFlowNode_ComponentObserver_BlueprintBase::IsDataValid(FDataValidationContext& Context) const
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UFlowNode_ComponentObserver_BlueprintBase, ReceiveObserveActor)))
		{
			Context.AddWarning(INVTEXT("Must Implement ObserveActor function to correctly register actors"));
			return EDataValidationResult::Valid;
		}
	}
	
	return Super::IsDataValid(Context);
}
#endif

