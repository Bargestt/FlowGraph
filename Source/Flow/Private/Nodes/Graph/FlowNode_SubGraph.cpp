// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Nodes/Graph/FlowNode_SubGraph.h"

#include "FlowAsset.h"
#include "FlowSettings.h"
#include "FlowSubsystem.h"
#include "Engine/AssetManager.h"
#include "Interfaces/FlowNodeWithExternalDataPinSupplierInterface.h"
#include "Types/FlowAutoDataPinsWorkingData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_SubGraph)

#define LOCTEXT_NAMESPACE "FlowNode_SubGraph"

FFlowPin UFlowNode_SubGraph::StartPin(TEXT("Start"));
FFlowPin UFlowNode_SubGraph::FinishPin(TEXT("Finish"));
const FName UFlowNode_SubGraph::AssetParams_MemberName = GET_MEMBER_NAME_CHECKED(ThisClass, AssetParams);

UFlowNode_SubGraph::UFlowNode_SubGraph()
	: bCanInstanceIdenticalAsset(false)
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
		if(StreamableHandle.IsValid())
		{
			// Load in progress, do nothing
		}
		else
		{
			if (GetSubFlow() != nullptr && !bAllowRestart)
			{
				// Already active
			}
			else
			{
				if (bForceLoad)
				{
					StartSubFlow();
				}
				else
				{				
					UE_LOG(LogFlow, Log, TEXT("FlowNode_SubGraph: AsyncLoad sub flow %s"), *Asset.ToString());
					StreamableHandle = UAssetManager::Get().LoadAssetList({ Asset.ToSoftObjectPath() }, FStreamableDelegate::CreateUObject(this, &ThisClass::StartSubFlow));
				}
			}
		}
	}
	else if (!PinName.IsNone())
	{
		if(StreamableHandle.IsValid())
		{
			PinsToExecute.Add(PinName);
		}
		else
		{
			GetFlowAsset()->TriggerCustomInput_FromSubGraph(this, PinName);
		}		
	}
}

void UFlowNode_SubGraph::Cleanup()
{
	if (CanBeAssetInstanced() && GetFlowSubsystem())
	{
		GetFlowSubsystem()->RemoveSubFlow(this, EFlowFinishPolicy::Keep);
	}
	StreamableHandle.Reset();
	PinsToExecute.Empty();
	
	Super::Cleanup();
}

void UFlowNode_SubGraph::ForceFinishNode()
{
	OnSubFlowFinished();
	TriggerFirstOutput(true);
}

void UFlowNode_SubGraph::OnSave_Implementation()
{
	bLoadPending = StreamableHandle.IsValid();
}

void UFlowNode_SubGraph::OnLoad_Implementation()
{
	if (!Asset.IsNull())
	{
		if (SavedAssetInstanceName.IsEmpty())
		{
			if (bLoadPending)
			{
				if (bForceLoad)
				{
					StartSubFlow();
				}
				else
				{	
					UE_LOG(LogFlow, Log, TEXT("FlowNode_SubGraph.OnLoad: AsyncLoad sub flow %s"), *Asset.ToString());
					StreamableHandle = UAssetManager::Get().LoadAssetList({ Asset.ToSoftObjectPath() }, FStreamableDelegate::CreateUObject(this, &ThisClass::StartSubFlow));
				}
			}
		}
		else
		{
			GetFlowSubsystem()->LoadSubFlow(this, SavedAssetInstanceName);
			SavedAssetInstanceName = FString();

			TArray<FName> Array = PinsToExecute;
			PinsToExecute.Empty();
			for (const auto& PinName : Array)
			{
				GetFlowAsset()->TriggerCustomInput_FromSubGraph(this, PinName);
			}
		}		
	}
}

void UFlowNode_SubGraph::StartSubFlow()
{
	StreamableHandle.Reset();
	
	if (GetFlowSubsystem())
	{
		GetFlowSubsystem()->CreateSubFlow(this);
	}

	TArray<FName> Array = PinsToExecute;
	PinsToExecute.Empty();
	for (const auto& PinName : Array)
	{
		GetFlowAsset()->TriggerCustomInput_FromSubGraph(this, PinName);
	}

	OnSubFlowStarted();
}

UFlowAsset* UFlowNode_SubGraph::GetSubFlow() const
{
	return GetFlowSubsystem()->GetInstancedSubFlows().FindRef(this);
}

#if WITH_EDITOR

FText UFlowNode_SubGraph::K2_GetNodeTitle_Implementation() const
{
	if (GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles && !Asset.IsNull())
	{
		return FText::Format(LOCTEXT("SubGraphTitle", "{0}\n{1}"), {Super::K2_GetNodeTitle_Implementation(), FText::FromString(Asset.ToSoftObjectPath().GetAssetName())});
	}

	return Super::K2_GetNodeTitle_Implementation();
}

FString UFlowNode_SubGraph::GetStatusString() const
{
	return StreamableHandle.IsValid() ? FString::Printf(TEXT("Loading: %f"), StreamableHandle->GetProgress()) : TEXT("");	
}

FString UFlowNode_SubGraph::GetNodeDescription() const
{
	FString Description;
	if (!GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles && !Asset.IsNull())
	{
		Description += Asset.ToSoftObjectPath().GetAssetName();
	}
	if (bForceLoad)
	{
		Description += FString(LINE_TERMINATOR) + TEXT("Force Load");		
	}
	if (bAllowRestart)
	{
		Description += FString(LINE_TERMINATOR) + TEXT("Allow Restart");
	}

	return Description;
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
		if (Asset.IsValid())
		{
			for (const FName& PinName : Asset->GetCustomInputs())
			{
				if (!PinName.IsNone())
				{
					ContextInputPins.AddUnique(FFlowPin(PinName));
				}
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
		if (Asset.IsValid())
		{
			for (const FName& PinName : Asset->GetCustomOutputs())
			{
				if (!PinName.IsNone())
				{
					ContextOutputPins.AddUnique(FFlowPin(PinName));
				}
			}
		}
	}

	return ContextOutputPins;
}

void UFlowNode_SubGraph::AutoGenerateDataPins(FFlowDataPinValueOwner& ValueOwner, FFlowAutoDataPinsWorkingData& InOutWorkingData)
{
	Super::AutoGenerateDataPins(ValueOwner, InOutWorkingData);

	if (Asset.IsNull())
	{
		return;
	}

	(void)Asset.LoadSynchronous();
	if (!Asset.IsValid())
	{
		return;
	}

	for (TPair<FGuid, TObjectPtr<UFlowNode>>& Node : Asset->Nodes)
	{
		if (const IFlowNodeWithExternalDataPinSupplierInterface* ExternalPinSuppliedNode = Cast<IFlowNodeWithExternalDataPinSupplierInterface>(Node.Value))
		{
			// If subgraph's current Flow Node uses an external data supplier (that will be this subgraph node),
			// We need to scrape the external input pins from the node and add them to our auto-generated pins list

			TArray<FFlowPin> ExternalInputPins;
			if (ExternalPinSuppliedNode->TryAppendExternalInputPins(ExternalInputPins))
			{
				const int32 NewNum = InOutWorkingData.AutoInputDataPinsNext.Num() + ExternalInputPins.Num();
				InOutWorkingData.AutoInputDataPinsNext.Reserve(NewNum);

				for (const FFlowPin& FlowPin : ExternalInputPins)
				{
					InOutWorkingData.AutoInputDataPinsNext.Add(FFlowPinSourceData(FlowPin, ValueOwner));
				}
			}
		}
	}
}

FFlowDataPinResult UFlowNode_SubGraph::TrySupplyDataPin(FName PinName) const
{
	if (PinName == AssetParams_MemberName)
	{
		// Prevent infinite recursion by sourcing the AssetParams pin directly 
		// (otherwise, it would attempt to resolve it below and infinitely crash our stack.
		// don't ask me how I know).
		return Super::TrySupplyDataPin(PinName);
	}

	if (!IsInputConnected(PinName))
	{
		const bool bHasAssetParams = IsInputConnected(AssetParams_MemberName) || !AssetParams.IsNull();
		if (bHasAssetParams)
		{
			// If not connected, we can source the value from the asset data params (if available)
			TObjectPtr<UObject> Value = nullptr;
			const EFlowDataPinResolveResult ResultEnum = Super::TryResolveDataPinValue<FFlowPinType_Object>(AssetParams_MemberName, Value);
			if (FlowPinType::IsSuccess(ResultEnum) && IsValid(Value))
			{
				if (const IFlowDataPinValueSupplierInterface* SupplierInterface = Cast<IFlowDataPinValueSupplierInterface>(Value))
				{
					return SupplierInterface->TrySupplyDataPin(PinName);
				}
				else
				{
					LogError(FString::Printf(TEXT("Could not cast object %s to IFlowDataPinValueSupplierInterface!  This is unexpected."), *Value->GetName()));

					return FFlowDataPinResult(EFlowDataPinResolveResult::FailedWithError);
				}
			}
		}
	}
	
	// Prefer the standard lookup if the pin is connected 
	// (or if there is no FlowAssetParams to ask)
	return Super::TrySupplyDataPin(PinName);
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
