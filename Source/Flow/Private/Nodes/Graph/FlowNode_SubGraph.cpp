// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/Graph/FlowNode_SubGraph.h"

#include "FlowAsset.h"
#include "FlowSettings.h"
#include "FlowSubsystem.h"
#include "Interfaces/FlowNodeWithExternalDataPinSupplierInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_SubGraph)

#define LOCTEXT_NAMESPACE "FlowNode_SubGraph"

FFlowPin UFlowNode_SubGraph::StartPin(TEXT("Start"));
FFlowPin UFlowNode_SubGraph::FinishPin(TEXT("Finish"));

UFlowNode_SubGraph::UFlowNode_SubGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bCanInstanceIdenticalAsset(false)
{
#if WITH_EDITOR
	Category = TEXT("Graph");
	NodeDisplayStyle = FlowNodeStyle::SubGraph;

	AllowedAssignedAssetClasses = {UFlowAsset::StaticClass()};
#endif

	InputPins = {StartPin};
	OutputPins = {FinishPin};
}

bool UFlowNode_SubGraph::CanBeAssetInstanced() const
{
	return !Asset.IsNull() && (bCanInstanceIdenticalAsset || Asset.ToString() != GetFlowAsset()->GetTemplateAsset()->GetPathName());
}

void UFlowNode_SubGraph::PreloadContent()
{
	if (CanBeAssetInstanced() && GetFlowSubsystem())
	{
		GetFlowSubsystem()->CreateSubFlow(this, FString(), true);
	}
}

void UFlowNode_SubGraph::FlushContent()
{
	if (CanBeAssetInstanced() && GetFlowSubsystem())
	{
		GetFlowSubsystem()->RemoveSubFlow(this, EFlowFinishPolicy::Abort);
	}
}

void UFlowNode_SubGraph::ExecuteInput(const FName& PinName)
{
	if (CanBeAssetInstanced() == false)
	{
		if (Asset.IsNull())
		{
			LogError(TEXT("Missing Flow Asset"));
		}
		else
		{
			LogError(FString::Printf(TEXT("Asset %s cannot be instance, probably is the same as the asset owning this SubGraph node."), *Asset.ToString()));
		}

		Finish();
		return;
	}

	if (PinName == TEXT("Start"))
	{
		if (GetFlowSubsystem())
		{
			GetFlowSubsystem()->CreateSubFlow(this);
		}
	}
	else if (!PinName.IsNone())
	{
		GetFlowAsset()->TriggerCustomInput_FromSubGraph(this, PinName);
	}
}

void UFlowNode_SubGraph::Cleanup()
{
	if (CanBeAssetInstanced() && GetFlowSubsystem())
	{
		GetFlowSubsystem()->RemoveSubFlow(this, EFlowFinishPolicy::Keep);
	}

	Super::Cleanup();
}

void UFlowNode_SubGraph::ForceFinishNode()
{
	TriggerFirstOutput(true);
}

void UFlowNode_SubGraph::OnLoad_Implementation()
{
	if (!SavedAssetInstanceName.IsEmpty() && !Asset.IsNull())
	{
		GetFlowSubsystem()->LoadSubFlow(this, SavedAssetInstanceName);
		SavedAssetInstanceName = FString();
	}
}

#if WITH_EDITOR

FText UFlowNode_SubGraph::GetNodeTitle() const
{
	if (UFlowSettings::Get()->bUseAdaptiveNodeTitles && !Asset.IsNull())
	{
		return FText::Format(LOCTEXT("SubGraphTitle", "{0}\n{1}"), {Super::GetNodeTitle(), FText::FromString(Asset.ToSoftObjectPath().GetAssetName())});
	}

	return Super::GetNodeTitle();
}

FString UFlowNode_SubGraph::GetNodeDescription() const
{
	if (!UFlowSettings::Get()->bUseAdaptiveNodeTitles && !Asset.IsNull())
	{
		return Asset.ToSoftObjectPath().GetAssetName();
	}

	return Super::GetNodeDescription();;
}

UObject* UFlowNode_SubGraph::GetAssetToEdit()
{
	return Asset.IsNull() ? nullptr : Asset.LoadSynchronous();
}

EDataValidationResult UFlowNode_SubGraph::ValidateNode()
{
	if (Asset.IsNull())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Flow Asset not assigned or invalid!"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

TArray<FFlowPin> UFlowNode_SubGraph::GetContextInputs() const
{
	TArray<FFlowPin> ContextInputPins = Super::GetContextInputs();

	if (!Asset.IsNull())
	{
		(void)Asset.LoadSynchronous();

		for (const FName& PinName : Asset.Get()->GetCustomInputs())
		{
			if (!PinName.IsNone())
			{
				ContextInputPins.AddUnique(FFlowPin(PinName));
			}
		}
	}

	return ContextInputPins;
}

TArray<FFlowPin> UFlowNode_SubGraph::GetContextOutputs() const
{
	TArray<FFlowPin> ContextOutputPins = Super::GetContextOutputs();

	if (!Asset.IsNull())
	{
		(void)Asset.LoadSynchronous();

		for (const FName& PinName : Asset.Get()->GetCustomOutputs())
		{
			if (!PinName.IsNone())
			{
				ContextOutputPins.AddUnique(FFlowPin(PinName));
			}
		}
	}

	return ContextOutputPins;
}

void UFlowNode_SubGraph::AutoGenerateDataPins(TMap<FName, FName>& PinNameToBoundPropertyMap, TArray<FFlowPin>& InputDataPins, TArray<FFlowPin>& OutputDataPins) const
{
	if (Asset.IsNull())
	{
		return;
	}

	(void)Asset.LoadSynchronous();

	for (TPair<FGuid, TObjectPtr<UFlowNode>>& Node : Asset->Nodes)
	{
		if (const IFlowNodeWithExternalDataPinSupplierInterface* ExternalPinSuppliedNode = Cast<IFlowNodeWithExternalDataPinSupplierInterface>(Node.Value))
		{
			// If subgraph's current Flow Node uses an external data supplier (that will be this subgraph node),
			// We need to scrape the external input pins from the node and add them to our auto-generated pins list

			TArray<FFlowPin> ExternalInputPins;
			if (ExternalPinSuppliedNode->TryAppendExternalInputPins(ExternalInputPins))
			{
				for (const FFlowPin& FlowPin : ExternalInputPins)
				{
					PinNameToBoundPropertyMap.Add(FlowPin.PinName, FlowPin.PinName);
				}

				InputDataPins.Append(ExternalInputPins);
			}
		}
	}
}

void UFlowNode_SubGraph::PostLoad()
{
	Super::PostLoad();

	SubscribeToAssetChanges();
}

void UFlowNode_SubGraph::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UFlowNode_SubGraph, Asset))
	{
		if (Asset)
		{
			Asset->OnSubGraphReconstructionRequested.Unbind();
		}
	}
}

void UFlowNode_SubGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFlowNode_SubGraph, Asset))
	{
		OnReconstructionRequested.ExecuteIfBound();
		SubscribeToAssetChanges();
	}
}

bool UFlowNode_SubGraph::CanSupplyDataPinValues_Implementation() const
{
	// SubGraph node cannot supply data-pin values directly (they are created via AutoGenerateDataPins instead)
	return false;
}

void UFlowNode_SubGraph::SubscribeToAssetChanges()
{
	if (Asset)
	{
		TWeakObjectPtr<UFlowNode_SubGraph> SelfWeakPtr(this);
		Asset->OnSubGraphReconstructionRequested.BindLambda([SelfWeakPtr]()
		{
			if (SelfWeakPtr.IsValid())
			{
				SelfWeakPtr->OnReconstructionRequested.ExecuteIfBound();
			}
		});
	}
}
#endif

#undef LOCTEXT_NAMESPACE
