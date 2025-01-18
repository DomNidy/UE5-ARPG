// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "EquipSlot.generated.h"

/**
 * @brief An actor component that handles the logic of equipping gear.
 *
 * Responsibilities:
 *	 - attaching meshes for gear (e.g., weapons, armor) to the correct sockets,
 *	 - calling the gear's `Equip` and `Unequip` methods as needed
 *   - defining gear to slot compatibility types (e.g., a 1 handed slot can equip a sword and dagger).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARPG_API UEquipSlot : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipSlot();

	/**
	 * @brief Try to equip a piece of equipment to this slot
	 * @param Equipment The equipment to equip
	 * @param OutEquipError Reference to equip error struct. Errors are written here.
	 * @return True if was equipped successfully, false if not.
	 */
	//UFUNCTION(BlueprintCallable)
	//virtual bool TryEquip(UItemInstance* Equipment);

	/**
	 * @brief Returns true if the provided container of equipment tags is compatible with this slot's supported equipment tags
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool VerifyEquipmentTagsCompatible(const FGameplayTagContainer& EquipmentTags) const;
private:
	/**
	 * @brief This slot will only accept equipment that matches at least one of the
	 *	equipment tags in this container. Currently using this over enum to allow for certain slots to be more
	 *  flexible in the types of gear they accept (or don't accept).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (GameplayTagFilter = "Equipment"))
	FGameplayTagContainer SupportedEquipmentTags;

	/**
	 * @brief The name of the socket on the player character that this slot will attach gear to (by default)
	 *	TODO: Allow Equippable items to override the socket locations.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName GearSocketName;
};
