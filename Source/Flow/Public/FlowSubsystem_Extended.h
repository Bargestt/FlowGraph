// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowSubsystem.h"

#include "FlowSubsystem_Extended.generated.h"

class UFlowSaveGame_Extended;

/**
 * 
 */
UCLASS()
class FLOW_API UFlowSubsystem_Extended : public UFlowSubsystem
{
	GENERATED_BODY()
	

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual void LoadRootFlow(UObject* Owner, UFlowAsset* FlowAsset, const FString& SavedAssetInstanceName, const bool bAllowMultipleInstances) override;
	virtual void LoadSubFlow(UFlowNode_SubGraph* SubGraphNode, const FString& SavedAssetInstanceName) override;
	
	virtual void OnBeginMakingLevelInvisible(UWorld* World, const ULevelStreaming* Streaming, ULevel* LoadedLevel);
	
	virtual void SaveSubLevel(ULevel* Level);
	virtual void OnGameSaved(UFlowSaveGame* SaveGame) override;
	virtual void OnGameLoaded(UFlowSaveGame* SaveGame) override;
	

private:
	virtual const FFlowComponentSaveData* GetLoadedComponentRecord(const UFlowComponent* Component) const override;
	virtual const FFlowAssetSaveData* GetLoadedAssetRecord(const UObject* Owner, const UFlowAsset* Asset, const FString& SavedAssetInstanceName) const override;
	
protected:
	UPROPERTY()
	TObjectPtr<UFlowSaveGame_Extended> LoadedSaveGame_Ext;

	
};


