// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerLibrary.h"

#include "GameFeaturesSubsystem.h"
#include "JsonObjectConverter.h"
#include "ModManager.h"
#include "ModManagerSettings.h"
#include "ModManagerUtils.h"
#include "ShaderCodeLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetRegistryState.h"
#include "Interfaces/IPluginManager.h"
#if ENGINE_MAJOR_VERSION == 4
#include "Engine/AssetManager.h"
#include "Serialization/LargeMemoryReader.h"
#endif

FPakPlatformFile* UModManagerLibrary::GetPakFileInterface()
{
	FPlatformFileManager& PlatformFileManager = FPlatformFileManager::Get();
	FPakPlatformFile* PakPlatformFile =
		static_cast<FPakPlatformFile*>(PlatformFileManager.FindPlatformFile(FPakPlatformFile::GetTypeName()));
	if (!PakPlatformFile)
	{
		IPlatformFile& PreviousPlatformFile = PlatformFileManager.GetPlatformFile();
		PakPlatformFile = static_cast<FPakPlatformFile*>(
			PlatformFileManager.GetPlatformFile(FPakPlatformFile::GetTypeName()));
		if (!PakPlatformFile)
		{
			UE_LOG(LogModManager, Warning, TEXT("The PakFile module did not provide a platform file instance."))
			return nullptr;
		}

		if (!PakPlatformFile->Initialize(&PreviousPlatformFile, TEXT("")))
		{
			UE_LOG(LogModManager, Warning, TEXT("Failed to initialize the PakFile platform layer."))
			return nullptr;
		}

		PlatformFileManager.SetPlatformFile(*PakPlatformFile);
	}
	
	return PakPlatformFile;
}

FString UModManagerLibrary::GetModsSearchPath()
{
	if (const auto ModManagerSettings = GetMutableDefault<UModManagerSettings>())
	{
		return ModManagerSettings->GetModsSearchPath();
	}

	return FPaths::ProjectModsDir();
}

FString UModManagerLibrary::GetModInfoFileName()
{
	if (const auto ModManagerSettings = GetMutableDefault<UModManagerSettings>())
	{
		return ModManagerSettings->GetModInfoFileName() + ".json";
	}

	return "modinfo.json";
}

TArray<FString> UModManagerLibrary::SearchModInfoFiles(FString SearchPath)
{
	TArray<FString> Files;
	const auto ModInfoFileName = GetModInfoFileName();
	IFileManager::Get().FindFilesRecursive(Files, *SearchPath, *ModInfoFileName,true, false, false);
	
	return Files;
}

bool UModManagerLibrary::LoadModInfoFromJson(FString JsonFilePath, FModInfo& OutModInfo, bool bUpdateTransientData)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to load mod info JSON: %s"), *JsonFilePath);
		return false;
	}
 
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
 
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to parse mod info JSON: %s"), *JsonFilePath);
		return false;
	}
	
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &OutModInfo, 0, 0))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to convert JSON to FModInfo: %s"), *JsonFilePath);
		return false;
	}

	if (bUpdateTransientData)
	{
		OutModInfo.ModInfoPath = JsonFilePath;
		const auto Paks = GetAllPaksInPath(*FPaths::GetPath(JsonFilePath), true);
		OutModInfo.ModPakFiles = Paks;
	}
 
	return true;
}

TArray<FModInfo> UModManagerLibrary::SearchMods(FString SearchPath)
{
	TArray<FModInfo> Result;
	
	const auto Files = SearchModInfoFiles(SearchPath);

	for (const auto FileName : Files)
	{
		FModInfo ModInfo;
		if (LoadModInfoFromJson(FileName, ModInfo))
		{
			Result.Add(ModInfo);
		}
	}

	return Result;
}

TArray<FString> UModManagerLibrary::GetAllPaksInPath(FString SearchPath, bool bIoStorage)
{
	TArray<FString> Files;
	FPakFileSearchVisitor PakVisitor(Files, bIoStorage);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	UE_LOG(LogModManager, Verbose, TEXT("Searching directory for pak files %s"), *SearchPath);
	PlatformFile.IterateDirectoryRecursively(*SearchPath, PakVisitor);

	if (Files.Num() == 0)
	{
		UE_LOG(LogModManager, Error, TEXT("Does not contain any pak files within %s."), *SearchPath);
		return Files;
	}
	else
	{
		UE_LOG(LogModManager, Verbose, TEXT("Contains %i pak files within %s."), Files.Num(), *SearchPath);
	}
	
	return Files;
}

void UModManagerLibrary::SetModEnableState(FString ModInfoJsonPath, TArray<FString> ModRelatedPaks, bool bEnable)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ModInfoJsonPath))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to load JSON file: %s"), *ModInfoJsonPath);
		return;
	}
	
	FModInfo ModInfo;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &ModInfo, 0, 0))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to parse JSON to struct: %s"), *ModInfoJsonPath);
		return;
	}
	
	ModInfo.Enabled = bEnable;
	ModInfo.ModInfoPath = ModInfoJsonPath;
	ModInfo.ModPakFiles = ModRelatedPaks;
	if (bEnable)
	{
		MountModPaks(ModInfo);
	}
	else
	{
		UnmountModPaks(ModInfo);
	}
	
	FString OutputJsonString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(ModInfo, OutputJsonString))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to serialize struct to JSON string"));
		return;
	}
	
	if (!FFileHelper::SaveStringToFile(OutputJsonString, *ModInfoJsonPath))
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to save JSON file: %s"), *ModInfoJsonPath);
		return;
	}
 
	UE_LOG(LogModManager, Log, TEXT("Successfully modified Enabled flag in: %s"), *ModInfoJsonPath);
}

bool UModManagerLibrary::TryAddAndMountPlugin(const FString& PluginName, const FString& PluginDescriptorPath)
{
	if (IPluginManager::Get().AddToPluginsList(PluginDescriptorPath))
	{
		UE_LOG(LogModManager, Log, TEXT("Mod plugin : %s is added successfully"), *PluginName)
#if ENGINE_MAJOR_VERSION == 5
		return IPluginManager::Get().MountExplicitlyLoadedPlugin(PluginName);
#else
		// UE4.27 has no bool return.
		IPluginManager::Get().MountExplicitlyLoadedPlugin(PluginName);
		// mount point should do right now because the delegate is delay one logic tick.
		const auto Plugin = IPluginManager::Get().FindPlugin(PluginName);
		FPackageName::RegisterMountPoint(Plugin->GetMountedAssetPath(), Plugin->GetContentDir());
		return true;
#endif
	}
	
	return false;
}

void UModManagerLibrary::TryActivateGameFeaturePlugin(const FString& PluginDescriptorPath)
{
	// Game feature try to load and activate this mod plugin.
	UGameFeaturesSubsystem& GFS = UGameFeaturesSubsystem::Get();
#if ENGINE_MAJOR_VERSION == 5
	FString PluginURL = UGameFeaturesSubsystem::GetPluginURL_FileProtocol(PluginDescriptorPath);
	GFS.RegisterGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateLambda([PluginURL](const UE::GameFeatures::FResult& GFCResult)
	{
		// If register successful, we try to load and active game feature plugin.
		if (!GFCResult.HasError())
		{
			UGameFeaturesSubsystem& GFS = UGameFeaturesSubsystem::Get();
			GFS.LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete());
		}
	}));
#else
	const auto PluginName = FPaths::GetBaseFilename(PluginDescriptorPath);
	const FString PreferredGameFeatureDataPath = FString::Printf(TEXT("/%s/%s.%s"), *PluginName, *PluginName, *PluginName);
	const auto StreamableHandle = GFS.LoadGameFeatureData(PreferredGameFeatureDataPath);
	if (StreamableHandle)
	{
		StreamableHandle->WaitUntilComplete(0.0f, false);
		GFS.LoadAndActivateGameFeaturePlugin(TEXT("file:") + PluginDescriptorPath, FGameFeaturePluginLoadComplete());
	}
#endif
}

void UModManagerLibrary::MountModPaks(FModInfo ModInfo)
{
	FPakPlatformFile* PakPlatformFile = GetPakFileInterface();;
	if (!PakPlatformFile)
	{
		UE_LOG(LogModManager, Error, TEXT("PakPlatformFile not found!"));
		return;
	}
	
	// Mount mod dependencies first.
	MountModDependencies(ModInfo);
	
	for (const FString& PakFilePath : ModInfo.ModPakFiles)
	{
		const int32 PakOrder = 999; // High order to override base pak.
		const bool bLoadIndex = true;

		const FString CustomMountPoint = TEXT("/") + ModInfo.ModPluginName + TEXT("/");
		const FString CustomRelativePath = ModInfo.CustomRelativePath;
		
		// Default is windows.
		FString CurrentPlatform = "Windows";
#if PLATFORM_WINDOWS
		CurrentPlatform = "Windows";
#elif PLATFORM_LINUX
		CurrentPlatform = "Linux";
#elif PLATFORM_MAC
		CurrentPlatform = "Mac";
#endif

		if (PakFilePath.Contains(*("-" + CurrentPlatform)))
		{
			MountModPakMain(PakFilePath, PakOrder, bLoadIndex, CustomMountPoint, CustomRelativePath, ModInfo.ModPluginName);
		}
	}
}

void UModManagerLibrary::MountModPakMain(const FString& PakFilePath, const int& PakOrder, const bool& bLoadIndex, 
	const FString& CustomMountPoint, const FString& CustomRelativePath, const FString& PluginName)
{
	FPakPlatformFile* PakPlatformFile = GetPakFileInterface();
	if (!PakPlatformFile)
	{
		UE_LOG(LogModManager, Error, TEXT("PakPlatformFile not found!"));
		return;
	}

	if (PakPlatformFile->Mount(*PakFilePath, PakOrder, nullptr, bLoadIndex))
	{
		// Set up mount point for plugin assets.
		if (!CustomMountPoint.IsEmpty() && CustomMountPoint != "/Game/" && CustomMountPoint != "/Engine/")
		{
			FString ProjectDir = FString::Printf(TEXT("../../../%s/"), FApp::GetProjectName());
			FString PhysicalMountPath = ProjectDir + CustomRelativePath;
			FString PhysicalMountPathContent = PhysicalMountPath + TEXT("Content/");
			FString PhysicalMountPathPluginDesc = PhysicalMountPath + PluginName + TEXT(".uplugin");
			
			// If we can load asset registry, that means this pak is a plugin type mod pak. Else that is a replacement mod pak
			// Should be loaded first no matter what.
			if (LoadAssetRegistry(PhysicalMountPath + TEXT("AssetRegistry.bin")))
			{
				UE_LOG(LogModManager, Log, TEXT("Asset registry %s is loaded. "), *(PhysicalMountPath + TEXT("AssetRegistry.bin")));
				
				// Load shader library.
				if (LoadShaderLibrary(GetModName(CustomMountPoint), PhysicalMountPathContent))
				{
					UE_LOG(LogModManager, Log, TEXT("Shader library load success for mod pak : %s"), *PakFilePath);
				}
				else
				{
					UE_LOG(LogModManager, Warning, TEXT("Shader library load failed for mod pak : %s"), *PakFilePath);
				}
			}
			
			// Load plugin
			if (TryAddAndMountPlugin(PluginName, PhysicalMountPathPluginDesc))
			{
				// Automatically mount point.
				UE_LOG(LogModManager, Log, TEXT("Mod plugin : %s is mounted successfully"), *PluginName);

				// Try activate game feature;
				TryActivateGameFeaturePlugin(PhysicalMountPathPluginDesc);
			}
			else
			{
				// Manually mount point.
				FPackageName::RegisterMountPoint(CustomMountPoint, PhysicalMountPathContent);
				UE_LOG(LogModManager, Log, TEXT("Not a mod plugin, registering mount point: %s -> %s"), *CustomMountPoint, *PhysicalMountPathContent);
			}
		}
	}
	else
	{
		UE_LOG(LogModManager, Warning, TEXT("Failed to mount mod pak: %s"), *PakFilePath);
	}
}

void UModManagerLibrary::MountModDependencies(FModInfo ModInfo)
{
	auto& PluginManager = IPluginManager::Get();
	const auto EnabledPlugins = PluginManager.GetEnabledPlugins();
	
	const auto ModsPath = GetModsSearchPath();
	const TArray<FModInfo> ModInfos = SearchMods(ModsPath);
	
	for (const auto Dependency : ModInfo.Dependencies)
	{
		const auto FoundPlugin = EnabledPlugins.FindByPredicate([Dependency](const TSharedRef<IPlugin> Plugin)
		{
			return Plugin->GetName() == Dependency;
		});

		// Should try to find mod and load
		if (!FoundPlugin)
		{
			const auto FoundMod = ModInfos.FindByPredicate([Dependency](const FModInfo& ModInfo)
			{
				return ModInfo.ModPluginName == Dependency;
			});

			if (FoundMod)
			{
				MountModPaks(*FoundMod);
			}
		}
	}
}

bool UModManagerLibrary::TryUnmountAndRemovePlugin(const FString& PluginName, const FString& PluginDescriptorPath)
{
	if (IPluginManager::Get().UnmountExplicitlyLoadedPlugin(PluginName, nullptr))
	{
		// Unmount plugin.
#if ENGINE_MAJOR_VERSION == 5
		if (IPluginManager::Get().RemoveFromPluginsList(PluginDescriptorPath, nullptr))
		{
			UE_LOG(LogModManager, Error, TEXT("Mod plugin : %s has been removed!"), *PluginName);
			return true;
		}
#else
		IPluginManager::Get().RefreshPluginsList();
		UE_LOG(LogModManager, Error, TEXT("Disabling plugin : %s, Refreshing plugin list"), *PluginName);
#endif
	}
	
	return false;
}

void UModManagerLibrary::UnmountModPaks(FModInfo ModInfo)
{
	auto& GFS = UGameFeaturesSubsystem::Get();
	FString PluginURL;
	const auto PluginName = ModInfo.ModPluginName;
	if (
#if ENGINE_MAJOR_VERSION == 5
		GFS.GetPluginURLByName(PluginName, PluginURL)
#else
		GFS.GetPluginURLForBuiltInPluginByName(PluginName, PluginURL)
#endif
		)
	{
		GFS.UninstallGameFeaturePlugin(PluginURL, FGameFeaturePluginUninstallComplete::CreateLambda([ModInfo](const UE::GameFeatures::FResult& GFResult)
		{
			UnmountModPaksMain(ModInfo);
		}));
	}
	else
	{
		UnmountModPaksMain(ModInfo);
	}
}

void UModManagerLibrary::UnmountModPaksMain(FModInfo ModInfo)
{
	// Try get pak platform file interface.
	FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile"));
	if (!PakPlatformFile)
	{
		UE_LOG(LogModManager, Error, TEXT("PakPlatformFile not found!"));
		return;
	}
	
	UnloadShaderLibrary(ModInfo.ModPluginName);

	// Paths
	const FString CustomRelativePath = ModInfo.CustomRelativePath;
	FString ProjectDir = FString::Printf(TEXT("../../../%s/"), FApp::GetProjectName());
	FString PhysicalMountPath = ProjectDir + CustomRelativePath;
	FString PhysicalMountPathPluginDesc = PhysicalMountPath + ModInfo.ModPluginName + TEXT(".uplugin");

	if (!TryUnmountAndRemovePlugin(ModInfo.ModPluginName, PhysicalMountPathPluginDesc))
	{
		// Remove the automatically-added, incorrect content mount point that the Plugin manager added.
		const FString CustomMountPoint = TEXT("/") + ModInfo.ModPluginName + TEXT("/");
		const FString PhysicalMountPathContent = PhysicalMountPath + TEXT("Content/");
		FPackageName::UnRegisterMountPoint(CustomMountPoint, PhysicalMountPathContent);
		UE_LOG(LogModManager, Log, TEXT("Unregistered mount point: %s -> %s"), *CustomMountPoint, *PhysicalMountPathContent);
	}
	
	for (const FString& PakFilePath : ModInfo.ModPakFiles)
	{
		if (PakPlatformFile->Unmount(*PakFilePath))
		{
			UE_LOG(LogModManager, Log, TEXT("Unmounted mod pak: %s"), *PakFilePath);
		}
		else
		{
			UE_LOG(LogModManager, Warning, TEXT("Failed to unmount mod pak: %s"), *PakFilePath);
		}
	}
}

FString UModManagerLibrary::GetPakFileName(FString ModPakFilePath)
{
	return FPaths::GetCleanFilename(ModPakFilePath);
}

bool UModManagerLibrary::LoadAssetRegistry(const FString AssetRegistryFilePath)
{
	FAssetRegistryState PluginAssetRegistry;
#if ENGINE_MAJOR_VERSION == 5
	if (FAssetRegistryState::LoadFromDisk(*AssetRegistryFilePath, FAssetRegistryLoadOptions(), PluginAssetRegistry))
#else
	if (LoadFromDisk(*AssetRegistryFilePath, FAssetRegistryLoadOptions(), PluginAssetRegistry))
#endif
	{
		TSharedPtr<class FAssetRegistryState> LoadedAssetRegistryState = MakeShared<class FAssetRegistryState>(MoveTemp(PluginAssetRegistry));

		// For debugging purposes, log out all the package names and assets within this UGC package.
		if (UE_LOG_ACTIVE(LogModManager, VeryVerbose))
		{
			TArray<FName> PackageNames;
			LoadedAssetRegistryState->GetPackageNames(PackageNames);
			if (PackageNames.Num() <= 0)
			{
				UE_LOG(LogModManager, Error, TEXT("AssetRegistry has no packages"));
				return false;
			}

			for (const auto& PN : PackageNames)
			{
				UE_LOG(LogModManager, VeryVerbose, TEXT("AssetRegistry contains package %s"), *PN.ToString());
			}

			TArray<FAssetData> AssetList;
			LoadedAssetRegistryState->GetAllAssets({}, AssetList);

			if (AssetList.Num() <= 0)
			{
				UE_LOG(LogModManager, Error, TEXT("AssetRegistry has no assets"));
				return false;
			}

			for (const FAssetData& Asset : AssetList)
			{
				UE_LOG(LogModManager, VeryVerbose, TEXT("AssetRegistry contains asset %s"),
					   *Asset.GetFullName());
			}

			PackageNames.Empty();
			AssetList.Empty();
		}

		UE_LOG(LogModManager, Verbose, TEXT("AssetRegistry loaded from %s. Contains %i assets."),
			   *AssetRegistryFilePath, LoadedAssetRegistryState.Get()->GetNumAssets());
		IAssetRegistry::GetChecked().AppendState(*LoadedAssetRegistryState.Get());
	}
	else
	{
		UE_LOG(LogModManager, Error, TEXT("Failed to load plugin asset registry state %s"), *AssetRegistryFilePath);
		return false;
	}

	return true;
}

FString UModManagerLibrary::GetModName(const FString CustomMountPoint)
{
	FString Input = CustomMountPoint;
	if (Input.StartsWith(TEXT("/"))) Input = Input.Mid(1);
	if (Input.EndsWith(TEXT("/"))) Input = Input.Left(Input.Len() - 1);

	return Input;
}

bool UModManagerLibrary::LoadShaderLibrary(const FString ModName, const FString ModContentDir)
{
	if (ModName.IsEmpty())
	{
		UE_LOG(LogModManager, Error, TEXT("Unable to load shader library on a null plugin."));
		return false;
	}

	bool bArchive = false;
	GConfig->GetBool(TEXT("/Script/UnrealEd.ProjectPackagingSettings"), TEXT("bShareMaterialShaderCode"), bArchive,
					 GGameIni);
	if (FApp::CanEverRender() && bArchive)
	{
		// load any shader libraries that may exist in this plugin
		FShaderCodeLibrary::OpenLibrary(ModName, ModContentDir);
	}

	return true;
}

bool UModManagerLibrary::UnloadShaderLibrary(const FString ModName)
{
	if (ModName.IsEmpty())
	{
		UE_LOG(LogModManager, Error, TEXT("Unable to unload shader library on a null plugin."));
		return false;
	}
	
	bool bArchive = false;
	GConfig->GetBool(TEXT("/Script/UnrealEd.ProjectPackagingSettings"), TEXT("bShareMaterialShaderCode"), bArchive,
					 GGameIni);
	if (FApp::CanEverRender() && bArchive)
	{
		FShaderCodeLibrary::CloseLibrary(ModName);
	}

	return true;
}

TArray<FString> UModManagerLibrary::GetAssetsInMountPoint(const FString MountPoint)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
 
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssetsByPath(FName(MountPoint), AssetList, true);
 
	TArray<FString> Assets;
	for (const FAssetData& Asset : AssetList)
	{
		Assets.Add(Asset.PackageName.ToString());
	}
	return Assets;
}

#if ENGINE_MAJOR_VERSION == 4
bool UModManagerLibrary::LoadFromDisk(const TCHAR* InPath, const FAssetRegistryLoadOptions& InOptions,
	FAssetRegistryState& OutState, FAssetRegistryVersion::Type* OutVersion)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("FAssetRegistryState::LoadFromDisk");
	check(InPath);

	TUniquePtr<FArchive> FileReader(IFileManager::Get().CreateFileReader(InPath));
	if (FileReader)
	{
		// It's faster to load the whole file into memory on a Gen5 console
		TArray64<uint8> Data;
		Data.SetNumUninitialized(FileReader->TotalSize());
		FileReader->Serialize(Data.GetData(), Data.Num());
		check(!FileReader->IsError());

		FLargeMemoryReader MemoryReader(Data.GetData(), Data.Num());
		
		return OutState.Load(MemoryReader, InOptions);
	}

	return false;
}
#endif