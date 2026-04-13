// Fill out your copyright notice in the Description page of Project Settings.


#include "DetailCustomizations/FlowActorDetails.h"
#include "ActorDetailsDelegates.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "FlowComponent.h"
#include "Graph/FlowGraphSettings.h"


FFlowActorDetails::~FFlowActorDetails()
{
	OnExtendActorDetails.RemoveAll(this);
}

void FFlowActorDetails::Register()
{
	if (GetDefault<UFlowGraphSettings>()->bShowFlowTagsInActorDetails)
	{
		OnExtendActorDetails.AddSP(this, &FFlowActorDetails::AddFlowCategory);
	}	
}

void FFlowActorDetails::AddFlowCategory(class IDetailLayoutBuilder& Details, const FGetSelectedActors& GetSelectedActors)
{
	if (GetSelectedActors.IsBound())
	{
		TArray<UObject*> Components;
		const TArray<TWeakObjectPtr<AActor>>& SelectedActors = GetSelectedActors.Execute();
		for (auto& WeakActor : SelectedActors)
		{
			if (const AActor* Actor = WeakActor.Get())
			{
				TInlineComponentArray<UFlowComponent*> FlowComps(Actor);		
				Components.Append(FlowComps);
			}
		}

		if (!Components.IsEmpty())
		{
			const ECategoryPriority::Type Priority = GetDefault<UFlowGraphSettings>()->bMarkFlowCategoryImportant ? ECategoryPriority::Important : ECategoryPriority::Default;

			const FText CategoryName = FText::Format(NSLOCTEXT("FlowDetails", "FlowCategoryFormat", "Flow Components: {0} "), FText::AsNumber(Components.Num()));
			const FString Tooltip = FString::JoinBy(Components, LINE_TERMINATOR, [](const UObject* Object)
			{				
				const UActorComponent* Component = CastChecked<UActorComponent>(Object);
				const FString Actor = Component->GetOwner() ? Component->GetOwner()->GetActorNameOrLabel() : TEXT("None");
				return FString(Actor + TEXT(".") + Object->GetName());				
			});
			
			IDetailCategoryBuilder& Category = Details.EditCategory(TEXT("Flow"), CategoryName, Priority);
			Category.SetToolTip(FText::FromString(Tooltip));
			Category.AddExternalObjectProperty(Components, GET_MEMBER_NAME_CHECKED(UFlowComponent, IdentityTags));
		}
	}
}
