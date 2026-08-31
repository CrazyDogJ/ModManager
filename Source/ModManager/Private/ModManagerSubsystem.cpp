// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerSubsystem.h"

// #include "ModManager.h"
// #include "ModManagerUtils.h"

// TArray<FString> UModManagerSubsystem::GetAssetsInPak(FString PakFileName)
// {
// 	for (const auto Pak : LoadedPakFiles)
// 	{
// 		if (Pak.PakName == PakFileName)
// 		{
// 			UE_LOG(LogModManager, Log, TEXT("Output files in pak file %s."), *PakFileName);
// 			return Pak.PakContents;
// 		}
// 	}
// 
// 	UE_LOG(LogModManager, Log, TEXT("Pak file %s not found."), *PakFileName);
// 	return TArray<FString>();
// }

bool UModManagerSubsystem::MountModPaks(const FString ModInfoPath)
{
	if (MountedMods.Find(ModInfoPath))
	{
		return false;
	}
	
	FModInfo OutModInfo;
	if (UModManagerLibrary::LoadModInfoFromJson(ModInfoPath, OutModInfo))
	{
		// Mod info enabled bool for unexpected issue.
		if (OutModInfo.Enabled)
		{
			UModManagerLibrary::MountModPaks(OutModInfo);
			MountedMods.Add(ModInfoPath, OutModInfo);
			OnModMounted.Broadcast(ModInfoPath, OutModInfo);
			return true;
		}
	}
	
	return false;
}

bool UModManagerSubsystem::UnmountModPaks(const FString ModInfoPath)
{
	if (const auto FoundModInfo = MountedMods.Find(ModInfoPath))
	{
		const auto ModInfoPure = *FoundModInfo;
		UModManagerLibrary::UnmountModPaks(ModInfoPure);
		OnModUnmounted.Broadcast(ModInfoPath, ModInfoPure);
		
		MountedMods.Remove(ModInfoPath);
		return true;
	}
	
	return false;
}

bool UModManagerSubsystem::FindMountedModByModInfoPath(FModInfo& OutInfo, const FString ModInfoPath) const
{
	if (const auto Found = MountedMods.Find(ModInfoPath))
	{
		OutInfo = *Found;
		return true;
	}
	
	OutInfo = FModInfo();
	return false;
}

bool UModManagerSubsystem::FindMountedModByModPluginName(FModInfo& OutInfo, const FString ModPluginName) const
{
	for (const auto ModInfoPair : MountedMods)
	{
		if (ModInfoPair.Value.ModPluginName == ModPluginName)
		{
			OutInfo = ModInfoPair.Value;
			return true;
		}
	}
	
	OutInfo = FModInfo();
	return false;
}

FString UModManagerSubsystem::TryGetUrlByModInfoPath(const FString ModInfoPath) const
{
	FModInfo OutModInfo;
	if (FindMountedModByModInfoPath(OutModInfo, ModInfoPath))
	{
		return OutModInfo.URL;
	}
	
	return FString();
}

FString UModManagerSubsystem::TryGetUrlByModPluginName(const FString ModPluginName) const
{
	FModInfo OutModInfo;
	if (FindMountedModByModPluginName(OutModInfo, ModPluginName))
	{
		return OutModInfo.URL;
	}
	
	return FString();
}

// void UModManagerSubsystem::OnPakFileMounted(const IPakFile& PakFile)
// {
// 	UE_LOG(LogModManager, Log, TEXT("Pak file %s added to mod manager list. "), *PakFile.PakGetPakFilename());
// 
// 	FPakFileContentsIterator Iterator {PakFile.PakGetPakFilename()};
// 	PakFile.PakVisitPrunedFilenames(Iterator);
// #if ENGINE_MAJOR_VERSION == 5
// 	LoadedPakFiles.Add(FPakFileContents(PakFile.PakGetPakFilename(), Iterator.OutFileNames));
// #else
// 	auto NewPakContents = FPakFileContents();
// 	NewPakContents.PakName = PakFile.PakGetPakFilename();
// 	NewPakContents.PakContents = Iterator.OutFileNames;
// 	LoadedPakFiles.Add(NewPakContents);
// #endif
// }

void UModManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

// #if ENGINE_MAJOR_VERSION == 5
// 	FCoreDelegates::GetOnPakFileMounted2()
// 	.AddUObject(this, &UModManagerSubsystem::OnPakFileMounted);
// #else
// 	FCoreDelegates::OnPakFileMounted2.AddUObject(this, &UModManagerSubsystem::OnPakFileMounted);
// #endif

	// Begin search.
	const auto ModsPath = UModManagerLibrary::GetModsSearchPath();
	const auto ModInfos = UModManagerLibrary::SearchModInfoFiles(ModsPath);
	
	for (const auto Itr : ModInfos)
	{
		MountModPaks(Itr);
	}
}

void UModManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// LoadedPakFiles.Empty();
}
