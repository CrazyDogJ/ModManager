#include "ModManagerEditor.h"

#include "LevelEditor.h"
#include "ModManagerCreator.h"
#include "ModManagerEditorCommands.h"
#include "ModManagerEditorStyle.h"
#include "ModManagerPackager.h"

#define LOCTEXT_NAMESPACE "FModManagerEditorModule"

DEFINE_LOG_CATEGORY(LogModManagerEditor)

void FModManagerEditorModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ModCreator = MakeShared<FModManagerCreator>();
	ModPackager = MakeShared<FModManagerPackager>();

	FModManagerEditorStyle::Initialize();
	FModManagerEditorStyle::ReloadTextures();

	FModManagerEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FModManagerEditorCommands::Get().CreateModAction,
		FExecuteAction::CreateRaw(this, &FModManagerEditorModule::CreateModButtonClicked),
		FCanExecuteAction()
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	// Add commands
	{
		FName MenuSection = "FileProject";
		FName ToolbarSection = "Content";

		// Add creator button to the menu
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension(MenuSection, EExtensionHook::After, PluginCommands,
			                               FMenuExtensionDelegate::CreateRaw(
				                               this, &FModManagerEditorModule::AddModCreatorMenuExtension));

			MenuExtenders.Add(MenuExtender);
			LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}

		// Add creator button to the toolbar
		{
			TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension(ToolbarSection, EExtensionHook::After, PluginCommands,
			                                     FToolBarExtensionDelegate::CreateRaw(
				                                     this, &FModManagerEditorModule::AddModCreatorToolbarExtension));

			ToolbarExtenders.Add(ToolbarExtender);
			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}

		// Add packager button to the menu
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension(MenuSection, EExtensionHook::After, PluginCommands,
			                               FMenuExtensionDelegate::CreateRaw(
				                               this, &FModManagerEditorModule::AddModPackagerMenuExtension));

			MenuExtenders.Add(MenuExtender);
			LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}

		// Add packager button to the toolbar
		{
			TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension(ToolbarSection, EExtensionHook::After, PluginCommands,
			                                     FToolBarExtensionDelegate::CreateRaw(
				                                     this, &FModManagerEditorModule::AddModPackagerToolbarExtension));

			ToolbarExtenders.Add(ToolbarExtender);
			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}
	}
}

void FModManagerEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// 修复：先撤销含模块指针的菜单命令，再释放其对象。
	if (auto* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		if (auto Manager = LevelEditor->GetMenuExtensibilityManager())
		{
			for (const auto& Extender : MenuExtenders) Manager->RemoveExtender(Extender);
		}
		if (auto Manager = LevelEditor->GetToolBarExtensibilityManager())
		{
			for (const auto& Extender : ToolbarExtenders) Manager->RemoveExtender(Extender);
		}
	}
	MenuExtenders.Reset();
	ToolbarExtenders.Reset();
	PluginCommands->UnmapAction(FModManagerEditorCommands::Get().CreateModAction);
	PluginCommands.Reset();
	ModPackager.Reset();
	ModCreator.Reset();
	FModManagerEditorStyle::Shutdown();

	FModManagerEditorCommands::Unregister();
}

void FModManagerEditorModule::CreateModButtonClicked()
{
	if (ModCreator.IsValid())
	{
		ModCreator->OpenNewPluginWizard();
	}
}

void FModManagerEditorModule::AddModCreatorToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FModManagerEditorCommands::Get().CreateModAction);
}

void FModManagerEditorModule::AddModCreatorMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FModManagerEditorCommands::Get().CreateModAction);
}

void FModManagerEditorModule::AddModPackagerToolbarExtension(FToolBarBuilder& Builder)
{
	FModManagerPackager* Packager = ModPackager.Get();

	Builder.AddComboButton(FUIAction(),
						   FOnGetContent::CreateSP(Packager, &FModManagerPackager::GeneratePackagerComboButtonContent),
						   LOCTEXT("PackageMod_Label", "Package Mod"),
						   LOCTEXT("PackageMod_Tooltip", "Pack your MOD for sharing"),
						   FSlateIcon(FModManagerEditorStyle::GetStyleSetName(), "ModManagerEditor.PackageModAction")
	);
}

void FModManagerEditorModule::AddModPackagerMenuExtension(FMenuBuilder& Builder)
{
	FModManagerPackager* Packager = ModPackager.Get();

	Builder.AddSubMenu(LOCTEXT("PackageModMenu_Label", "Package Mod"),
					   LOCTEXT("PackageModMenu_Tooltip", "Pack your MOD for sharing"),
					   FNewMenuDelegate::CreateSP(Packager, &FModManagerPackager::GeneratePackagerMenuContent),
					   false,
					   FSlateIcon(FModManagerEditorStyle::GetStyleSetName(), "ModManagerEditor.PackageModAction")
	);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FModManagerEditorModule, ModManagerEditor)