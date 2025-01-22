// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.h"
#include "Misc/Guid.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bAllowPutItemsIn = false; // can we move items into the inventory?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bAllowTakeItemsOut = false; // can we move items out of the inventory?
};

/**
 * @brief Represents the ability of an ISC to access a specific inventory.
 *
 * When an inventory is given to an ISC, an FInventoryGrant is created that
 * defines what permissions the ISC will have over the inventory and a pointer
 * to the inventory itself.
 *
 * This data is specific to an ISC.
 */
USTRUCT(BlueprintType)
struct FInventoryGrant
{
	GENERATED_BODY()
	FInventoryGrant()
	{
		Inventory = nullptr;
		GrantGuid = FGuid::NewGuid();
		InventoryPermissionSet = FInventoryPermissionSet();
	}

	FInventoryGrant(UInventory* Inventory, FInventoryPermissionSet InventoryPermissionSet)
		: Inventory(Inventory)
		, InventoryPermissionSet(InventoryPermissionSet)
	{
		GrantGuid = FGuid::NewGuid();
	}

	/**
	 * @brief Unique identifier for an inventory grant
	 */
	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FGuid GrantGuid;

	UPROPERTY()
	TObjectPtr<UInventory> Inventory;

	UPROPERTY()
	FInventoryPermissionSet InventoryPermissionSet;
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
	 * @brief Creates an inventory grant and assigns it to this ISC. If the owner actor is not authoritative, this is ignored.
	 * An owner actor is considered authoritative if it's net role (locally) is ENetRole::ROLE_Authority
	 *
	 * @param Inventory The inventory we want to give this ASC access to.
	 * @param PermissionSet Permission level that this ISC should have over the Inventory.
	 */
	virtual void GiveInventory(UInventory* Inventory, const FInventoryPermissionSet& PermissionSet);


	/**
	 * @brief Creates a new UInventory with the specified Inventory class, and then grants the inventory.
	 *
	 * @param InventoryClass Class of the Inventory we want to create
	 * @param PermissionSet The permissions
	 * @param Name
	 * @return
	 */
	virtual UInventory* CreateAndGiveInventory(TSubclassOf<UInventory> InventoryClass, const FInventoryPermissionSet& PermissionSet);

	/**
	 * @brief Return a pointer to the inventory grant with the matching grant guid
	 *
	 * Useful to allow the client to check what permissions its grant allows it over an inventory.
	 *
	 * If no inventory with a matching guid is found in the grants, nullptr is returned.
	 */
	virtual FInventoryGrant* GetInventoryGrant(FGuid Guid);


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
	 * @brief Used to lock Inventories array while iterating over it.
	 */
	mutable FCriticalSection InventoryListLock;

	/**
	 * @brief Maps inventory names to their respective inventory grants.
	 */
	UPROPERTY(Replicated)
	TArray<FInventoryGrant> Inventories;
};
