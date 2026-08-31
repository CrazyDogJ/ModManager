// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerSettings.h"

UModManagerSettings::UModManagerSettings()
{
}

FString UModManagerSettings::GetModsSearchPath() const
{
	const FString ModDirectorySetting = FPaths::ProjectDir() + ModsSearchPath;

	if (IFileManager::Get().DirectoryExists(*ModDirectorySetting))
	{
		return ModDirectorySetting;
	}
	
	return FPaths::ProjectModsDir();
}

FString UModManagerSettings::GetModInfoFileName() const
{
	if (ModInfoFileName.IsEmpty())
	{
		return "modinfo";
	}
	
	return ModInfoFileName;
}
