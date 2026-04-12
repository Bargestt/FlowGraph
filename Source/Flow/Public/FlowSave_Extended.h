// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowSave.h"
#include "FlowSave_Extended.generated.h"

class UFlowComponent;
class UFlowAsset;

USTRUCT(BlueprintType)
struct FLOW_API FFlowSaveData_Level
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Flow")
	TMap<FName, FFlowComponentSaveData> FlowComponents;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Flow")
	TMap<FName, FFlowAssetSaveData> FlowInstances;	
};

USTRUCT(BlueprintType)
struct FLOW_API FFlowSaveData_World
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Flow")
	TMap<FName, FFlowSaveData_Level> LevelSaves;
};



UCLASS(BlueprintType)
class FLOW_API UFlowSaveGame_Extended : public UFlowSaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Flow")
	TMap<FName, FFlowSaveData_World> WorldSaves;	
};

struct FFlowSaveUtils
{
	static FName GetSaveId_World(const class UWorld* World);
	static FName GetSaveId_Level(const class ULevel* Level);
	static FName GetSaveId_Component(const class UFlowComponent* Component);		
	static ULevel* GetLevelFromObject(const UObject* Object);
};
