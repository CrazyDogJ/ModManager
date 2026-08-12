// Fill out your copyright notice in the Description page of Project Settings.


#include "SModPackagerDialog.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SModPackagerDialog::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._SettingsProxy;
	ParentWindow = InArgs._ParentWindow;
 
	// 获取细节面板模块并创建细节视图
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	
	TSharedRef<IDetailsView> DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);;
	DetailsView->SetObject(Proxy.Get());
 
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(10)
		[
			DetailsView
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SButton).Text(FText::FromString("Confirm")).OnClicked(this, &SModPackagerDialog::OnConfirm)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SButton).Text(FText::FromString("Cancel")).OnClicked(this, &SModPackagerDialog::OnCancel)
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
