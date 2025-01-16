// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "ARPGEquipSlot.h"
#include "ARPGSwordBase.generated.h"

UCLASS()
class ARPG_API AARPGSwordBase : public AActor, public IARPGEquipableGear
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AARPGSwordBase();


	//~ Begin IARPGEquipableGear interface
	virtual void OnEquippedToSlot(UARPGEquipSlot* Slot) override;
	virtual FGameplayTagContainer GetEquipmentTags() const override;
	//~ End IARPGEquipableGear interface
protected:


	virtual void BeginPlay() override;

	/**
	 * @brief Mesh component for the sword
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	/**
	 * @brief The hitbox for the weapon. When swinging the weapon, collisions will be checked against this.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	UBoxComponent* WeaponHitBox;


	/**
	 * @brief Enables collision on the box component, allowing swings to be detected.
	 *	The weapon will not register any overlap or collision events unless it's in the prepared for attack state.
	 * Call this before the attack.
	 */
	UFUNCTION(BlueprintCallable)
	void PrepareForAttack();

	/**
	 * @brief Disables collision on the box component, swings will not detect any more collisions
	 *	Call this when the attack animation is done.
	 */
	UFUNCTION(BlueprintCallable)
	void FinishForAttack();
};
