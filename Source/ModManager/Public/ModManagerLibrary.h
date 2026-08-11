// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION == 4
#include "AssetRegistry/AssetRegistryState.h"
#endif
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModManagerLibrary.generated.h"

USTRUCT(BlueprintType)
struct MODMANAGER_API FModInfo
{
	GENERATED_BODY()
 
	UPROPERTY(BlueprintReadOnly)
	FString ModName;
 
	UPROPERTY(BlueprintReadOnly)
	FString Version;
 
	UPROPERTY(BlueprintReadOnly)
	FString Author;
 
	UPROPERTY(BlueprintReadOnly)
	FString Description;
 
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> CompatibleGameVersions;
 
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Dependencies;
 
	UPROPERTY(BlueprintReadOnly)
	int32 LoadOrder;
 
	UPROPERTY(BlueprintReadOnly)
	bool Enabled;

	UPROPERTY(BlueprintReadOnly)
	FString CustomMountPoint;

public:
	UPROPERTY(BlueprintReadOnly, SkipSerialization)
	FString ModInfoPath;
	
	UPROPERTY(BlueprintReadOnly, SkipSerialization)
	TArray<FString> ModPakFiles;
};

/**
 * Mod manage function library base.
 */
UCLASS()
class MODMANAGER_API UModManagerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Get mods search path by plugin settings.
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Manager")
	static FString GetModsSearchPath();

	/**
	 * Load mod info from json file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static bool LoadModInfoFromJson(FString JsonFilePath, FModInfo& OutModInfo);
	
	/**
	 * Search mods by given search path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static TArray<FModInfo> SearchMods(FString SearchPath); 
	
	/**
	 * Get all pak files in given dir.
	 * @param SearchPath in search path, usually the GetModsSearchPath()
	 * @param bRecurse should search recursely
	 * @return pak files dir
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static TArray<FString> GetAllPaksInPath(FString SearchPath, bool bRecurse = true);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void SetModEnableState(FString ModInfoJsonPath, TArray<FString> ModRelatedPaks, bool bEnable);
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void MountModPaks(FModInfo ModInfo);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void UnmountModPaks(FModInfo ModInfo);

	/** Call when game instance init. */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void InitModManager();

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static FString GetPakFileName(FString ModPakFilePath);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static bool LoadAssetRegistry(const FString AssetRegistryFilePath);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static FString GetModName(const FString CustomMountPoint);
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static bool LoadShaderLibrary(const FString ModName, const FString ModContentDir);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static bool UnloadShaderLibrary(const FString ModName);
	
#if ENGINE_MAJOR_VERSION == 4
protected:
	static bool LoadFromDisk(const TCHAR* InPath, const FAssetRegistryLoadOptions& InOptions, FAssetRegistryState& OutState, FAssetRegistryVersion::Type* OutVersion = nullptr);
#endif
};
