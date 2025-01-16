// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "ARPGEquipSlot.generated.h"

USTRUCT(BlueprintType)
struct FARPGEquipError
{
	GENERATED_BODY()

	/**
	 * @brief Text to display to user letting them know why their gear failed to equip
	 */
	UPROPERTY()
	FText ErrorText;
};

UINTERFACE(MinimalAPI, Blueprintable)
class UARPGEquipableGear : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for equipable gear. Should be implemented by any class that can be equiped.
 */
class ARPG_API IARPGEquipableGear
{
	GENERATED_BODY()

public:
	/**
	 * Function called when this piece of gear is equipped to a slot
	 * @param Slot The equip slot gear was equipped to
	 */
	virtual void OnEquippedToSlot(UARPGEquipSlot* Slot) = 0;

	/**
	 * @brief Function that returns equipment tags of the equipment
	 */
	virtual FGameplayTagContainer GetEquipmentTags() const = 0;
};

/**
 * @brief An actor component that handles the logic of equipping gear.
 *
 * Responsibilities:
 *	 - attaching meshes for gear (e.g., weapons, armor) to the correct sockets,
 *	 - calling the gear's `Equip` and `Unequip` methods as needed
 *   - defining gear to slot compatibility types (e.g., a 1 handed slot can equip a sword and dagger).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARPG_API UARPGEquipSlot : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UARPGEquipSlot();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief Try to equip a piece of equipment to this slot
	 * @param Equipment The equipment to equip
	 * @param OutEquipError Reference to equip error struct. Errors are written here.
	 * @return True if was equipped successfully, false if not.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool TryEquip(TScriptInterface<IARPGEquipableGear> Equipment, FARPGEquipError& OutEquipError);

	/**
	 * @brief Returns true if the provided container of equipment tags is compatible with this slot's supported equipment tags
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool VerifyEquipmentTagsCompatible(const FGameplayTagContainer& EquipmentTags) const;
private:
	/**
	 * @brief This slot will only accept equipment that matches at least one of the
	 *	equipment tags in this container
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (GameplayTagFilter = "Equipment"))
	FGameplayTagContainer SupportedEquipmentTags;

	/**
	 * @brief The name of the socket on the player character that this slot will attach gear to (by default)
	 *	TODO: Allow equipable items to override the socket locations.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName GearSocketName;
};
