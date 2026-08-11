#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModManagerEditor, Log, All);

class FModManagerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // When the Create Button is clicked
    void CreateModButtonClicked();

    /** Adds the plugin creator as a new toolbar button */
    void AddModCreatorToolbarExtension(FToolBarBuilder& Builder);

    /** Adds the plugin creator as a new menu option */
    void AddModCreatorMenuExtension(FMenuBuilder& Builder);

    /** Adds the plugin packager as a new toolbar button */
    void AddModPackagerToolbarExtension(FToolBarBuilder& Builder);

    /** Adds the plugin packager as a new menu option */
    void AddModPackagerMenuExtension(FMenuBuilder& Builder);

private:
    TSharedPtr<class FModManagerCreator> ModCreator;
    TSharedPtr<class FModManagerPackager> ModPackager;
    TSharedPtr<class FUICommandList> PluginCommands;
};
