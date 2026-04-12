// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors
#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

struct FFlowIdentity;
struct FGameplayTagContainer;
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
	
public:
	TArray<FFlowIdentity> GetCurrentValues() const;		
	
private:	
	void ResolveCategoriesMeta(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
	
	TSharedRef<SWidget> CreateHeaderWidget();	
	TSharedRef<SWidget> CreateTagPicker();
	
	void OpenTagPicker_UseCurrentSelection();
	void OpenTagPicker_UseActor(AActor* Actor);		
	
	void FocusActor(AActor* Actor);		
	
	void UseActor_SelectedInViewport();
	void UseActor_Explicit(AActor* Actor);	
	void UseActor_FromEyeDrop();	
	
	TSharedRef<SWidget> MenuContent_ActorPicker();
	TSharedRef<SWidget> MenuContent_FindMatching();
	
	bool CanOpenMatchingPopup() const;
	void FindMatchingPopup();

private:
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> IdentityTagsHandle;
	
	TSharedPtr<IDetailsView> OwningView;

	TSharedPtr<SWidget> HeaderWidget;
	TSharedPtr<SWidget> PopUpAnchor;
	
	TSharedPtr<class SFlowIdentityTagSelector> TagSelector;
};