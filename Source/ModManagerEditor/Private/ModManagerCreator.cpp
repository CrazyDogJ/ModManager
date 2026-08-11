// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerCreator.h"

#include "IPluginBrowser.h"
#include "ModManagerPluginWizardDefinition.h"

#define LOCTEXT_NAMESPACE "FModManagerCreator"

const FName FModManagerCreator::ModManagerEditorPluginCreatorName("ModManagerPluginCreator");

FModManagerCreator::FModManagerCreator()
{
	RegisterTabSpawner();
}

FModManagerCreator::~FModManagerCreator()
{
	UnregisterTabSpawner();
}

void FModManagerCreator::OpenNewPluginWizard(bool bSuppressErrors) const
{
	if (IPluginBrowser::IsAvailable())
	{
		FGlobalTabmanager::Get()->TryInvokeTab(ModManagerEditorPluginCreatorName);
	}
	else if (!bSuppressErrors)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
							 LOCTEXT("PluginBrowserDisabled",
									 "Creating a game mod requires the use of the Plugin Browser, but it is currently disabled."));
	}
}

void FModManagerCreator::RegisterTabSpawner()
{
	FTabSpawnerEntry& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ModManagerEditorPluginCreatorName,
		FOnSpawnTab::CreateRaw(this, &FModManagerCreator::HandleSpawnPluginTab));

	// Set a default size for this tab
	FVector2D DefaultSize(900.0f, 800.0f);
	FTabManager::RegisterDefaultTabWindowSize(ModManagerEditorPluginCreatorName, DefaultSize);

	Spawner.SetDisplayName(LOCTEXT("NewModTabHeader", "Create New MOD Package"));
	Spawner.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FModManagerCreator::UnregisterTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ModManagerEditorPluginCreatorName);
}

TSharedRef<SDockTab> FModManagerCreator::HandleSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	check(IPluginBrowser::IsAvailable());
	return IPluginBrowser::Get().SpawnPluginCreatorTab(SpawnTabArgs, MakeShared<FModManagerPluginWizardDefinition>());
}

#undef LOCTEXT_NAMESPACE