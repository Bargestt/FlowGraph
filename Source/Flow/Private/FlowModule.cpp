// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "FlowModule.h"

#include "FlowVersion.h"
#include "Modules/ModuleManager.h"

const FGuid FFlowVersion::GUID(0x2e8af3ae, 0xde0944e2, 0xaf936817, 0x9250369c);

static FCustomVersionRegistration GRegisterFlowCustomVersion(FFlowVersion::GUID, FFlowVersion::LatestVersion, TEXT("FlowVersion"));

void FFlowModule::StartupModule()
{
}

void FFlowModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FFlowModule, Flow)
