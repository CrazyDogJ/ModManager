// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModManagerLibrary.h"
#include "UObject/Object.h"
#include "ModPackagerSettingsProxy.generated.h"

UENUM(BlueprintType)
enum class ETargetPlatform : uint8
{
	Win64		UMETA(DisplayName = "Win 64"),
	Win32		UMETA(DisplayName = "Win 32"),
	Mac			UMETA(DisplayName = "Mac"),
	Linux		UMETA(DisplayName = "Linux"),
};

UCLASS()
class MODMANAGEREDITOR_API UModPackagerSettingsProxy : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Mod Configuration")
	FModInfo Settings;
	
	UPROPERTY(EditAnywhere, Category = "Mod Configuration")
	ETargetPlatform TargetPlatform;
};
