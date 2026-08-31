// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ModManagerSettings.generated.h"

/**
 * Mod manager settings object.
 */
UCLASS(Config = Game, defaultconfig)
class MODMANAGER_API UModManagerSettings : public UObject
{
	GENERATED_BODY()
public:
	UModManagerSettings();

	/**
	 * Mods directory of the current game to search.
	 * Example : "Mods/" means mod manager will search mods in FPaths::ProjectDir() + TEXT("Mods/").
	 */
	UPROPERTY(EditDefaultsOnly, config)
	FString ModsSearchPath = TEXT("Mods/");

	/**
	 * If ModsSearchPath invalid, we will use default mods folder defined in engine.
	 * @return 
	 */
	FString GetModsSearchPath() const;
	
	/** Mod info file name. */
	UPROPERTY(EditDefaultsOnly, config)
	FString ModInfoFileName = TEXT("modinfo");
	
	/** Get pure file name. */
	FString GetModInfoFileName() const;
};
