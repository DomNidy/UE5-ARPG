// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "ARPG/Abilities/ARPGAbilitySet.h"
#include "ItemData.generated.h"

/**
 * Default static data about items
 */
UCLASS(Blueprintable)
class ARPG_API UARPGItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Unique ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FGuid ItemId;

	// Used by the filtering system and other systems to classify and query for items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FGameplayTagContainer ItemTypeTags;

	// Localized name for display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemDisplayName;

	// Localized description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemDescription;

	// Maximum amount of this item that can be present in a stack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (ClampMin = 1))
	int32 MaxStackSize = 99;

	// Icon displayed in UI inventory windows
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* Icon;
};



/**
 * Default static data about items.
 *
 * This data is used to set the default state for proper equipment UObjects.
 *
 * This data is effectively the "base state" of a piece of equipment, and is used to store default state of equipment.
 * The reason why we take this approach over having the CDO act as the default state is because we want to define many
 * different default states for different types of equipment. For example, maybe we want all swords to grant a "Sword Melee" ability,
 * and we want all hammers to grant a "Hammer Smash" ability. To do this with CDOs, we would need to create sword and hammer subclasses,
 * which can lead to a complex inheritance hierarchy.
 */
UCLASS(Blueprintable)
class ARPG_API UARPGEquipmentData : public UARPGItemData
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
};
