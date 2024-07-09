// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/Route/FlowNode_NamedReroute.h"

#include "FlowAsset.h"

UFlowNode_NamedReroute::UFlowNode_NamedReroute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("Route");
	NodeStyle = EFlowNodeStyle::Logic;
#endif
	AllowedSignalModes = {EFlowSignalMode::Enabled, EFlowSignalMode::Disabled};
	
	Color = FLinearColor::Black;
	bIsInput = true;
	RebuildPins();
}

void UFlowNode_NamedReroute::RebuildPins()
{
	if (IsInput())
	{
		InputPins = { DefaultInputPin };
		OutputPins.Reset();
	}
	else
	{
		InputPins.Reset();
		OutputPins = { DefaultOutputPin };		
	}
#if WITH_EDITORONLY_DATA
	OnReconstructionRequested.ExecuteIfBound();
#endif	
}

void UFlowNode_NamedReroute::ExecuteInput(const FName& PinName)
{
	if (IsInput() && !InputName.IsNone())
	{
		UFlowAsset* FlowAsset = GetFlowAsset();
		if (FlowAsset)
		{		
			const TMap<FGuid, UFlowNode*>& Map = FlowAsset->GetNodes();
			for (const FGuid& TargetId : TargetNodes)
			{
				UFlowNode_NamedReroute* Target = Cast<UFlowNode_NamedReroute>(Map.FindRef(TargetId));
				if (Target && Target->IsOutput() && Target->OutputName == InputName)
				{
					Target->TriggerFirstOutput(true);
				}
			}
		}
	}
}

#if WITH_EDITOR

void UFlowNode_NamedReroute::UpdateReroutes(UFlowNode_NamedReroute* Changed, UFlowAsset* Asset)
{
	if (Asset)
	{		
		TMultiMap<FName, TObjectPtr<UFlowNode_NamedReroute>> Reroutes;
		
		const TMap<FGuid, UFlowNode*>& Map = Asset->GetNodes();
		for (const auto& Pair : Map)
		{
			UFlowNode_NamedReroute* Reroute = Cast<UFlowNode_NamedReroute>(Pair.Value);
			if (Reroute)
			{
				FName RerouteName = Reroute->GetRerouteName();
				if (RerouteName.IsNone())
				{
					Reroute->TargetNodes.Empty(); 
				}
				else
				{
					Reroutes.Add(RerouteName, Reroute);					
				}
			}
		}

		for (const auto& Pair : Reroutes)
		{
			UFlowNode_NamedReroute* Reroute = Pair.Value;
			Reroute->TargetNodes.Empty();
			
			if (Reroute->IsInput())
			{
				TArray<TObjectPtr<UFlowNode_NamedReroute>> Matches;				
				Reroutes.MultiFind(Reroute->InputName, Matches);
							
				for (UFlowNode_NamedReroute* Match : Matches)
				{
					if (Match != Reroute && Match->IsOutput())
					{
						Match->Color = Reroute->Color;
						Reroute->TargetNodes.Add(Match->NodeGuid);
					}					
				}				
			}
		}
	}
}


TArray<FName> UFlowNode_NamedReroute::GetOutputNameOptions() const
{
	TArray<FName> Options;
	
	UFlowAsset* Asset = GetFlowAsset();
	if (Asset)
	{
		const TMap<FGuid, UFlowNode*>& Map = Asset->GetNodes();
		for (const auto& Pair : Map)
		{
			UFlowNode_NamedReroute* Reroute = Cast<UFlowNode_NamedReroute>(Pair.Value);
			if (Reroute && Reroute->IsInput() && !Reroute->InputName.IsNone())
			{
				Options.Add(Reroute->InputName);
			}
		}
	}

	Options.StableSort(FNameLexicalLess());
	Options.Insert(NAME_None, 0);
	return Options;
}

FText UFlowNode_NamedReroute::GetNodeTitle() const
{	
	return GetFlowAsset() ? FText::FromString(FString::Printf(TEXT("Route: %s"), *GetRerouteName().ToString())) : Super::GetNodeTitle();
}

bool UFlowNode_NamedReroute::GetDynamicTitleColor(FLinearColor& OutColor) const
{
	OutColor = Color;
	return true;
}

FString UFlowNode_NamedReroute::GetNodeDescription() const
{
	return Super::GetNodeDescription();
}

void UFlowNode_NamedReroute::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetRerouteName().IsNone())
	{
		Color = FLinearColor::Black;
	}
	else
	{
		if (Color == FLinearColor::Black)
		{
			Color = FLinearColor::MakeRandomColor();
		}
	}

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFlowNode_NamedReroute, bIsInput))
	{
		RebuildPins();
	}

	UpdateReroutes(this, GetFlowAsset());
}

#endif // WITH_EDITOR




