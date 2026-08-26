// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ModManagerAssetManager.generated.h"

// If you are using ue4.27, you should set this to asset manager 
// to make editor know where to scan primary data assets for game feature
UCLASS()
class MODMANAGER_API UModManagerAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
protected:
	virtual void ScanPrimaryAssetTypesFromConfig() override;
	
public:
	void ScanPrimaryAssetTypesInPlugins();
};
