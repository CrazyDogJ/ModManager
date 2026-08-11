// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class FPakFileContentsIterator final : public IPlatformFile::FDirectoryVisitor
{
public:
	FString PakName;
	TArray<FString> OutFileNames;
	
	explicit FPakFileContentsIterator(const FString& InPakName) : PakName(InPakName)
	{
	}

	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		if (FString(FilenameOrDirectory).Contains(".uasset") ||
			FString(FilenameOrDirectory).Contains(".umap"))
		{
			OutFileNames.Add(FilenameOrDirectory);
		}
		
		return true;
	}
};

class FPakFileSearchVisitor final : public IPlatformFile::FDirectoryVisitor
{
	TArray<FString>& FoundFiles;

public:
	explicit FPakFileSearchVisitor(TArray<FString>& InFoundFiles) : FoundFiles(InFoundFiles) {}
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		if (bIsDirectory == false)
		{
			const FString Filename(FilenameOrDirectory);
			if (Filename.MatchesWildcard(TEXT("*.pak")) && !FoundFiles.Contains(Filename))
			{
				FoundFiles.Add(Filename);
			}
		}
		return true;
	}
};