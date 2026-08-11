// Fill out your copyright notice in the Description page of Project Settings.


#include "ModManagerEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FModManagerEditorStyle::StyleInstance = NULL;

void FModManagerEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FModManagerEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

void FModManagerEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FModManagerEditorStyle::Get()
{
	return *StyleInstance;
}

FName FModManagerEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ModManagerEditorStyle"));
	return StyleSetName;
}

TSharedRef<class FSlateStyleSet> FModManagerEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("ModManagerEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ModManager")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ModManagerEditor.PackageModAction", new IMAGE_BRUSH(TEXT("PackageMod_64x"), Icon40x40));
	Style->Set("ModManagerEditor.CreateModAction", new IMAGE_BRUSH(TEXT("CreateMod_64x"), Icon40x40));

	return Style;
}
