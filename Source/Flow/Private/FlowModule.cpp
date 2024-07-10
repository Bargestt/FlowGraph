// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "FlowModule.h"

#include "Modules/ModuleManager.h"
#include "FlowTag.h"

FGlobalFlowTags FGlobalFlowTags::GTags;

void FFlowModule::StartupModule()
{
}

void FFlowModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FFlowModule, Flow)
