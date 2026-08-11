// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModManager.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif
#include "ModManagerSettings.h"

DEFINE_LOG_CATEGORY(LogModManager)

#define LOCTEXT_NAMESPACE "FModManagerModule"

void FModManagerModule::StartupModule()
{
	RegisterSettings();
}

void FModManagerModule::ShutdownModule()
{
	UnregisterSettings();
}

void FModManagerModule::UnregisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Mod Manager");
	}
#endif // WITH_EDITOR
}

void FModManagerModule::RegisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Mod Manager", LOCTEXT("RuntimeSettingsName", "Mod Manager"),
										 LOCTEXT("RuntimeSettingsDescription", "Configure the Mod Manager plugin"),
										 GetMutableDefault<UModManagerSettings>());
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModManagerModule, ModManager)