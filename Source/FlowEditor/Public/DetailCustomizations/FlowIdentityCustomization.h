
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IPropertyTypeCustomization.h"

class UFlowComponent;
class IPropertyHandle;

class FFlowIdentityCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FFlowIdentityCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface
private:
	TSharedRef<SWidget> CreateHeaderWidget();

	TSharedRef<SWidget> GenerateActorPicker();
	TSharedRef<SWidget> GenerateTagPicker();
	void SelectCurrent();
	void OnActorSelected(AActor* Actor);

	ECheckBoxState IsTagChecked(FGameplayTag Tag) const;
	void OnTagCheckChanged(ECheckBoxState NewState, FGameplayTag Tag);

	void ApplyAll();
	void ApplySelected();
	void AppendAll();
	void AppendSelected();
	void ClearTags();
	
	void SelectAll();
	void SelectNone();

	void MoveTags_ActorToNode();
	void MoveTags_PickedActorToNode(AActor* Actor);

	
	TSharedRef<SWidget> GenerateMatchingActorsPicker();
	void OnMatchingActorPicked(AActor* Actor);

private:
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> IdentityTagsHandle;
	TSharedPtr<IPropertyHandle> MatchTypeHandle;

	TSharedPtr<SWidget> HeaderWidget;
	TSharedPtr<SVerticalBox> TagOptions;

	TWeakObjectPtr<UFlowComponent> SelectionSource;
	TMap<FGameplayTag, bool> SelectedOptions;
	bool bCloseOnSelection;
};