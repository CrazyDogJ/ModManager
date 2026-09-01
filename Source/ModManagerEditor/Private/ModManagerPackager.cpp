// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerPackager.h"

#include "DesktopPlatformModule.h"
#include "FileHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "IDesktopPlatform.h"
#include "IUATHelperModule.h"
#include "JsonObjectConverter.h"
#include "ModManagerEditor.h"
#include "ModManagerEditorCommands.h"
#include "ProjectDescriptor.h"
#include "SModPackagerDialog.h"
#include "Interfaces/IMainFrameModule.h"
#include "Interfaces/IProjectManager.h"
#if ENGINE_MAJOR_VERSION == 4
#include "ModManagerLibrary.h"
#include "Misc/FileHelper.h"
#endif
#include "ModPackagerSettingsProxy.h"
#include "Misc/LocalTimestampDirectoryVisitor.h"

#define LOCTEXT_NAMESPACE "ModManagerPackager"

DEFINE_LOG_CATEGORY(ModManagerPackager);

FModManagerPackager::FModManagerPackager()
{
}

FModManagerPackager::~FModManagerPackager()
{
}

void FModManagerPackager::OpenPluginPackager(TSharedRef<IPlugin> Plugin)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	FString DefaultDirectory = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	FString OutputDirectory;

	// Prompt the user to save all dirty packages. We'll ensure that if any packages from the mod that the user wants to
	// package are dirty that they will not be able to save them.

	if (!IsAllContentSaved(Plugin))
	{
		FEditorFileUtils::SaveDirtyPackages(true, true, true);
	}

	if (IsAllContentSaved(Plugin))
	{
		void* ParentWindowWindowHandle = nullptr;
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		auto NewInfo = MakeModInfoFromPlugin(Plugin);
		
		ETargetPlatform TargetPlatform;
		if (ShowModSettingsWindow(NewInfo, TargetPlatform))
		{
			if (DesktopPlatform->OpenDirectoryDialog(ParentWindowWindowHandle, LOCTEXT("SelectOutputFolderTitle", "Select mod output directory:").ToString(), DefaultDirectory, OutputDirectory))
			{
				PackagePlugin(Plugin, OutputDirectory, NewInfo, TargetPlatform);
			}
		}
	}
	else
	{
		FText PackageModError = FText::Format(LOCTEXT("PackageModError_UnsavedContent", "You must save all assets in {0} before you can share it."), FText::FromString(Plugin->GetName()));

		FMessageDialog::Open(EAppMsgType::Ok, PackageModError);
	}
}

bool FModManagerPackager::ShowModSettingsWindow(FModInfo& OutSettings, ETargetPlatform& OutTargetPlatform)
{
	UModPackagerSettingsProxy* Proxy = NewObject<UModPackagerSettingsProxy>();
	Proxy->Settings = OutSettings;
	Proxy->AddToRoot(); // 防止垃圾回收
 
	TSharedRef<SWindow> ModalWindow = SNew(SWindow)
		.Title(FText::FromString("Mod Settings"))
		.ClientSize(FVector2D(500, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false);
 
	TSharedRef<SModPackagerDialog> DialogWidget = SNew(SModPackagerDialog)
		.SettingsProxy(Proxy)
		.ParentWindow(ModalWindow);
 
	ModalWindow->SetContent(DialogWidget);
 
	// 关键：以模态形式添加窗口
	GEditor->EditorAddModalWindow(ModalWindow);
 
	bool bSuccess = false;
	if (DialogWidget->bUserConfirmed)
	{
		OutSettings = Proxy->Settings;
		OutTargetPlatform = Proxy->TargetPlatform;
		bSuccess = true;
	}
 
	Proxy->RemoveFromRoot();
	return bSuccess;
}

bool FModManagerPackager::IsAllContentSaved(TSharedRef<IPlugin> Plugin)
{
	bool bAllContentSaved = true;

	TArray<UPackage*> UnsavedPackages;
	FEditorFileUtils::GetDirtyContentPackages(UnsavedPackages);
	FEditorFileUtils::GetDirtyWorldPackages(UnsavedPackages);

	if (UnsavedPackages.Num() > 0)
	{
		FString PluginBaseDir = Plugin->GetBaseDir();

		for (UPackage* Package : UnsavedPackages)
		{
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename))
			{
				if (PackageFilename.Find(PluginBaseDir) == 0)
				{
					bAllContentSaved = false;
					break;
				}
			}
		}
	}

	return bAllContentSaved;
}

void FModManagerPackager::PackagePlugin(TSharedRef<class IPlugin> Plugin, const FString& OutputDirectory, const FModInfo& InModInfo, const ETargetPlatform& TargetPlatform)
{
#if PLATFORM_WINDOWS
	FText PlatformName = LOCTEXT("PlatformName_Windows", "Win64");
#elif PLATFORM_MAC
	FText PlatformName = LOCTEXT("PlatformName_Mac", "Mac");
#elif PLATFORM_LINUX
	FText PlatformName = LOCTEXT("PlatformName_Linux", "Linux");
#else
	FText PlatformName = LOCTEXT("PlatformName_Desktop", "Desktop");
#endif

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString ReleaseVersion;
	FString GameModuleName;

	FString ProjectDir = FPaths::GetPath(FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
	FString ReleasesDir = FString::Printf(TEXT("%s/Releases"), *ProjectDir);

	// look for any AssetRegistry.bin files to find the release directory to base this packaging on
	TArray<FString> AssetRegistries = GetAllFilesInDirectory(ReleasesDir, true, TEXT("AssetRegistry"), TEXT(""));
	if( AssetRegistries.Num() < 1 )
	{
		UE_LOG(ModManagerPackager, Error, TEXT("Failed to find an AssetRegistry to determine the release data to use!  Make sure you propeerly set up [ProjectDir]/Releases as described in the documentation."));
		return;
	}

	// fetch first dir name under the Releases dir from the first AssetRegistry found, to determine the release data to use for this packaging
	TArray<FString> AssetRegistryPathSegments;
	AssetRegistries[0].Replace(*ReleasesDir, TEXT("")).TrimChar(*TEXT("/")).ParseIntoArray(AssetRegistryPathSegments, TEXT("/"));
	ReleaseVersion = AssetRegistryPathSegments[0];
	UE_LOG(ModManagerPackager, Display, TEXT("***** Basing mod packaging on release: %s *****"), *ReleaseVersion);

	// determine the game module, the mods are packaged for - VERY simple implementation, this only works if you have ONE module (or the first one is the right one)
	const FProjectDescriptor* Project = IProjectManager::Get().GetCurrentProject();

	if( Project->Modules.Num() > 1 )
	{
		UE_LOG(ModManagerPackager, Warning, TEXT("There are more than one modules in this project, you might want to doublecheck if we're using the right one!"));
	}

	GameModuleName = Project->Modules[0].Name.ToString();
	UE_LOG(ModManagerPackager, Display, TEXT("***** Determined game module name mods are supposingly packaged for: %s *****"), *GameModuleName);

	// set up some path names where things are going
	FString StagePath = FString::Printf(TEXT("%s/Temp"), *OutputDirectory);

	// clean a possibly left over staging area
	PlatformFile.DeleteDirectoryRecursively(*StagePath);

	const auto PluginName = Plugin->GetName();
	
	// Target platform string.
	FString TargetPlatformString;
	switch (TargetPlatform)
	{
	case ETargetPlatform::Win64: TargetPlatformString = TEXT("Win64");
		break;
	case ETargetPlatform::Win32: TargetPlatformString = TEXT("Win32");
		break;
	case ETargetPlatform::Mac: TargetPlatformString = TEXT("Mac");
		break;
	case ETargetPlatform::Linux: TargetPlatformString = TEXT("Linux");
		break;
	}
	
	// UAT command for packaging our mod
	FString CommandLine = FString::Printf(TEXT(
		"BuildCookRun"
		" -project=\"%s\""
		" -dlcname=\"%s\""
		" -basedonreleaseversion=\"%s\""
		" -archivedirectory=\"%s\""
		" -targetplatform=\"%s\""
		" -DLCIncludeEngineContent -DLCPakPluginFile -nodebuginfo"
		" -noP4"
		// UAT should be compiled already
		" -nocompile -nocompileeditor"
		// Avoid encrypt
		" -noencrypt -encryptini=false -nongryption"
		// Avoid cook all
		" -cook -stage -package -pak -archive -compressed -distribution"
		" -clientconfig=Development -serverconfig=Development"),
	                                      *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()),
	                                      *PluginName,
	                                      *ReleaseVersion,
	                                      *StagePath,
	                                      *TargetPlatformString
	);

	FText PackagingText = FText::Format(LOCTEXT("ModManagerEditor_PackagePluginTaskName", "Packaging {0}"), FText::FromString(Plugin->GetName()));

	// Disable other plugins before create uat task.
	DisableOtherModPlugins(Plugin);
	
	// Disable encryto when packaging mods
	DisableEncryption();
	
	FString FriendlyName = Plugin->GetDescriptor().FriendlyName;
	IUATHelperModule::Get().CreateUatTask(CommandLine, PlatformName, PackagingText,
	    PackagingText, FModManagerEditorStyle::Get().GetBrush(TEXT("ModManagerEditor.PackageModAction")), 
#if ENGINE_MAJOR_VERSION == 5
	    nullptr,
#else
#endif
	    [OutputDirectory, StagePath, this, PluginName, InModInfo]
		(FString TaskResult, double TimeSec)
	    {
	        // find all paks and move to needed folder.
	        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	        const TArray<FString> FinalFiles = UModManagerLibrary::GetAllPaksInPath(StagePath, true);
	    	
	    	const FString Path = OutputDirectory + "/" + PluginName;
	        if (!PlatformFile.DirectoryExists(*Path))
	        {
	        	PlatformFile.CreateDirectory(*Path);
	        }
	    	
	        for (const auto ModPackageFile : FinalFiles)
	        {
	        	const auto FileName = FPaths::GetCleanFilename(ModPackageFile);
	        	PlatformFile.CopyFile(*(Path + "/" + FileName), *ModPackageFile, EPlatformFileRead::None, 
#if ENGINE_MAJOR_VERSION == 5
	        		EPlatformFileWrite::AttemptDeleteAndCreate
#else
	        		EPlatformFileWrite::AllowRead
#endif
	        		);
	        }
	    	
	    	// Save to json file.
	    	FString OutputJsonString;
	    	FJsonObjectConverter::UStructToJsonObjectString<FModInfo>(InModInfo, OutputJsonString);
	    	
	    	const auto ModInfoFileName = UModManagerLibrary::GetModInfoFileName();
	    	const FString ModJsonFile = Path + "/" + ModInfoFileName;
	    	FFileHelper::SaveStringToFile(OutputJsonString, *ModJsonFile);
	    	
	    	// Delete temp folders.
	        PlatformFile.DeleteDirectoryRecursively(*StagePath);
	    	// Enable other mod plugins after we finish uat task.
	    	EnableLastModPlugins();
	    	RestoreEncryptionSettings();
	    });
}

void FModManagerPackager::FindAvailableGameMods(TArray<TSharedRef<IPlugin>>& OutAvailableGameMods)
{
	OutAvailableGameMods.Empty();

	// Find available game mods from the list of discovered plugins

	for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		// All game project plugins that are marked as mods are valid
		if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Project && 
			(Plugin->GetType() == EPluginType::Mod || 
			Plugin->GetDescriptorFileName().Contains(TEXT("/GameFeatures/"))))
		{
			UE_LOG(LogModManagerEditor, Display, TEXT("Adding %s"), *Plugin->GetName());
			OutAvailableGameMods.AddUnique(Plugin);
		}
	}
}

void FModManagerPackager::GeneratePackagerMenuContent_Internal(class FMenuBuilder& MenuBuilder, const TArray<TSharedPtr<FUICommandInfo>>& Commands)
{
	for (TSharedPtr<FUICommandInfo> Command : Commands)
	{
		MenuBuilder.AddMenuEntry(Command, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FModManagerEditorStyle::GetStyleSetName(), "ModManagerEditor.Folder"));
	}
}

void FModManagerPackager::GeneratePackagerMenuContent(class FMenuBuilder& MenuBuilder)
{
	TArray<TSharedRef<IPlugin>> AvailableGameMods;
	FindAvailableGameMods(AvailableGameMods);
	GetAvailableModCommands(AvailableGameMods);

	// Regenerate the action list
	TSharedPtr<FUICommandList> GameModActionsList = MakeShareable(new FUICommandList);

	for (int32 Index = 0; Index < ModCommands.Num(); ++Index)
	{
		GameModActionsList->MapAction(
			ModCommands[Index],
			FExecuteAction::CreateRaw(this, &FModManagerPackager::OpenPluginPackager, AvailableGameMods[Index]),
			FCanExecuteAction()
		);
	}

	TArray<TSharedPtr<FUICommandInfo>> Commands;

	MenuBuilder.PushCommandList(GameModActionsList.ToSharedRef());
	GeneratePackagerMenuContent_Internal(MenuBuilder, ModCommands);
	MenuBuilder.PopCommandList();
}

TSharedRef<SWidget> FModManagerPackager::GeneratePackagerComboButtonContent()
{
	// Regenerate the game mod commands
	TArray<TSharedRef<IPlugin>> AvailableGameMods;
	FindAvailableGameMods(AvailableGameMods);

	GetAvailableModCommands(AvailableGameMods);

	// Regenerate the action list
	TSharedPtr<FUICommandList> GameModActionsList = MakeShareable(new FUICommandList);

	for (int32 Index = 0; Index < ModCommands.Num(); ++Index)
	{
		GameModActionsList->MapAction(
			ModCommands[Index],
			FExecuteAction::CreateRaw(this, &FModManagerPackager::OpenPluginPackager, AvailableGameMods[Index]),
			FCanExecuteAction()
		);
	}

	// Show the drop down menu
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, GameModActionsList);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("PackageMod", "Share..."));
	{
		GeneratePackagerMenuContent_Internal(MenuBuilder, ModCommands);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FModManagerPackager::GetAvailableModCommands(const TArray<TSharedRef<IPlugin>>& AvailableMod)
{
	if (ModCommands.Num() > 0)
	{
		// Unregister UI Commands
		FModManagerEditorCommands::Get().UnregisterModCommands(ModCommands);
	}
	ModCommands.Empty(AvailableMod.Num());

	ModCommands = FModManagerEditorCommands::Get().RegisterModCommands(AvailableMod);
}

TArray<FString> FModManagerPackager::GetAllFilesInDirectory(const FString Directory, const bool FullPath, const FString OnlyFilesStartingWith, const FString OnlyFilesWithExtension)
{
	// Get all files in directory
	TArray<FString> DirectoriesToSkip;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToSkip, false);
	PlatformFile.IterateDirectory(*Directory, Visitor);
	TArray<FString> Files;

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString FilePath = TimestampIt.Key();
		const FString FileName = FPaths::GetCleanFilename(FilePath);
		bool ShouldAddFile = true;

		// Check if filename starts with required characters
		if (!OnlyFilesStartingWith.IsEmpty())
		{
			const FString left = FileName.Left(OnlyFilesStartingWith.Len());

			if (!(FileName.Left(OnlyFilesStartingWith.Len()).Equals(OnlyFilesStartingWith, ESearchCase::IgnoreCase)))
			{
				ShouldAddFile = false;
			}
		}

		// Check if file extension is required characters
		if (!OnlyFilesWithExtension.IsEmpty())
		{
			if (!(FPaths::GetExtension(FileName, false).Equals(OnlyFilesWithExtension, ESearchCase::IgnoreCase)))
			{
				ShouldAddFile = false;
			}
		}

		// Add full path to results
		if (ShouldAddFile)
		{
			Files.Add(FullPath ? FilePath : FileName);
		}
	}

	return Files;
}

void FModManagerPackager::DisableOtherModPlugins(TSharedRef<IPlugin> ExcludePlugin)
{
	DisabledModPlugins.Empty();
	
	for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Project &&
			Plugin->GetType() == EPluginType::Mod &&
			Plugin != ExcludePlugin &&
			Plugin->IsEnabled())
		{
			DisabledModPlugins.Add(Plugin->GetName());
			FText OutResult;
			IProjectManager::Get().SetPluginEnabled(Plugin->GetName(), false, OutResult);
			// Try to save the project file if needed
			if (IProjectManager::Get().IsCurrentProjectDirty())
			{
				IProjectManager::Get().SaveCurrentProjectToDisk(OutResult);
			}
		}
	}
}

void FModManagerPackager::DisableEncryption()
{
	const auto DefaultCryptoKeysSettings = GetMutableDefault<UCryptoKeysSettings>();
	bEncryptPakIniFiles = DefaultCryptoKeysSettings->bEncryptPakIniFiles;
	bEncryptAllAssetFiles = DefaultCryptoKeysSettings->bEncryptAllAssetFiles;
	bEncryptPakIndex = DefaultCryptoKeysSettings->bEncryptPakIndex;
	bEncryptUAssetFiles = DefaultCryptoKeysSettings->bEncryptUAssetFiles;
	EncryptionString = DefaultCryptoKeysSettings->EncryptionKey;
	SecondaryEncryptionKeys = DefaultCryptoKeysSettings->SecondaryEncryptionKeys;
	DefaultCryptoKeysSettings->bEncryptAllAssetFiles = false;
	DefaultCryptoKeysSettings->bEncryptPakIndex = false;
	DefaultCryptoKeysSettings->bEncryptUAssetFiles = false;
	DefaultCryptoKeysSettings->bEncryptPakIniFiles = false;
	DefaultCryptoKeysSettings->EncryptionKey = FString();
	DefaultCryptoKeysSettings->SecondaryEncryptionKeys = {};
	DefaultCryptoKeysSettings->SaveConfig(CPF_Config, *GetConfigFilename(DefaultCryptoKeysSettings));
}

void FModManagerPackager::EnableLastModPlugins()
{
	for (auto Itr : DisabledModPlugins)
	{
		FText OutResult;
		IProjectManager::Get().SetPluginEnabled(Itr, true, OutResult);
		// Try to save the project file if needed
		if (IProjectManager::Get().IsCurrentProjectDirty())
		{
			IProjectManager::Get().SaveCurrentProjectToDisk(OutResult);
		}
	}

	DisabledModPlugins.Empty();
}

void FModManagerPackager::RestoreEncryptionSettings() const
{
	const auto DefaultCryptoKeysSettings = GetMutableDefault<UCryptoKeysSettings>();
	DefaultCryptoKeysSettings->bEncryptAllAssetFiles = bEncryptAllAssetFiles;
	DefaultCryptoKeysSettings->bEncryptPakIndex = bEncryptPakIndex;
	DefaultCryptoKeysSettings->bEncryptUAssetFiles = bEncryptUAssetFiles;
	DefaultCryptoKeysSettings->bEncryptPakIniFiles = bEncryptPakIniFiles;
	DefaultCryptoKeysSettings->EncryptionKey = EncryptionString;
	DefaultCryptoKeysSettings->SecondaryEncryptionKeys = SecondaryEncryptionKeys;
	DefaultCryptoKeysSettings->SaveConfig(CPF_Config, *GetConfigFilename(DefaultCryptoKeysSettings));
}

FModInfo FModManagerPackager::MakeModInfoFromPlugin(TSharedRef<IPlugin> Plugin)
{
	FModInfo ModInfo;
	
	ModInfo.ModName = Plugin->GetName();
	ModInfo.Author = Plugin->GetDescriptor().CreatedBy;
	ModInfo.Description = Plugin->GetDescriptor().Description;
	ModInfo.Version = FString::Printf(TEXT("%d"), Plugin->GetDescriptor().Version);
	ModInfo.ModPluginName = Plugin->GetName();
	// Add dependencies.
	for (FPluginReferenceDescriptor Descriptor : Plugin->GetDescriptor().Plugins)
	{
		ModInfo.Dependencies.Add(Descriptor.Name);
	}
	
	if (Plugin->GetType() == EPluginType::Mod)
	{
		ModInfo.CustomRelativePath = TEXT("Mods/") + ModInfo.ModPluginName + TEXT("/");
	}
	else if (Plugin->GetDescriptorFileName().Contains(TEXT("/GameFeatures/")))
	{
		ModInfo.CustomRelativePath = TEXT("Plugins/GameFeatures/") + ModInfo.ModPluginName + TEXT("/");
	}
	
	return ModInfo;
}

#undef LOCTEXT_NAMESPACE
