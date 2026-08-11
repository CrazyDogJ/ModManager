// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerEditorCommands.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FModManagerEditorCommands"

void FModManagerEditorCommands::RegisterCommands()
{
	UI_COMMAND(CreateModAction, "Create Mod", "Create a new MOD package in a mod plugin", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PackageModAction, "Package Mod", "Pack your MOD for sharing", EUserInterfaceActionType::Button, FInputChord());
}

TArray<TSharedPtr<FUICommandInfo>> FModManagerEditorCommands::RegisterModCommands(const TArray<TSharedRef<class IPlugin>>& ModList) const
{
	TArray<TSharedPtr<FUICommandInfo>> AvailableModActions;
	AvailableModActions.Reserve(ModList.Num());

	FModManagerEditorCommands* MutableThis = const_cast<FModManagerEditorCommands*>(this);

	for (int32 Index = 0; Index < ModList.Num(); ++Index)
	{
		AvailableModActions.Add(TSharedPtr<FUICommandInfo>());
		TSharedRef<IPlugin> Mod = ModList[Index];

		FString CommandName = "ModEditorMod_" + Mod->GetName();

		FUICommandInfo::MakeCommandInfo(MutableThis->AsShared(),
										AvailableModActions[Index],
										FName(*CommandName),
										FText::FromString("MOD: " + Mod->GetName()),
										FText::FromString("MOD located at " + Mod->GetBaseDir()),
										FSlateIcon(),
										EUserInterfaceActionType::Button,
										FInputChord());
	}

	return AvailableModActions;
}

void FModManagerEditorCommands::UnregisterModCommands(TArray<TSharedPtr<FUICommandInfo>>& UICommands) const
{
	FModManagerEditorCommands* MutableThis = const_cast<FModManagerEditorCommands*>(this);

	for (TSharedPtr<FUICommandInfo> Command : UICommands)
	{
		FUICommandInfo::UnregisterCommandInfo(MutableThis->AsShared(), Command.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE