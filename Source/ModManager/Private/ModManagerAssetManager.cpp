// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerAssetManager.h"

#include "GameFeatureData.h"
#include "Interfaces/IPluginManager.h"

void UModManagerAssetManager::ScanPrimaryAssetTypesFromConfig()
{
	Super::ScanPrimaryAssetTypesFromConfig();

	ScanPrimaryAssetTypesInPlugins();
}

void UModManagerAssetManager::ScanPrimaryAssetTypesInPlugins()
{
	const auto& PluginManager = IPluginManager::Get();
	const auto ContentPlugins = PluginManager.GetEnabledPluginsWithContent();
	TArray<FString> Paths;
	for (const auto Itr : ContentPlugins)
	{
		Paths.Add("/" + Itr->GetName());
	}
	
	ScanPathsForPrimaryAssets(FPrimaryAssetType("GameFeatureData"), Paths, UGameFeatureData::StaticClass(), false, false);
}
