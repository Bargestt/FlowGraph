// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowSave_Extended.h"

#include "FlowComponent.h"



FName FFlowSaveUtils::GetSaveId_World(const UWorld* World)
{
	return World ? World->GetFName() : NAME_None;
}

FName FFlowSaveUtils::GetSaveId_Level(const ULevel* Level)
{
	if(!Level)
	{
		return NAME_None;
	}
	
	if (Level->IsPersistentLevel())
	{
		return TEXT("Persistent");		
	}
	
	if (ULevelStreaming* StreamingLevel = ULevelStreaming::FindStreamingLevel(Level))
	{
		return *StreamingLevel->GetWorldAsset().GetAssetName();
	}

	return Level->GetOuter()->GetFName();
}

FName FFlowSaveUtils::GetSaveId_Component(const UFlowComponent* Component)
{
	if(!Component)
	{
		return NAME_None;
	}
	
	FString Name = Component->GetName();	
	if (const AActor* Actor = Component->GetOwner())
	{
		Name = Actor->GetName() + TEXT(".") + Name;
	}
	
	return *Name;	
}

ULevel* FFlowSaveUtils::GetLevelFromObject(const UObject* Object)
{
	ULevel* Result = nullptr;
	for ( const UObject* NextOuter = Object; Result == nullptr && NextOuter != nullptr; NextOuter = NextOuter->GetOuter() )
	{		
		if ( const UActorComponent* ActorComponent = Cast<UActorComponent>(NextOuter) )
		{
			Result = ActorComponent->GetComponentLevel();
			break;
		}
		else if ( const AActor* Actor = Cast<AActor>(NextOuter) )
		{
			Result = Actor->GetLevel();
			break;
		}
	}
	
	return Result;
}
