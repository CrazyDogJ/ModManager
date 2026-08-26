// Fill out your copyright notice in the Description page of Project Settings.


#include "SModPackagerDialog.h"

#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
#define LOCTEXT_NAMESPACE "BlutilityMenuExtensions"

void SModPackagerDialog::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._SettingsProxy;
	ParentWindow = InArgs._ParentWindow;
 
	// 获取细节面板模块并创建细节视图
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	
	TSharedRef<IDetailsView> DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);;
	DetailsView->SetObject(Proxy.Get(), true);
 
	ChildSlot.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(10)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				DetailsView
			]
		]
		+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).HAlign(HAlign_Right).Padding(10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SButton).Text(LOCTEXT("OKButton", "OK")).OnClicked(this, &SModPackagerDialog::OnConfirm)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SButton).Text(LOCTEXT("Cancel", "Cancel")).OnClicked(this, &SModPackagerDialog::OnCancel)
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
