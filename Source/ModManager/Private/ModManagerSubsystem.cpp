// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerSubsystem.h"

#include "ModManager.h"
#include "ModManagerLibrary.h"
#include "ModManagerUtils.h"

TArray<FString> UModManagerSubsystem::GetAssetsInPak(FString PakFileName)
{
	for (const auto Pak : LoadedPakFiles)
	{
		if (Pak.PakName == PakFileName)
		{
			UE_LOG(LogModManager, Log, TEXT("Output files in pak file %s."), *PakFileName);
			return Pak.PakContents;
		}
	}

	UE_LOG(LogModManager, Log, TEXT("Pak file %s not found."), *PakFileName);
	return TArray<FString>();
}

void UModManagerSubsystem::OnPakFileMounted(const IPakFile& PakFile)
{
	UE_LOG(LogModManager, Log, TEXT("Pak file %s added to mod manager list. "), *PakFile.PakGetPakFilename());

	FPakFileContentsIterator Iterator {PakFile.PakGetPakFilename()};
	PakFile.PakVisitPrunedFilenames(Iterator);
#if ENGINE_MAJOR_VERSION == 5
	LoadedPakFiles.Add(FPakFileContents(PakFile.PakGetPakFilename(), Iterator.OutFileNames));
#else
	auto NewPakContents = FPakFileContents();
	NewPakContents.PakName = PakFile.PakGetPakFilename();
	NewPakContents.PakContents = Iterator.OutFileNames;
	LoadedPakFiles.Add(NewPakContents);
#endif
}

void UModManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if ENGINE_MAJOR_VERSION == 5
	FCoreDelegates::GetOnPakFileMounted2()
	.AddUObject(this, &UModManagerSubsystem::OnPakFileMounted);
#else
	FCoreDelegates::OnPakFileMounted2.AddUObject(this, &UModManagerSubsystem::OnPakFileMounted);
#endif

	UModManagerLibrary::InitModManager();
}

void UModManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	LoadedPakFiles.Empty();
}
