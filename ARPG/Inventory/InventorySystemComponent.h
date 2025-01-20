// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.h"
#include "InventorySystemComponent.generated.h"

#define INVENTORY_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogInventorySystem, Verbosity, Format, ##__VA_ARGS__); \
}

/**
 * @brief When an inventory is given to an ISC, a
 * permission set must be specified. This indicates what actions
 * an ISC should be able to perform on a specific inventory.
 *
 * For example, we might want to give the same UInventory
 * to multiple ISCs and allow each of them to put items into
 * the inventory, but only allow one to transfer items
 * out of the inventory.
 *
 */
USTRUCT(BlueprintType)
struct FInventoryPermissionSet
{
	GENERATED_BODY()

	UPROPERTY()
	bool bAllowAddItems = 0;

	UPROPERTY()
	bool bAllowTakeItems = 0;
};


/**
* Manages ownership of Inventories for an Actor. Can be player, or non player.
*
* The InventorySystemComponent can be "granted" multiple inventories. Inventories can also be revoked.
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARPG_API UInventorySystemComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Sets default values for this component's properties
	UInventorySystemComponent();

	// ----------------------------------------------------------------------------------------------------------------
	//	Inventories
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Grants an inventory to an ISC. If the owner actor is not authoritative, this is ignored.
	 *	An owner actor is considered authoritative if it's net role is ENetRole::ROLE_Authority

	 * @param Name The name to assign to the granted inventory.
	 */
	virtual void GiveInventory(UInventory* Inventory, const FInventoryPermissionSet& PermissionSet, FName Name);

	/**
	 * @brief Return a pointer to the inventory with the specified name.
	 *
	 * If no inventory with a matching name is found, nullptr is returned instead.
	 */
	virtual UInventory* GetInventory(FName Name);


	// ----------------------------------------------------------------------------------------------------------------
	//	Debugging
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	virtual FString GetDebugString() const;

	/**
	 * @brief Dump info about all inventories to output
	 */
	UFUNCTION(Exec, Category = "Inventory|Debug")
	virtual void DebugDumpInventories() const;

protected:
	virtual void BeginPlay() override;
private:

	/**
	 * @brief Maps inventory names to their respective inventories.
	 */
	UPROPERTY()
	TMap<FName, UInventory*> Inventories;
};
