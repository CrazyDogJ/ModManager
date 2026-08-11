// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(ModManagerPackager, Log, All);

struct FModManagerCommand
{
	TSharedPtr<class IPlugin> PluginInfo;
	TSharedPtr<class FUICommandInfo> CommandInfo;
};

class MODMANAGEREDITOR_API FModManagerPackager : public TSharedFromThis<FModManagerPackager>
{
public:
	FModManagerPackager();
	~FModManagerPackager();

	void OpenPluginPackager(TSharedRef<class IPlugin> Plugin);

	void PackagePlugin(TSharedRef<class IPlugin> Plugin, const FString& OutputDirectory);

	/** Generates submenu content for the plugin packager command */
	void GeneratePackagerMenuContent(class FMenuBuilder& MenuBuilder);

	/** Generates the menu content for the plugin packager toolbar button */
	TSharedRef<class SWidget> GeneratePackagerComboButtonContent();

private:
	/** Gets all available game mod plugin packages  */
	void FindAvailableGameMods(TArray<TSharedRef<class IPlugin>>& OutAvailableGameMods);

	/** Gets all available game mod plugins and registers command info for them */
	void GetAvailableModCommands(const TArray<TSharedRef<class IPlugin>>& AvailableMod);

	/** Generates menu content for the supplied set of commands */
	void GeneratePackagerMenuContent_Internal(class FMenuBuilder& MenuBuilder, const TArray<TSharedPtr<FUICommandInfo>>& Commands);

	/**
 	 * Checks if a plugin has any unsaved content
 	 *
	 * @param	Plugin			The plugin to check for unsaved content
	 * @return	True if all mod content has been saved, false otherwise
	 */
	bool IsAllContentSaved(TSharedRef<class IPlugin> Plugin);

	/**
	 * Gets all the files in a given directory.
	 * From: https://forums.unrealengine.com/t/how-to-get-file-list-in-a-directory/315248
	 *
	 * @param Directory The full path of the directory we want to iterate over.
	 * @param FullPath Whether the returned list should be the full file paths or just the filenames.
	 * @param OnlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
	 * @param OnlyFilesWithExtension Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
	 * @return A list of files (including the extension).
	 */
	TArray<FString> GetAllFilesInDirectory(const FString Directory, const bool FullPath, const FString OnlyFilesStartingWith, const FString OnlyFilesWithExtension);

	/**
	 * Disable other mod plugins before we make a mod package.
	 * @param ExcludePlugin The plugin that we are packaging.
	 */
	void DisableOtherModPlugins(TSharedRef<IPlugin> ExcludePlugin);

	/**
	 * Enable other mod plugins that disabled at last uat task.
	 */
	void EnableLastModPlugins();
	
private:
	TArray<TSharedPtr<class FUICommandInfo>> ModCommands;
	TArray<FString> DisabledModPlugins;
};
