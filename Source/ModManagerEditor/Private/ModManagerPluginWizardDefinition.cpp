// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerPluginWizardDefinition.h"

#include "AssetToolsModule.h"
#include "FindDirectoriesVisitor.h"
#include "GameFeatureData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Features/IPluginsEditorFeature.h"
#if ENGINE_MAJOR_VERSION != 5
#include "Misc/FileHelper.h"
#endif

#define LOCTEXT_NAMESPACE "ModManagerPluginWizard"

FModManagerPluginWizardDefinition::FModManagerPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("ModManager"))->GetBaseDir();
	TArray<FString> FoundDirs;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString TemplatesBaseDir = PluginBaseDir / TEXT("Templates");
	FFindDirectoriesVisitor FindDirectoriesVisitor(PlatformFile, FoundDirs);

	// Find the Content Only Template that ships with the plugin.
	// Download the Robo Recall Mod Kit and check the Plugins/OdinEditor code for how to build and use your own mod templates from your game content

	// The base template that will be included with/for all created mods
	FPluginTemplateDescription* BackingTemplateDesc = new FPluginTemplateDescription(FText::FromString("Base Mod"), FText::FromString("Empty Mod"), TEXT("BaseTemplate"), true, EHostType::Runtime);
#if ENGINE_MAJOR_VERSION == 5
	BackingTemplateDesc->SortPriority = 100;
	BackingTemplateDesc->bCanBePlacedInEngine = false;
#endif
	BackingTemplate = MakeShareable(BackingTemplateDesc);
	BackingTemplatePath = PluginBaseDir / TEXT("Templates") / BackingTemplate->OnDiskPath;
	TemplateDefinitions.Add(BackingTemplate.ToSharedRef());

	SelectedTemplates.Empty();
	SelectedTemplates.Add(BackingTemplate);

	// Find all additional mod templates and add them to the list of available selections
	FindTemplates();
}

void FModManagerPluginWizardDefinition::FindTemplates()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("ModManager"))->GetBaseDir();
	TArray<FString> FoundDirs;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString TemplatesBaseDir = PluginBaseDir / TEXT("Templates");
	FFindDirectoriesVisitor FindDirectoriesVisitor(PlatformFile, FoundDirs);

	PlatformFile.IterateDirectory(*TemplatesBaseDir, FindDirectoriesVisitor);

	for (FString TemplateDirPath : FoundDirs)
	{
		FString TemplateDir = FPaths::GetCleanFilename(TemplateDirPath);

		// exclude template directories starting with tow underscores (we consider those "disabled") and the base template, as we already added that one
		if ((!TemplateDir.StartsWith("__")) && (!TemplateDir.Equals("BaseTemplate")))
		{
			TArray<FString> TemplateDetails;
			FString TemplateName = FName::NameToDisplayString(TemplateDir, false);
			FString TemplateDescription = TEXT("");
			int32 TemplateSortPriority = 100;
			TSharedPtr<FPluginTemplateDescription> TemplateDescShrPtr;
			FString TemplateDetailsFilePath = FString::Printf(TEXT("%s.txt"), *(TemplatesBaseDir / TemplateDir));

			// see if we have a .TXT file supplying us with better details for the template
			TemplateDetails.Empty();
			FFileHelper::LoadFileToStringArray(TemplateDetails, *TemplateDetailsFilePath);
			if (TemplateDetails.Num() >= 1)
			{
				TemplateName = TemplateDetails[0];
			}
			if (TemplateDetails.Num() >= 2)
			{
				TemplateDescription = TemplateDetails[1];
			}
			if (TemplateDetails.Num() >= 3)
			{
				TemplateSortPriority = FCString::Atoi(*TemplateDetails[2]);
			}

			FPluginTemplateDescription* TemplateDesc = new FPluginTemplateDescription(
				FText::FromString(TemplateName),
				FText::FromString(TemplateDescription),
				TemplateDir,
				true,
				EHostType::Runtime);
#if ENGINE_MAJOR_VERSION == 5
			TemplateDesc->SortPriority = TemplateSortPriority;  // this seem to do nothing?!
			TemplateDesc->bCanBePlacedInEngine = false;
#endif
			TemplateDescShrPtr = MakeShareable(TemplateDesc);

			TemplateDefinitions.Add(TemplateDescShrPtr.ToSharedRef());
		}
	}
}

const TArray<TSharedRef<FPluginTemplateDescription>>& FModManagerPluginWizardDefinition::GetTemplatesSource() const
{
	return TemplateDefinitions;
}

#if ENGINE_MAJOR_VERSION == 5
void FModManagerPluginWizardDefinition::OnTemplateSelectionChanged(
	TSharedPtr<FPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo)
{
	SelectedTemplates.Empty();
	SelectedTemplates.Add(InSelectedItem);
}
#else
void FModManagerPluginWizardDefinition::OnTemplateSelectionChanged(
	TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo)
{
	SelectedTemplates.Empty();
	SelectedTemplates.Append(InSelectedItems);
}
#endif

#if ENGINE_MAJOR_VERSION == 5
TSharedPtr<FPluginTemplateDescription> FModManagerPluginWizardDefinition::GetSelectedTemplate() const
{
	if (SelectedTemplates.Num() > 0)
	{
		return SelectedTemplates[0];
	}

	return BackingTemplate;
}
#else
TArray<TSharedPtr<FPluginTemplateDescription>> FModManagerPluginWizardDefinition::GetSelectedTemplates() const
{
	if (SelectedTemplates.Num() > 0)
	{
		return SelectedTemplates;
	}

	return {BackingTemplate};
}
#endif

void FModManagerPluginWizardDefinition::ClearTemplateSelection()
{
	SelectedTemplates.Empty();
}

bool FModManagerPluginWizardDefinition::HasValidTemplateSelection() const
{
	// A mod should be created even if no templates are actually selected
	return true;
}

bool FModManagerPluginWizardDefinition::HasModules() const
{
	bool bHasModules = false;

	for (TSharedPtr<FPluginTemplateDescription> Template : SelectedTemplates)
	{
		if (FPaths::DirectoryExists(PluginBaseDir / TEXT("Templates") / Template->OnDiskPath / TEXT("Source")))
		{
			bHasModules = true;
			break;
		}
	}

	return bHasModules;
}

bool FModManagerPluginWizardDefinition::IsMod() const
{
	if (SelectedTemplates.IsValidIndex(0))
	{
		if (SelectedTemplates[0].Get()->Name.ToString() == "Game Feature Plugin Content Only")
		{
			return false;
		}
	}
	
	return true;
}

void FModManagerPluginWizardDefinition::OnShowOnStartupCheckboxChanged(ECheckBoxState CheckBoxState)
{
}

ECheckBoxState FModManagerPluginWizardDefinition::GetShowOnStartupCheckBoxState() const
{
	return ECheckBoxState();
}

FText FModManagerPluginWizardDefinition::GetInstructions() const
{
	return LOCTEXT("CreateNewModPanel", "Choose a mod type to create, fill out the details like name, etc.\nand hit the \"Create Mod\" button.");
}

bool FModManagerPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	// Replace this file with your own 128x128 image if desired.
	OutIconPath = BackingTemplatePath / TEXT("Resources/Icon128.png");
	return false;
}

EHostType::Type FModManagerPluginWizardDefinition::GetPluginModuleDescriptor() const
{
	return BackingTemplate->ModuleDescriptorType;
}

ELoadingPhase::Type FModManagerPluginWizardDefinition::GetPluginLoadingPhase() const
{
	return BackingTemplate->LoadingPhase;
}

bool FModManagerPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate,
	FString& OutIconPath) const
{
	FString TemplateName = InTemplate->Name.ToString();

	OutIconPath = PluginBaseDir / TEXT("Resources");

	if (TemplateToIconMap.Contains(TemplateName))
	{
		OutIconPath /= TemplateToIconMap[TemplateName];
	}
	else
	{
		// Couldn't find a suitable icon to use for this template, so use the default one instead
		OutIconPath /= TEXT("Icon128.png");
	}

	return false;
}

FString FModManagerPluginWizardDefinition::GetPluginFolderPath() const
{
	return BackingTemplatePath;
}

TArray<FString> FModManagerPluginWizardDefinition::GetFoldersForSelection() const
{
	TArray<FString> SelectedFolders;
	SelectedFolders.Add(BackingTemplatePath); // This will always be a part of the mod plugin

	for (TSharedPtr<FPluginTemplateDescription> Template : SelectedTemplates)
	{
		SelectedFolders.AddUnique(PluginBaseDir / TEXT("Templates") / Template->OnDiskPath);
	}

	return SelectedFolders;
}

void FModManagerPluginWizardDefinition::PluginCreated(const FString& PluginName, bool bWasSuccessful) const
{
	// Override Category to Mods
	if (bWasSuccessful)
	{
		TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (Plugin != nullptr)
		{
			FPluginDescriptor Desc = Plugin->GetDescriptor();
			if (IsMod())
			{
				Desc.Category = "Mods";
			}
			else
			{
				Desc.Category = "Game Features";
				CreateGameFeatureDataAsset(Plugin);
			}
			FText UpdateFailureText;
			Plugin->UpdateDescriptor(Desc, UpdateFailureText);
		}
	}
}

#if ENGINE_MAJOR_VERSION == 4
ESelectionMode::Type FModManagerPluginWizardDefinition::GetSelectionMode() const
{
	return ESelectionMode::Single;
}

bool FModManagerPluginWizardDefinition::AllowsEnginePlugins() const
{
	return false;
}

bool FModManagerPluginWizardDefinition::CanContainContent() const
{
	return true;
}
#endif

TSharedPtr<class SWidget> FModManagerPluginWizardDefinition::GetCustomHeaderWidget()
{
	if (!CustomHeaderWidget.IsValid())
	{
		FString IconPath;
		GetPluginIconPath(IconPath);

		const FName BrushName(*IconPath);
		const FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(BrushName);
		if ((Size.X > 0) && (Size.Y > 0))
		{
			IconBrush = MakeShareable(new FSlateDynamicImageBrush(BrushName, FVector2D(Size.X, Size.Y)));
		}

		CustomHeaderWidget = SNew(SHorizontalBox)
			// Header image
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .Padding(4.0f)
			[
				SNew(SBox)
				.WidthOverride(80.0f)
				.HeightOverride(80.0f)
				[
					SNew(SImage)
					.Image(IconBrush.IsValid() ? IconBrush.Get() : nullptr)
				]
			];
	}

	return CustomHeaderWidget;
}

void FModManagerPluginWizardDefinition::CreateGameFeatureDataAsset(const TSharedPtr<IPlugin>& Plugin)
{
	// If the template includes an existing game feature data, do not create a new one.
	TArray<FAssetData> ObjectList;
	FARFilter AssetFilter;
	AssetFilter.ClassNames.Add(UGameFeatureData::StaticClass()->GetFName());
	AssetFilter.PackagePaths.Add(FName(Plugin->GetMountedAssetPath()));
	AssetFilter.bRecursiveClasses = true;
	AssetFilter.bRecursivePaths = true;

	IAssetRegistry::GetChecked().GetAssets(AssetFilter, ObjectList);

	UObject* GameFeatureDataAsset = nullptr;

	if (ObjectList.Num() <= 0)
	{
		// Create the game feature data asset
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FString const& AssetName = Plugin->GetName();
		GameFeatureDataAsset = AssetToolsModule.Get().CreateAsset(AssetName, Plugin->GetMountedAssetPath(), 
			UGameFeatureData::StaticClass(), /*Factory=*/ nullptr);
	}
	else
	{
		GameFeatureDataAsset = ObjectList[0].GetAsset();
	}
}

#undef LOCTEXT_NAMESPACE