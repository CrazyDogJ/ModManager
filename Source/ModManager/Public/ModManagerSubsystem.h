// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModManagerLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModManagerSubsystem.generated.h"

class FPakFileContentsIterator;
class FPakFile;

// USTRUCT()
// struct FPakFileContents
// {
// 	GENERATED_BODY()
// 
// 	FString PakName;
// 	TArray<FString> PakContents;
// };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModStateChangeEvent, const FString&, ModInfoPath, const FModInfo&, ModInfo);

UCLASS()
class MODMANAGER_API UModManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	// TArray<FString> GetAssetsInPak(FString PakFileName);
	
	UPROPERTY(BlueprintAssignable)
	FOnModStateChangeEvent OnModMounted;
	
	UPROPERTY(BlueprintAssignable)
	FOnModStateChangeEvent OnModUnmounted;
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	bool MountModPaks(FString ModInfoPath);
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	bool UnmountModPaks(FString ModInfoPath);
	
	UFUNCTION(BlueprintPure, Category = "Mod Manager|Mod Manager Subsystem")
	TMap<FString, FModInfo> GetMountedMods() { return MountedMods; }
	
	UFUNCTION(BlueprintPure, Category = "Mod Manager|Mod Manager Subsystem")
	bool FindMountedModByModInfoPath(FModInfo& OutInfo, FString ModInfoPath) const;
	
	UFUNCTION(BlueprintPure, Category = "Mod Manager|Mod Manager Subsystem")
	bool FindMountedModByModPluginName(FModInfo& OutInfo, FString& OutModInfoPath, FString ModPluginName) const;
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	FString TryGetUrlByModInfoPath(FString ModInfoPath) const;
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager|Mod Manager Subsystem")
	FString TryGetUrlByModPluginName(FString ModPluginName) const;
	
protected:
	TMap<FString, FModInfo> MountedMods;
	
	// void OnPakFileMounted(const IPakFile& PakFile);
	
	// TArray<FPakFileContents> LoadedPakFiles;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
