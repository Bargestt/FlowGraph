// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors
#pragma once

#include "Misc/Guid.h"

struct FFlowVersion
{
	FFlowVersion() = delete;
	
	enum Type
	{
		InitialVersion,
		
		// Replaced gameplay tag containers with FFlowIdentity
		FlowIdentity,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	const FLOW_API static FGuid GUID;
};