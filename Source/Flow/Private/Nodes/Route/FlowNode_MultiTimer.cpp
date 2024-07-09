// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/Route/FlowNode_MultiTimer.h"

UFlowNode_MultiTimer::UFlowNode_MultiTimer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("Route");
	NodeStyle = EFlowNodeStyle::Latent;
#endif

	InputPins.Add(FFlowPin(TEXT("Skip")));
	InputPins.Add(FFlowPin(TEXT("Restart")));

	OutputPins.Reset();
	OutputPins.Add(FFlowPin(TEXT("Completed")));
	OutputPins.Add(FFlowPin(TEXT("Skipped")));
}

void UFlowNode_MultiTimer::ExecuteInput(const FName& PinName)
{
	if (PinName == TEXT("In"))
	{
		if (!TimerHandles.IsEmpty())
		{
			LogError(TEXT("Timer already active"));
			return;
		}

		SetTimers();
	}
	else if (PinName == TEXT("Skip"))
	{
		TriggerOutput(TEXT("Skipped"), true);
	}
	else if (PinName == TEXT("Restart"))
	{
		Restart();
	}
}

void UFlowNode_MultiTimer::SetTimers()
{
	if (UWorld* World = GetWorld())
	{
		for (int32 Index = 0; Index < Events.Num(); Index++)
		{
			float Delay = Events[Index];
			FTimerHandle& TimerHandle = TimerHandles.FindOrAdd(Index);


			FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnCompletion, Index);
			
			if (Delay > UE_KINDA_SMALL_NUMBER)
			{
				World->GetTimerManager().SetTimer(TimerHandle, Delegate, Delay, false);
			}
			else
			{
				TimerHandle = World->GetTimerManager().SetTimerForNextTick(Delegate);
			}
		}

		if (TimerHandles.IsEmpty())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TriggerCompleted);
		}
	}
	else
	{
		LogError(TEXT("No valid world"));
		TriggerCompleted();
	}
}

void UFlowNode_MultiTimer::Restart()
{
	Cleanup();
	SetTimers();
}

void UFlowNode_MultiTimer::OnCompletion(int32 Index)
{
	if (Events.IsValidIndex(Index))
	{
		TimerHandles.Remove(Index);
		
		TriggerOutput(FString::Printf(TEXT("Event_%d"), Index), false);
		if (TimerHandles.IsEmpty())
		{
			TriggerCompleted();
		}
	}
}

void UFlowNode_MultiTimer::TriggerCompleted()
{
	TriggerOutput(TEXT("Completed"), true);
}

void UFlowNode_MultiTimer::Cleanup()
{
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : TimerHandles)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);			
		}
	}		

	TimerHandles.Empty();
	TimeLeft.Empty();
}

void UFlowNode_MultiTimer::OnSave_Implementation()
{
	TimeLeft.Reset();
	
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : TimerHandles)
		{
			if (Pair.Value.IsValid())
			{
				TimeLeft.Add(Pair.Key, World->GetTimerManager().GetTimerRemaining(Pair.Value));
			}				
		}
	}
}

void UFlowNode_MultiTimer::OnLoad_Implementation()
{	
	if (UWorld* World = GetWorld())
	{
		for (const auto& Pair : TimeLeft)
		{
			const float Time = Pair.Value;
			const int32 Index = Pair.Key;
			if (Time >= 0 && Events.IsValidIndex(Index))
			{
				FTimerHandle& TimerHandle = TimerHandles.FindOrAdd(Index);
				FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnCompletion, Index);
				
				if (Time > UE_KINDA_SMALL_NUMBER)
				{
					World->GetTimerManager().SetTimer(TimerHandle, Delegate, Time, false);
				}
				else
				{
					TimerHandle = World->GetTimerManager().SetTimerForNextTick(Delegate);
				}		
			}
		}

		if (TimerHandles.IsEmpty())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TriggerCompleted);
		}
	}
	TimeLeft.Empty();	
}

#if WITH_EDITOR
TArray<FFlowPin> UFlowNode_MultiTimer::GetContextOutputs()
{
	TArray<FFlowPin> Pins;
	for (int32 Index = 0; Index < Events.Num(); Index++)
	{
		float Delay = Events[Index];
		Pins.Add(FFlowPin(FString::Printf(TEXT("Event_%d"), Index), FText::FromString(FString::Printf(TEXT("%.2fs"), Delay))));
	}
	return Pins;
}

FString UFlowNode_MultiTimer::GetNodeDescription() const
{
	return FString();
}

FString UFlowNode_MultiTimer::GetStatusString() const
{
	TArray<FString> Statuses;
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : TimerHandles)
		{
			if (Pair.Value.IsValid())
			{
				Statuses.Add(FString::Printf(TEXT("%.2f"), World->GetTimerManager().GetTimerRemaining(Pair.Value)));
			}
		}
	}

	return FString::Join(Statuses, LINE_TERMINATOR);
}

void UFlowNode_MultiTimer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFlowNode_MultiTimer, Events) ||
		PropertyChangedEvent.GetPropertyName() == TEXT("Name") ||
		PropertyChangedEvent.GetPropertyName() == TEXT("Delay"))		
	{
		OnReconstructionRequested.ExecuteIfBound();
	}
}
#endif // WITH_EDITOR
