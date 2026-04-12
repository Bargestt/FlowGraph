// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors
#pragma once

#include "EngineDefines.h"
#include "Engine/StreamableManager.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"
#include "MovieScene/MovieSceneFlowEventReceiverInterface.h"

#include "Nodes/FlowNode.h"
#include "Types/FlowIdentity.h"
#include "FlowNode_PlayLevelSequence.generated.h"

class UFlowLevelSequencePlayer;

DECLARE_MULTICAST_DELEGATE(FFlowNodeLevelSequenceEvent);

UENUM()
enum class EFlowLevelSequenceRestart : uint8
{
	Allow,
	Disallow,
	DisallowWithError,
};

/**
 * Order of triggering outputs after calling Start
 * - PreStart, just before starting playback
 * - Started
 * - Out (always, even if Sequence is invalid)
 * - Completed
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Play Level Sequence"))
class FLOW_API UFlowNode_PlayLevelSequence : public UFlowNode
	, public IMovieSceneFlowEventReceiverInterface
{
	GENERATED_BODY()
	
public:
	UFlowNode_PlayLevelSequence();
	
public:	
	static FFlowNodeLevelSequenceEvent OnPlaybackStarted;
	static FFlowNodeLevelSequenceEvent OnPlaybackCompleted;

	UPROPERTY(EditAnywhere, Category = "Sequence")
	TSoftObjectPtr<ULevelSequence> Sequence;

	UPROPERTY(EditAnywhere, Category = "Sequence")
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bPlayReverse;

	UPROPERTY(EditAnywhere, Category = "Sequence")
	FLevelSequenceCameraSettings CameraSettings;
	
	/* Level Sequence playback can be moved to any place in the world by applying Transform Origin.
	 * Enabling this option will use actor that created Root Flow instance, i.e. World Settings or Player Controller/
	 * See https://docs.unrealengine.com/5.0/en-US/creating-level-sequences-with-dynamic-transforms-in-unreal-engine/ */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bUseGraphOwnerAsTransformOrigin;
	
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bUseIdentityAsTransformOrigin;

	UPROPERTY(EditAnywhere, Category = "Sequence", meta=(EditCondition="bUseIdentityAsTransformOrigin", EditConditionHides))
	FFlowIdentity TransformOriginIdentity;

	/* If true, playback of this level sequence on the server will be synchronized across other clients. */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bReplicates;

	/* Always relevant for network (overrides bOnlyRelevantToOwner). */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bAlwaysRelevant;

	/* If True, Play Rate will by multiplied by Custom Time Dilation.
	 * Enabling this option will use Custom Time Dilation from actor that created Root Flow instance, i.e. World Settings or Player Controller. */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bApplyOwnerTimeDilation;
	
	
	
	UPROPERTY(EditAnywhere, Category = "Sequence")
	EFlowLevelSequenceRestart Restart = EFlowLevelSequenceRestart::Disallow;
	
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bAutoPreload = true;

	UPROPERTY(EditAnywhere, Category = "Binds")
	bool bAutoFillBindings;
	
	UPROPERTY(EditAnywhere, Category = "Binds", meta=(GetKeyOptions="GetTaggedBindings"))
	TMap<FName, FFlowIdentity> SequenceBinds;
	
protected:
	UPROPERTY()
	TObjectPtr<ULevelSequence> LoadedSequence;

	UPROPERTY()
	TObjectPtr<UFlowLevelSequencePlayer> SequencePlayer;

	/* Play Rate set by the user in PlaybackSettings. */
	float CachedPlayRate;

	UPROPERTY(SaveGame)
	float StartTime;

	UPROPERTY(SaveGame)
	float ElapsedTime;

	UPROPERTY(SaveGame)
	float TimeDilation;

	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> StreamingHandle;
public:
#if WITH_EDITOR
	// IFlowContextPinSupplierInterface
	virtual bool SupportsContextPins() const override { return true; }
	virtual TArray<FFlowPin> GetContextOutputs() const override;
	// --

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	UFUNCTION()
	TArray<FName> GetTaggedBindings() const;
#endif

	virtual void PreloadContent() override;
	virtual void FlushContent() override;

	virtual void InitializeInstance() override;
	virtual void CreatePlayer();

protected:
	virtual void ExecuteInput(const FName& PinName) override;

	virtual void OnSave_Implementation() override;
	virtual void OnLoad_Implementation() override;

	// IMovieSceneFlowEventReceiverInterface
	virtual void TriggerEvent(const FString& EventName) override;

public:
	void OnTimeDilationUpdate(const float NewTimeDilation);

protected:
	virtual void OnPreStart();
	virtual void OnStart();	
	UFUNCTION()
	virtual void OnPlaybackFinished();

public:
	virtual void StopPlayback();
	virtual void PausePlayback();
	virtual void ResumePlayback();

protected:
	virtual void Cleanup() override;

public:
	FString GetPlaybackProgress() const;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
	
	virtual FString GetStatusString() const override;
	virtual UObject* GetAssetToEdit() override;
#endif

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(struct FVisualLogEntry* Snapshot) const override;
#endif
};


