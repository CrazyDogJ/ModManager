// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModManagerSubsystem.generated.h"

class FPakFileContentsIterator;
class FPakFile;

USTRUCT()
struct FPakFileContents
{
	GENERATED_BODY()

	FString PakName;
	TArray<FString> PakContents;
};

/**
 * 
 */
UCLASS()
class MODMANAGER_API UModManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	TArray<FString> GetAssetsInPak(FString PakFileName);
	
protected:
	void OnPakFileMounted(const IPakFile& PakFile);
	
	TArray<FPakFileContents> LoadedPakFiles;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
