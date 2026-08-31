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
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	FString ModName;
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	FString Version;
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	FString Author;
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	FString Description;
 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mod Info")
	FString URL;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	TArray<FString> CompatibleGameVersions;
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	TArray<FString> Dependencies;
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Mod Info")
	int32 LoadOrder = 0;
 
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mod Info")
	bool Enabled = true;

	// For plugins mod, usually used in plugin identify.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mod Data")
	FString ModPluginName;
	
	// For plugins mod, usually used in finding plugin descriptor path.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mod Data")
	FString CustomRelativePath;

public:
	UPROPERTY(BlueprintReadOnly, SkipSerialization, Transient)
	FString ModInfoPath;
	
	UPROPERTY(BlueprintReadOnly, SkipSerialization, Transient)
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
	 * Get mod info file name.
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Manager")
	static FString GetModInfoFileName();
	
	/** 
	 * Search mod info files in path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static TArray<FString> SearchModInfoFiles(FString SearchPath);
	
	/**
	 * Load mod info from json file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static bool LoadModInfoFromJson(FString JsonFilePath, FModInfo& OutModInfo, bool bUpdateTransientData = true);
	
	/**
	 * Search mods by given search path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static TArray<FModInfo> SearchMods(FString SearchPath); 
	
	/**
	 * Get all pak files in given dir.
	 * @param SearchPath in search path, usually the GetModsSearchPath()
	 * @param bIoStorage is using io storage.
	 * @return pak files dir
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static TArray<FString> GetAllPaksInPath(FString SearchPath, bool bIoStorage = true);

	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void SetModEnableState(FString ModInfoJsonPath, TArray<FString> ModRelatedPaks, bool bEnable);
	
	static bool TryAddAndMountPlugin(const FString& PluginName, const FString& PluginDescriptorPath);
	
	static void TryActivateGameFeaturePlugin(const FString& PluginDescriptorPath);
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void MountModPaks(FModInfo ModInfo);

	static void MountModPakMain(const FString& PakFilePath, const int& PakOrder, const bool& bLoadIndex, 
		const FString& CustomMountPoint, const FString& CustomRelativePath, const FString& PluginName);
	
	static void MountModDependencies(FModInfo ModInfo);
	
	static bool TryUnmountAndRemovePlugin(const FString& PluginName, const FString& PluginDescriptorPath);
	
	UFUNCTION(BlueprintCallable, Category = "Mod Manager")
	static void UnmountModPaks(FModInfo ModInfo);

	static void UnmountModPaksMain(FModInfo ModInfo);

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
	
	UFUNCTION(BlueprintPure, Category = "Mod Manager")
	static TArray<FString> GetAssetsInMountPoint(FString MountPoint);
	
#if ENGINE_MAJOR_VERSION == 4
protected:
	static bool LoadFromDisk(const TCHAR* InPath, const FAssetRegistryLoadOptions& InOptions, FAssetRegistryState& OutState, FAssetRegistryVersion::Type* OutVersion = nullptr);
#endif
};
