// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "FlowModule.h"

#include "Modules/ModuleManager.h"
#include "FlowTag.h"

FGlobalFlowTags FGlobalFlowTags::GTags;

#define LOCTEXT_NAMESPACE "Flow"

void FFlowModule::StartupModule()
{
}

void FFlowModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlowModule, Flow)
