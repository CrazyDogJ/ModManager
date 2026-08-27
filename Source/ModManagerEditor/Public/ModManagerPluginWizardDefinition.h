// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "IPluginWizardDefinition.h"
#include "Interfaces/IPluginManager.h"

class MODMANAGEREDITOR_API FModManagerPluginWizardDefinition : public IPluginWizardDefinition
{
public:
	FModManagerPluginWizardDefinition();
	void FindTemplates();

	// Begin IPluginWizardDefinition interface
	virtual const TArray<TSharedRef<FPluginTemplateDescription>>& GetTemplatesSource() const override;
#if ENGINE_MAJOR_VERSION == 5
	virtual void OnTemplateSelectionChanged(TSharedPtr<FPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo) override;
	virtual TSharedPtr<FPluginTemplateDescription> GetSelectedTemplate() const override;
#else
	virtual void OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo) override;
	virtual TArray<TSharedPtr<FPluginTemplateDescription>> GetSelectedTemplates() const override;
#endif
	virtual void ClearTemplateSelection() override;
	virtual bool HasValidTemplateSelection() const override;

	virtual bool CanShowOnStartup() const override { return true; }
	virtual bool HasModules() const override;
	virtual bool IsMod() const override;
	virtual void OnShowOnStartupCheckboxChanged(ECheckBoxState CheckBoxState) override;
	virtual ECheckBoxState GetShowOnStartupCheckBoxState() const override;
	virtual TSharedPtr<class SWidget> GetCustomHeaderWidget() override;
	virtual FText GetInstructions() const override;

	virtual bool GetPluginIconPath(FString& OutIconPath) const override;
	virtual EHostType::Type GetPluginModuleDescriptor() const override;
	virtual ELoadingPhase::Type GetPluginLoadingPhase() const override;
	virtual bool GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const override;
	virtual FString GetPluginFolderPath() const override;
	virtual TArray<FString> GetFoldersForSelection() const override;
	virtual void PluginCreated(const FString& PluginName, bool bWasSuccessful) const override;
	// End IPluginWizardDefinition interface
	
#if ENGINE_MAJOR_VERSION == 4
	// TODO : 4.27 Side codes
	virtual ESelectionMode::Type GetSelectionMode() const override;
	virtual bool AllowsEnginePlugins() const override;
	virtual bool CanContainContent() const override;
#endif
	
private:
	static void CreateGameFeatureDataAsset(const TSharedPtr<IPlugin>& Plugin);
	
	/** The available templates for the mod. They should function as mixins to the backing template */
	TArray<TSharedRef<FPluginTemplateDescription>> TemplateDefinitions;

	/** The content that will be used when creating the mod */
	TArray<TSharedPtr<FPluginTemplateDescription>> SelectedTemplates;

	/** The base directory of this plugin. Used for accessing the templates used to create mods */
	FString PluginBaseDir;

	/**
	 * The path to the template that ultimately serves as the template that the mod will be based on. It's not intended to be
	 * selected directly, but rather other templates will act as mixins to define what content will exist in the plugin.
	 */
	FString BackingTemplatePath;

	/** The backing template definition for the mod. This should never be directly selectable */
	TSharedPtr<FPluginTemplateDescription> BackingTemplate;

	/** The base code template definition. Can be directly selectable to create an "empty" code mod, but should be included with any code mod selection */
	TSharedPtr<FPluginTemplateDescription> BaseCodeTemplate;

	/** Maps a specific template to a specific icon file */
	TMap<FString, FString> TemplateToIconMap;

	/** Brush used for drawing the custom header widget */
	TSharedPtr<struct FSlateDynamicImageBrush> IconBrush;

	/** Custom header widget */
	TSharedPtr<class SWidget> CustomHeaderWidget;
};
