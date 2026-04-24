// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowSubsystem_Extended.h"

#include "FlowAsset.h"
#include "FlowSave_Extended.h"
#include "Nodes/Graph/FlowNode_SubGraph.h"
#include "Streaming/LevelStreamingDelegates.h"



void UFlowSubsystem_Extended::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FLevelStreamingDelegates::OnLevelBeginMakingInvisible.AddUObject(this, &ThisClass::OnBeginMakingLevelInvisible);
}

void UFlowSubsystem_Extended::Deinitialize()
{
	Super::Deinitialize();
	
	FLevelStreamingDelegates::OnLevelBeginMakingInvisible.RemoveAll(this);
}

void UFlowSubsystem_Extended::LoadRootFlow(UObject* Owner, UFlowAsset* FlowAsset, const FString& SavedAssetInstanceName, const bool bAllowMultipleInstances)
{
	Super::LoadRootFlow(Owner, FlowAsset, SavedAssetInstanceName, bAllowMultipleInstances);
}

void UFlowSubsystem_Extended::LoadSubFlow(UFlowNode_SubGraph* SubGraphNode, const FString& SavedAssetInstanceName)
{	
	Super::LoadSubFlow(SubGraphNode, SavedAssetInstanceName);	
}

void UFlowSubsystem_Extended::OnBeginMakingLevelInvisible(UWorld* World, const ULevelStreaming* Streaming, ULevel* LoadedLevel)
{
	if (LoadedLevel && World && !World->bIsTearingDown)
	{
		SaveSubLevel(LoadedLevel);
	}
}

void UFlowSubsystem_Extended::SaveSubLevel(ULevel* Level)
{
	UFlowSaveGame_Extended* SaveGame = Cast<UFlowSaveGame_Extended>(LoadedSaveGame);
	if (!SaveGame)
	{
		return;
	}
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::SaveSubLevel);
	
	const FName SaveId_World = FFlowSaveUtils::GetSaveId_World(GetWorld());
	const FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(Level);

	UE_LOG(LogFlow, Log, TEXT("SaveSubLevel %s::%s"), *SaveId_World.ToString(), *SaveId_Level.ToString());

	FFlowSaveData_World& WorldState = SaveGame->WorldSaves.FindOrAdd(SaveId_World);	

	// Clear current level state
	WorldState.LevelSaves.Remove(SaveId_Level);

	
	// save Flow Graphs
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::SaveFlow);				
		
		for (const auto& [Instance, WeakOwner] : RootInstances)
		{
			if (Instance)
			{
				UObject* Owner = WeakOwner.Get();
				if (Owner && FFlowSaveUtils::GetLevelFromObject(Owner) == Level)
				{
					TArray<FFlowAssetSaveData> SavedInstances;			
					if (UFlowComponent* FlowComponent = Cast<UFlowComponent>(Owner))
					{
						if (FlowComponent->CanSave())
						{
							FlowComponent->SaveRootFlow(SavedInstances);
						}					
					}
					else
					{
						Instance->SaveInstance(SavedInstances);
					}

					for (const FFlowAssetSaveData& SavedInstance : SavedInstances)
					{
						if (SavedInstance.WorldName.IsEmpty())
						{
							WorldState.LevelSaves.FindOrAdd(NAME_None).FlowInstances.Add(*SavedInstance.InstanceName, SavedInstance);
						}
						else
						{
							WorldState.LevelSaves.FindOrAdd(SaveId_Level).FlowInstances.Add(*SavedInstance.InstanceName, SavedInstance);
						}
					}
				}
			}
		}		
	}

	
	// save Flow Components
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::SaveComponents);			
		
		// retrieve all registered components
		TArray<TWeakObjectPtr<UFlowComponent>> ComponentsArray;
		FlowComponentRegistry.GenerateValueArray(ComponentsArray);

		// ensure uniqueness of entries
		const TSet<TWeakObjectPtr<UFlowComponent>> RegisteredComponents = TSet<TWeakObjectPtr<UFlowComponent>>(ComponentsArray);

		// write archives to SaveGame
		for (const TWeakObjectPtr<UFlowComponent> RegisteredComponent : RegisteredComponents)
		{
			UFlowComponent* Component = RegisteredComponent.Get();
			if (Component && RegisteredComponent->GetWorld() && RegisteredComponent->GetComponentLevel() == Level)
			{
				if (RegisteredComponent->CanSave())
				{
					FFlowComponentSaveData SaveData = RegisteredComponent->SaveInstance();			
					WorldState.LevelSaves.FindOrAdd(SaveId_Level).FlowComponents.Add(FFlowSaveUtils::GetSaveId_Component(Component), SaveData);
				}
			}
		}
	}
}

void UFlowSubsystem_Extended::OnGameSaved(UFlowSaveGame* InSaveGame)
{
	UFlowSaveGame_Extended* SaveGame = Cast<UFlowSaveGame_Extended>(InSaveGame);
	if (!SaveGame)
	{
		return;
	}
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::OnGameSaved);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FName SaveId_World = FFlowSaveUtils::GetSaveId_World(GetWorld());

	FFlowSaveData_World& WorldState = SaveGame->WorldSaves.FindOrAdd(SaveId_World);

	// Copy cached data
	if (LoadedSaveGame_Ext && LoadedSaveGame_Ext != SaveGame)
	{
		if (FFlowSaveData_World* CachedWorldState = LoadedSaveGame_Ext->WorldSaves.Find(SaveId_World))
		{
			for (auto& [Key, Data] : CachedWorldState->LevelSaves)
			{
				if (!Key.IsNone())
				{
					WorldState.LevelSaves.Add(Key, Data);
				}
			}
		}			
	}


	// clear existing data, in case we received reused SaveGame instance
	// we only remove data for the current world + global Flow Graph instances (i.e. not bound to any world if created by UGameInstanceSubsystem)
	// we keep data bound to other worlds
	{
		// Remove global
		if (FFlowSaveData_Level* FlowSaveData_Level = WorldState.LevelSaves.Find(NAME_None))
		{
			FlowSaveData_Level->FlowInstances.Empty();
			FlowSaveData_Level->FlowComponents.Empty();
		}
		SaveGame->FlowInstances.Empty();
		SaveGame->FlowComponents.Empty();

		// Clear active level flows, they will be saved now
		const TArray<ULevel*>& Levels = World->GetLevels();
		for (const ULevel* Level : Levels)
		{
			if (Level)
			{
				const FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(Level);
				if (FFlowSaveData_Level* FlowSaveData_Level = WorldState.LevelSaves.Find(SaveId_Level))
				{
					FlowSaveData_Level->FlowComponents.Empty();
					FlowSaveData_Level->FlowInstances.Empty();
				}
			}
		}
	}
	

	// save Flow Graphs
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::SaveFlow);
		
		for (const auto& [Instance, WeakOwner] : RootInstances)
		{
			if (Instance)
			{
				UObject* Owner = WeakOwner.Get();
				if (Owner)
				{					
					TArray<FFlowAssetSaveData> SavedInstances;			
					if (UFlowComponent* FlowComponent = Cast<UFlowComponent>(Owner))
					{
						if (FlowComponent->CanSave())
						{
							FlowComponent->SaveRootFlow(SavedInstances);
						}					
					}
					else
					{
						Instance->SaveInstance(SavedInstances);
					}

					FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(FFlowSaveUtils::GetLevelFromObject(Owner));

					for (const FFlowAssetSaveData& SavedInstance : SavedInstances)
					{
						if (SavedInstance.WorldName.IsEmpty())
						{
							WorldState.LevelSaves.FindOrAdd(NAME_None).FlowInstances.Add(*SavedInstance.InstanceName, SavedInstance);
						}
						else
						{
							WorldState.LevelSaves.FindOrAdd(SaveId_Level).FlowInstances.Add(*SavedInstance.InstanceName, SavedInstance);
						}					
					}
				}
			}
		}		
	}

	
	// save Flow Components
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UFlowSubsystem::SaveComponents);
		
		// retrieve all registered components
		TArray<TWeakObjectPtr<UFlowComponent>> ComponentsArray;
		FlowComponentRegistry.GenerateValueArray(ComponentsArray);

		// ensure uniqueness of entries
		const TSet<TWeakObjectPtr<UFlowComponent>> RegisteredComponents = TSet<TWeakObjectPtr<UFlowComponent>>(ComponentsArray);

		// write archives to SaveGame
		for (const TWeakObjectPtr<UFlowComponent> RegisteredComponent : RegisteredComponents)
		{
			UFlowComponent* Component = RegisteredComponent.Get();
			if (Component && Component->GetWorld())
			{
				ULevel* ComponentLevel = Component->GetComponentLevel();
				if (ComponentLevel && RegisteredComponent->CanSave())
				{
					FFlowComponentSaveData SaveData = RegisteredComponent->SaveInstance();

					FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(ComponentLevel);
					WorldState.LevelSaves.FindOrAdd(SaveId_Level).FlowComponents.Add(FFlowSaveUtils::GetSaveId_Component(Component), SaveData);
				}
			}
		}
	}
}

void UFlowSubsystem_Extended::OnGameLoaded(UFlowSaveGame* SaveGame)
{
	Super::OnGameLoaded(SaveGame);
	LoadedSaveGame_Ext = Cast<UFlowSaveGame_Extended>(LoadedSaveGame);
}

const FFlowComponentSaveData* UFlowSubsystem_Extended::GetLoadedComponentRecord(const UFlowComponent* Component) const
{
	if (!LoadedSaveGame_Ext)
	{
		return nullptr;
	}
	
	const FName SaveId_World = FFlowSaveUtils::GetSaveId_World(Component->GetWorld());	
	if (FFlowSaveData_World* FlowSaveData_World = LoadedSaveGame_Ext->WorldSaves.Find(SaveId_World))
	{
		const FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(Component->GetComponentLevel());
		if (FFlowSaveData_Level* FlowSaveData_Level = FlowSaveData_World->LevelSaves.Find(SaveId_Level))
		{
			const FName SaveId_Instance = FFlowSaveUtils::GetSaveId_Component(Component);
			return FlowSaveData_Level->FlowComponents.Find(SaveId_Instance);
		}
	}
	
	return nullptr;
}

const FFlowAssetSaveData* UFlowSubsystem_Extended::GetLoadedAssetRecord(const UObject* Owner, const UFlowAsset* Asset, const FString& SavedAssetInstanceName) const
{	
	if (!LoadedSaveGame_Ext)
	{
		return nullptr;
	}
	
	const bool bAssetBoundToWorld = Asset->IsBoundToWorld();
	
	if (const UFlowNode_SubGraph* SubGraphNode = Cast<UFlowNode_SubGraph>(Owner))
	{
		const FName SaveId_World = bAssetBoundToWorld ? FFlowSaveUtils::GetSaveId_World(GetWorld()) : NAME_None;
		if (FFlowSaveData_World* FlowSaveData_World = LoadedSaveGame_Ext->WorldSaves.Find(SaveId_World))
		{
			const AActor* RootFlowActorOwner = SubGraphNode->TryGetRootFlowActorOwner();
			const FName SaveId_Level = RootFlowActorOwner ? FFlowSaveUtils::GetSaveId_Level(RootFlowActorOwner->GetLevel()) : NAME_None;
			
			if (FFlowSaveData_Level* FlowSaveData_Level = FlowSaveData_World->LevelSaves.Find(SaveId_Level))
			{
				return FlowSaveData_Level->FlowInstances.Find(*SavedAssetInstanceName);
			}
		}
	}
	else
	{
		const FName SaveId_World = bAssetBoundToWorld ? FFlowSaveUtils::GetSaveId_World(GetWorld()) : NAME_None;
		if (FFlowSaveData_World* FlowSaveData_World = LoadedSaveGame_Ext->WorldSaves.Find(SaveId_World))
		{
			const FName SaveId_Level = FFlowSaveUtils::GetSaveId_Level(FFlowSaveUtils::GetLevelFromObject(Owner));
			
			if (FFlowSaveData_Level* FlowSaveData_Level = FlowSaveData_World->LevelSaves.Find(SaveId_Level))
			{
				return FlowSaveData_Level->FlowInstances.Find(*SavedAssetInstanceName);
			}
		}
	}
	
	return nullptr;
}

