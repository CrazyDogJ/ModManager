// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModManagerLibrary.h"
#include "ModPackagerSettingsProxy.h"
#include "Widgets/SCompoundWidget.h"

class MODMANAGEREDITOR_API SModPackagerDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SModPackagerDialog) {}
	SLATE_ARGUMENT(TWeakObjectPtr<UModPackagerSettingsProxy>, SettingsProxy)
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
 
	bool bUserConfirmed = false;
 
private:
	FReply OnConfirm() { bUserConfirmed = true; CloseWindow(); return FReply::Handled(); }
	FReply OnCancel() { bUserConfirmed = false; CloseWindow(); return FReply::Handled(); }
	
	void CloseWindow() { if (ParentWindow.IsValid()) ParentWindow.Pin()->RequestDestroyWindow(); }
 
	TWeakObjectPtr<UModPackagerSettingsProxy> Proxy;
	TWeakPtr<SWindow> ParentWindow;
};
