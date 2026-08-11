// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ModManagerEditorStyle.h"

class MODMANAGEREDITOR_API FModManagerEditorCommands : public TCommands<FModManagerEditorCommands>
{
public:
	FModManagerEditorCommands()
		: TCommands<FModManagerEditorCommands>(TEXT("ModManagerEditor"), NSLOCTEXT("Contexts", "ModManagerEditor", "ModManagerEditor Plugin"), NAME_None, FModManagerEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

	TArray<TSharedPtr<FUICommandInfo>> RegisterModCommands(const TArray<TSharedRef<class IPlugin>>& ModList) const;
	void UnregisterModCommands(TArray<TSharedPtr<FUICommandInfo>>& UICommands) const;

public:
	TSharedPtr<FUICommandInfo> CreateModAction;
	TSharedPtr<FUICommandInfo> PackageModAction;
};
