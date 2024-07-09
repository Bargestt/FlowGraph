

#include "DetailCustomizations/FlowIdentityCustomization.h"

#include "DetailWidgetRow.h"
#include "FlowTypes.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "ActorTreeItem.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "FlowComponent.h"
#include "LevelEditor.h"
#include "SceneOutlinerFilters.h"
#include "SceneOutlinerModule.h"
#include "SLevelViewport.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowIdentityCustomization"

void FFlowIdentityCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructPropertyHandle = PropertyHandle;
	IdentityTagsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowIdentity, IdentityTags));
	MatchTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowIdentity, IdentityMatchType));	

	if (StructPropertyHandle->HasMetaData(TEXT("ShowOnlyInnerProperties")))
	{
		return;
	}

	HeaderWidget = CreateHeaderWidget();
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		HeaderWidget.ToSharedRef()
	];
}

void FFlowIdentityCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (StructPropertyHandle->HasMetaData(TEXT("ShowOnlyInnerProperties")))
	{
		HeaderWidget = CreateHeaderWidget();
		ChildBuilder.AddCustomRow(LOCTEXT("HeaderRow", "Header Row"))
		.ValueContent()
		[
			HeaderWidget.ToSharedRef()
		];
	}
	
	FString Categories = StructPropertyHandle->GetMetaData("Categories");
	if (!Categories.IsEmpty())
	{
		IdentityTagsHandle->SetInstanceMetaData("Categories", Categories);
	}	
	ChildBuilder.AddProperty(IdentityTagsHandle.ToSharedRef());
	ChildBuilder.AddProperty(MatchTypeHandle.ToSharedRef());

	TSharedPtr<IPropertyHandle> ComponentFilterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowIdentity, ComponentFilter));	
	if (ComponentFilterHandle.IsValid())
	{
		ChildBuilder.AddProperty(ComponentFilterHandle.ToSharedRef());
	}
	TSharedPtr<IPropertyHandle> ActorFilterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowIdentity, ActorFilter));
	if (ActorFilterHandle.IsValid())
	{
		ChildBuilder.AddProperty(ActorFilterHandle.ToSharedRef());
	}
}

TSharedRef<SWidget> FFlowIdentityCustomization::CreateHeaderWidget()
{
	const FMargin IconPadding(2, 0);
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(IconPadding)
		[
			SNew(SButton)
			.ButtonStyle( FAppStyle::Get(), "HoverHintOnly" )
			.ContentPadding(4.0f)
			.ToolTipText(LOCTEXT("ApplyToNode", "Apply Identity to Node"))
			.Content()
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.Previous"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
			.OnClicked_Lambda([this]()
			{
				this->MoveTags_ActorToNode();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(IconPadding)
		[
			SNew(SComboButton)
			.ButtonStyle( FAppStyle::Get(), "HoverHintOnly" )
			.HasDownArrow(false)
			.ContentPadding(4.0f)
			.ToolTipText(LOCTEXT("SelectActorIndentity", "Pick actor identity from Level"))
			.ButtonContent()
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.SelectInViewport"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
			.OnGetMenuContent( this, &FFlowIdentityCustomization::GenerateActorPicker )
		]		
		+ SHorizontalBox::Slot().AutoWidth().Padding(IconPadding)
		[
			PropertyCustomizationHelpers::MakeInteractiveActorPicker(FOnGetAllowedClasses::CreateLambda([](TArray<const UClass*>& Classes)
			{
				Classes.Add(AActor::StaticClass());
			}),
			FOnShouldFilterActor::CreateLambda([](const AActor* Actor) -> bool		
			{
				const UFlowComponent* Comp = Actor->FindComponentByClass<UFlowComponent>();
				return Comp && !Comp->IdentityTags.IsEmpty();
			}),
			FOnActorPicked::CreateSP(this, &FFlowIdentityCustomization::MoveTags_PickedActorToNode))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(IconPadding)
		[
			SNew(SComboButton)
			 .ButtonStyle( FAppStyle::Get(), "HoverHintOnly" )
			 .HasDownArrow(false)
			 .ContentPadding(4.0f)
			 .ToolTipText(LOCTEXT("FindMatchingActors", "Find matching actors"))
			 .ButtonContent()
			 [
				SNew(SImage)
				 .Image(FAppStyle::Get().GetBrush("Symbols.SearchGlass"))
				 .ColorAndOpacity(FSlateColor::UseForeground())
			 ]
			 .OnGetMenuContent( this, &FFlowIdentityCustomization::GenerateMatchingActorsPicker )
		];
}

TSharedRef<SWidget> FFlowIdentityCustomization::GenerateActorPicker()
{
	FMenuBuilder MenuBuilder(false, nullptr);	
	
	TArray<FFlowIdentity> Identities;
	{
		TArray<const void*> RawPtrs;
		StructPropertyHandle->AccessRawData(RawPtrs);
		for (const void* RawPtr : RawPtrs)
		{
			if (RawPtr)
			{
				Identities.Add(*reinterpret_cast<const FFlowIdentity*>(RawPtr));
			}			
		}
	}
	
	FUIAction DeniedAction(FExecuteAction(), FCanExecuteAction::CreateLambda([](){return false;}));
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("FilterInfo", "Filter Info"));
	{
		TArray<FString> ActorFilters;
		TArray<FString> CompFilters;
		for (const FFlowIdentity& Identity : Identities)
		{
			if (Identity.ActorFilter)
			{
				ActorFilters.Add(GetNameSafe(Identity.ActorFilter));
			}
			if (Identity.ComponentFilter)
			{
				CompFilters.Add(GetNameSafe(Identity.ComponentFilter));
			}
		}

		if (!ActorFilters.IsEmpty())
		{
			MenuBuilder.AddMenuEntry(FText::Format(LOCTEXT("ActorFilters", "Actor Class: {0}"), FText::FromString(FString::Join(ActorFilters, TEXT(", ")))),
				FText::GetEmpty(), FSlateIcon(), DeniedAction);
		}
		if (!CompFilters.IsEmpty())
		{
			MenuBuilder.AddMenuEntry(FText::Format(LOCTEXT("ComponentFilters", "Component Class: {0}"), FText::FromString(FString::Join(CompFilters, TEXT(", ")))),
				FText::GetEmpty(), FSlateIcon(), DeniedAction);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("BrowseHeader", "Browse"));
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("UseSelected", "Use selected"), FText::GetEmpty(), FSlateIcon(),
		   FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::SelectCurrent)));
		
		FOnShouldFilterActor OnShouldFilter = FOnShouldFilterActor::CreateLambda([Identities](const AActor* Actor) -> bool		
		{
			const UFlowComponent* Comp = Actor->FindComponentByClass<UFlowComponent>();
			if (Comp && !Comp->IdentityTags.IsEmpty())
			{
				for (const FFlowIdentity& Identity : Identities)
				{
					if (Identity.ActorFilter && !Actor->IsA(Identity.ActorFilter))
					{
						return false;
					}
					if (Identity.ComponentFilter && !Comp->IsA(Identity.ComponentFilter))
					{
						return false;
					}
				}
				return true;
			}
			return false;
		});
	
		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

		FSceneOutlinerInitializationOptions InitOptions;
		InitOptions.Filters->AddFilterPredicate<FActorTreeItem>(OnShouldFilter);
		InitOptions.bFocusSearchBoxWhenOpened = true;
		
		InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 0));
		InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::ActorInfo(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 10));

		
		TSharedPtr<SWidget> MenuContent =
			SNew(SBox)
			.WidthOverride(350)
			.HeightOverride(300)
			[
				SceneOutlinerModule.CreateActorPicker(InitOptions,
					FOnActorPicked::CreateSP(this, &FFlowIdentityCustomization::OnActorSelected))
			];
		
		MenuBuilder.AddWidget(MenuContent.ToSharedRef(), FText::GetEmpty(), true);
	}
	MenuBuilder.EndSection();

	MenuBuilder.AddWidget(GenerateTagPicker(), FText::GetEmpty(), true);

	return SNew(SBox)
		[
			MenuBuilder.MakeWidget()
		];	
}

TSharedRef<SWidget> FFlowIdentityCustomization::GenerateTagPicker()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	FUIAction DeniedAction(FExecuteAction(), FCanExecuteAction::CreateLambda([](){return false;}));	
	MenuBuilder.AddMenuEntry(
		MakeAttributeLambda([this]()
		{
			const UFlowComponent* Comp = SelectionSource.Get();
			return FText::FromString(GetNameSafe(Comp ? Comp->GetOwner() : nullptr));
		}),
		MakeAttributeLambda([this]()
		{			
			const UFlowComponent* Comp = SelectionSource.Get();
			AActor* Owner = Comp ? Comp->GetOwner() : nullptr;
			return FText::Format(LOCTEXT("TagSourceTooltip", "Actor: {0}\nComponent: {1}\nPath: {2}"),
				FText::FromString(GetNameSafe(Owner ? Owner->GetClass() : nullptr)),
				FText::FromString(GetNameSafe(Comp ? Comp->GetClass() : nullptr)),
				FText::FromString(GetPathNameSafe(Owner))
				);
		}),
		FSlateIcon(), DeniedAction);

	
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ApplyTag", "Apply Tags"));
	{
		FMenuBarBuilder ApplyActions(nullptr);	
	
		ApplyActions.AddMenuEntry(LOCTEXT("SetAll", "Set All"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::ApplyAll)));

		ApplyActions.AddMenuEntry(LOCTEXT("SetSelected", "Set Selected"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::ApplySelected)));

		ApplyActions.AddMenuEntry(LOCTEXT("AppendAll", "Append All"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::AppendAll)));

		ApplyActions.AddMenuEntry(LOCTEXT("AppendSelected", "Append Selected"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::AppendSelected)));

		ApplyActions.AddMenuEntry(LOCTEXT("Clear", "Clear"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::ClearTags)));

		MenuBuilder.AddWidget(ApplyActions.MakeWidget(), FText::GetEmpty(), true);
	}
	MenuBuilder.EndSection();
	
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("SelectTag", "Select Tags"));
	{
		FMenuBarBuilder SelectionActions(nullptr);	
	
		SelectionActions.AddMenuEntry(LOCTEXT("Select All", "Select All"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::SelectAll)));
		
		SelectionActions.AddMenuEntry(LOCTEXT("Select None", "Select None"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FFlowIdentityCustomization::SelectNone)));
		
		MenuBuilder.AddWidget(SelectionActions.MakeWidget(), FText::GetEmpty(), true);
				
		TSharedRef<SWidget> OptionsContent =
			SNew(SBox)
            .MinDesiredHeight(100)
			.VAlign(VAlign_Top)
			[
				SAssignNew(TagOptions, SVerticalBox)
			];
		OnActorSelected(nullptr);
		MenuBuilder.AddWidget(OptionsContent, FText::GetEmpty(), true);
	}
	MenuBuilder.EndSection();

	return SNew(SBox)
		[
			MenuBuilder.MakeWidget()
		];
}

void FFlowIdentityCustomization::SelectCurrent()
{
	TArray<AActor*> SelectedActors = GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->GetSelectedLevelActors();
	for (AActor* SelectedActor : SelectedActors)
	{
		UFlowComponent* Comp = SelectedActor ? SelectedActor->FindComponentByClass<UFlowComponent>() : nullptr;
		if (Comp && !Comp->IdentityTags.IsEmpty())
		{
			OnActorSelected(SelectedActor);
			break;
		}
	}
}

void FFlowIdentityCustomization::OnActorSelected(AActor* Actor)
{
	SelectionSource.Reset();
	SelectedOptions.Reset();
	TagOptions->ClearChildren();
	
	UFlowComponent* Comp = Actor ? Actor->FindComponentByClass<UFlowComponent>() : nullptr;
	if (Comp)
	{
		SelectionSource = Comp;
		FGameplayTagContainer Tags = Comp->IdentityTags;
		for(const FGameplayTag& Tag : Tags)
		{
			SelectedOptions.Add(Tag, false);
			TagOptions->AddSlot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(this, &FFlowIdentityCustomization::IsTagChecked, Tag)
					.OnCheckStateChanged(this, &FFlowIdentityCustomization::OnTagCheckChanged, Tag)
				]
				+ SHorizontalBox::Slot().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(FText::FromString(Tag.ToString()))
				]				
			];
		}
	}
	else
	{
		TagOptions->AddSlot()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("SelectActorHint", "Select actor with Flow Component"))		
		];
	}
}

ECheckBoxState FFlowIdentityCustomization::IsTagChecked(FGameplayTag Tag) const
{
	const bool* bIsSelected = SelectedOptions.Find(Tag);
	if (bIsSelected)
	{
		return (*bIsSelected) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Undetermined;
}

void FFlowIdentityCustomization::OnTagCheckChanged(ECheckBoxState NewState, FGameplayTag Tag)
{
	SelectedOptions[Tag] = (NewState == ECheckBoxState::Checked);
}

void FFlowIdentityCustomization::ApplyAll()
{
	FGameplayTagContainer Tags;
	for (auto& Pair : SelectedOptions)
	{
		Tags.AddTag(Pair.Key);
	}
	
	FString Value;
	FGameplayTagContainer::StaticStruct()->ExportText(Value, &Tags, nullptr, nullptr, PPF_None, nullptr);
	IdentityTagsHandle->SetValueFromFormattedString(Value);

	if (bCloseOnSelection)
	{
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FFlowIdentityCustomization::ApplySelected()
{
	FGameplayTagContainer Tags;
	for (auto& Pair : SelectedOptions)
	{
		if (Pair.Value == true)
		{
			Tags.AddTag(Pair.Key);
		}		
	}
	
	FString Value;
	FGameplayTagContainer::StaticStruct()->ExportText(Value, &Tags, nullptr, nullptr, PPF_None, nullptr);
	IdentityTagsHandle->SetValueFromFormattedString(Value);
	
	if (bCloseOnSelection)
	{
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FFlowIdentityCustomization::AppendAll()
{
	FGameplayTagContainer Tags;
	for (auto& Pair : SelectedOptions)
	{
		Tags.AddTag(Pair.Key);	
	}
	
	TArray<FGameplayTagContainer> MatchContainers;
	{
		TArray<const void*> RawPtrs;
		IdentityTagsHandle->AccessRawData(RawPtrs);
		for (const void* RawPtr : RawPtrs)
		{
			if (RawPtr)
			{
				MatchContainers.Add(*reinterpret_cast<const FGameplayTagContainer*>(RawPtr));
			}
		}
	}

	TArray<FString> Values;
	for (const FGameplayTagContainer& Container : MatchContainers)
	{
		FGameplayTagContainer CombinedTags = Container;
		CombinedTags.AppendTags(Tags);
		
		FString Value;
		FGameplayTagContainer::StaticStruct()->ExportText(Value, &CombinedTags, nullptr, nullptr, PPF_None, nullptr);
		IdentityTagsHandle->SetValueFromFormattedString(Value);

		Values.Add(Value);
	}
	
	IdentityTagsHandle->SetPerObjectValues(Values);
	
	if (bCloseOnSelection)
	{
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FFlowIdentityCustomization::AppendSelected()
{
	FGameplayTagContainer Tags;
	for (auto& Pair : SelectedOptions)
	{
		if (Pair.Value == true)
		{
			Tags.AddTag(Pair.Key);
		}		
	}
	
	TArray<FGameplayTagContainer> MatchContainers;
	{
		TArray<const void*> RawPtrs;
		IdentityTagsHandle->AccessRawData(RawPtrs);
		for (const void* RawPtr : RawPtrs)
		{			
			if (RawPtr)
			{
				MatchContainers.Add(*reinterpret_cast<const FGameplayTagContainer*>(RawPtr));
			}
		}
	}

	TArray<FString> Values;
	for (const FGameplayTagContainer& Container : MatchContainers)
	{
		FGameplayTagContainer CombinedTags = Container;
		CombinedTags.AppendTags(Tags);
		
		FString Value;
		FGameplayTagContainer::StaticStruct()->ExportText(Value, &CombinedTags, nullptr, nullptr, PPF_None, nullptr);
		IdentityTagsHandle->SetValueFromFormattedString(Value);

		Values.Add(Value);
	}
	
	IdentityTagsHandle->SetPerObjectValues(Values);

	if (bCloseOnSelection)
	{
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FFlowIdentityCustomization::ClearTags()
{
	IdentityTagsHandle->SetValueFromFormattedString(FGameplayTagContainer().ToString());
}

void FFlowIdentityCustomization::SelectAll()
{
	for (auto& Pair : SelectedOptions)
	{
		Pair.Value = true;
	}
}

void FFlowIdentityCustomization::SelectNone()
{
	for (auto& Pair : SelectedOptions)
	{
		Pair.Value = false;
	}
}

void FFlowIdentityCustomization::MoveTags_ActorToNode()
{
	TSharedRef<SWidget> Widget = GenerateTagPicker();
	SelectCurrent();
	//bCloseOnSelection = true;	
	
	FSlateApplication::Get().PushMenu(
		HeaderWidget.ToSharedRef(),
		FWidgetPath(),
		Widget,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));	
}

void FFlowIdentityCustomization::MoveTags_PickedActorToNode(AActor* Actor)
{
	TSharedRef<SWidget> Widget = GenerateTagPicker();
	OnActorSelected(Actor);
	//bCloseOnSelection = true;
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( "LevelEditor");
	TSharedPtr<SLevelViewport> LevelEditor = LevelEditorModule.GetFirstActiveLevelViewport();
	
	FSlateApplication::Get().PushMenu(
		LevelEditor.ToSharedRef(),
		FWidgetPath(),
		Widget,
		FSlateApplication::Get().GetCursorPos() + FVector2D(0, 50),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

TSharedRef<SWidget> FFlowIdentityCustomization::GenerateMatchingActorsPicker()
{
	TArray<FFlowIdentity> Identities;
	{
		TArray<const void*> RawPtrs;
		StructPropertyHandle->AccessRawData(RawPtrs);
		for (const void* RawPtr : RawPtrs)
		{
			if (RawPtr)
			{
				Identities.Add(*reinterpret_cast<const FFlowIdentity*>(RawPtr));
			}			
		}
	}
	
	FOnShouldFilterActor OnShouldFilter = FOnShouldFilterActor::CreateLambda([&Identities](const AActor* Actor) -> bool		
	{
		UFlowComponent* Comp = Actor->FindComponentByClass<UFlowComponent>();
		if (Comp && !Comp->IdentityTags.IsEmpty())
		{
			for (const FFlowIdentity& Identity : Identities)
			{
				if (Identity.Matches(Actor, Comp))
				{
					return true;
				}
			}
		}
		return false;
	});
	
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerInitializationOptions InitOptions;
	InitOptions.Filters->AddFilterPredicate<FActorTreeItem>(OnShouldFilter);
	InitOptions.bFocusSearchBoxWhenOpened = true;

	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 0));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::ActorInfo(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 10));

	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("BrowseMatching", "Browse Matching Actors"));
	{
		if (Identities.Num() > 1)
		{			
			MenuBuilder.AddMenuEntry(FText::Format(LOCTEXT("Multiple filters", "Showing matches from {0} selected nodes"), FText::AsNumber(Identities.Num())), FText::GetEmpty(), FSlateIcon(), FUIAction());
		}
		
		TSharedPtr<SWidget> MenuContent =
			SNew(SBox)
			.WidthOverride(350)
			.HeightOverride(300)
			[
				SceneOutlinerModule.CreateActorPicker(InitOptions,
					FOnActorPicked::CreateSP(this, &FFlowIdentityCustomization::OnMatchingActorPicked))
			];
		
		MenuBuilder.AddWidget(MenuContent.ToSharedRef(), FText::GetEmpty(), true);
	}
	
	return MenuBuilder.MakeWidget();	
}

void FFlowIdentityCustomization::OnMatchingActorPicked(AActor* Actor)
{
	if (Actor)
	{
		GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->SetSelectedLevelActors({Actor});
		GEditor->MoveViewportCamerasToActor(*Actor, true);
	}
}
#undef LOCTEXT_NAMESPACE
