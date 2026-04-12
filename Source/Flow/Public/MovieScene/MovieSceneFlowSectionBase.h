// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors
#pragma once

#include "MovieSceneFlowEventInterface.h"
#include "MovieSceneSection.h"
#include "MovieSceneFlowSectionBase.generated.h"

/**
 * Base class for flow sections.
 */
UCLASS()
class FLOW_API UMovieSceneFlowSectionBase : public UMovieSceneSection
	, public IMovieSceneFlowEventInterface
{
	GENERATED_BODY()

public:
	virtual TArrayView<FString> GetAllEntryPoints() override { return TArrayView<FString>(); }
};
