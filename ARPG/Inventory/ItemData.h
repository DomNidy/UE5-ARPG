// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "ARPG/Abilities/ARPGAbilitySet.h"
#include "ItemData.generated.h"

/**
 * Default data about items used to initialize ItemInstances.
 */
UCLASS(Blueprintable)
class ARPG_API UItemData : public UDataAsset
{
	GENERATED_BODY()

public:

	// Unique ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FGuid ItemId;

	// Used by the filtering system and other systems to classify and query for items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FGameplayTag ItemTypeTag;

	// Localized name for display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemDisplayName;

	// Localized description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemDescription;

	// Icon displayed in UI inventory windows
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* ItemIcon;

	virtual FString GetDebugString() const;
};


/**
 * Base item data asset for equipment-type items.
 */
UCLASS(Blueprintable)
class ARPG_API UEquipmentData : public UItemData
{
	GENERATED_BODY()

public:
	/**
	 * @brief Mesh for the piece of equipment when it's in the game world (equipped on character or on ground)
	 * Not required.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/**
	 * @brief Ability set granted when this equipment is applied to the character
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	UARPGAbilitySet* AbilitySet;

	virtual FString GetDebugString() const override;
};
