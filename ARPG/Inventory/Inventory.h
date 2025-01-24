// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemInstance.h"
#include "Inventory.generated.h"

class UInventorySystemComponent;

/**
 * A struct that can hold an ItemInstance
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemInstance> Item;

	UPROPERTY()
	int32 TestProperty = 1;
};

// Event dispatched when an inventory changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * An object where ItemInstances are stored. Any and all ItemInstances are owned by an inventory,
 * and any and all Inventories are owned by a single InventorySystemComponent. Multiple
 * InventorySystemComponents can be given access to an Inventory, and the level of access an ISC
 * has over a particular inventory can be controlled with their FInventoryGrant.
 *
 *
 * Ownership & Grants:
 *
 *     Ownership of an Inventory is held by a single InventorySystemComponent. When an InventorySystemComponent
 *	   owns an Inventory, the inventory will always be replicated to them. The server will still need to
 *	   validate an owner's actions to prevent them from cheating.
 *
 *	   Other InventorySystemComponents can be given granular levels of access to an Inventory they are not the owner of.
 *	   This is accomplished through the FInventoryGrant struct and permissions. Only the owner is capable of
 *	   issuing grants to other ISCs.
 *
 * Capacity:
 *
 *     Inventories contain one or many inventory slots, which are what ItemInstances are held in.
 *     The number of slots defines the capacity of the inventory, and slots can be dynamically added or
 *     removed at runtime.
 *
 *
 * Examples of Inventories:
 *
 *     Every "container" in the game that items can be placed in, or taken from, is an inventory.
 *     The equipment tab in the game is an inventory, trade windows are Inventories, and even loot
 *     on the ground in the game world is owned by an inventory.
 *
 * Motivation:
 *
 *     One of the most vital and important interactions in the game is looting items. The process of looting
 *     an item is actually a "transaction" between two Inventories. When an item is looted, it is moved from one
 *	   Inventory (e.g., the world Inventory) to the character Inventory. Similarly, if the player drops an item
 *	   on the ground, that item is moved from the character Inventory to the world Inventory.
 *
 *
 * Validates (allows or blocks) actions performed on Inventories, such as:
 *
 *	 - Moving items to different Inventories

 *   - Moving items around to different slots.
 *
 *	 - Receiving items from other Inventories.
 *
 * All operations are validated on the authority (server).
 */
UCLASS(Blueprintable, BlueprintType)
class ARPG_API UInventory : public UObject
{
	GENERATED_BODY()

public:
	UInventory(const FObjectInitializer& ObjectInitializer);

	// Get the Actor that owns this inventory (which is the actor that owns this inventory's ISC)
	AActor* GetOwningActor() const;
	UInventorySystemComponent* GetOwningInventorySystemComponent() const;
	UInventorySystemComponent* GetOwningInventorySystemComponentChecked() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Networking
	// ----------------------------------------------------------------------------------------------------------------
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// ----------------------------------------------------------------------------------------------------------------
	//	Inventory validation
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Checks that the inventory is valid, which means it does not violate any invariants (like not having an owning inventory
	 * system component.
	 * @return True if the inventory is valid, false if it's not
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsValidInventory() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Slots
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Causes runtime error if an invalid index is passed
	 * @return Reference to the FInventorySlot at the specified index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot& GetSlot(int Index);

	// ----------------------------------------------------------------------------------------------------------------
	//	Delegates / Events
	// ----------------------------------------------------------------------------------------------------------------
	/** Parameterless delegate that fires whenever the inventory changes (e.g., item added/removed, capacity changes, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

protected:
	/**
	 * @brief Slots of the inventory, which may or may not contain an ItemInstance.
	 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> Slots;


protected:
#pragma region ReceivingItems
	friend class UInventorySystemComponent;


	/** Method invoked by the InventorySystemComponent this inventory belongs to. */
	bool TryReceiveItem(const UItemInstance* Item, const UInventory* SourceInventory);

	// ----------------------------------------------------------------------------------------------------------------
	//	Receiving item hooks
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Executed before an item is added (received) to this inventory.
	 * @param Item The item that we're receiving
	 * @param SourceInventory The inventory that we're receiving this item from.
	 *
	 * When this code runs, the SourceInventory still has ownership over the Item.
	 */
	virtual void PreItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const;

	/**
	 * @brief Executed after an item is successfully added (received) to this inventory.
	 * @param Item The item that we received.
	 * @param SourceInventory The inventory this item came from
	 *
	 * When this code runs, we now have ownership over the Item.
	 */
	virtual void PostItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const;
#pragma 
private:
	/**
	 * @brief Reference to the ISC that owns this inventory.
	 *
	 * This will only be defined and non null on the client that is the original
	 * owner of this inventory (not someone that was given a grant after it was created),
	 * and on the server.
	 */
	UPROPERTY(Replicated)
	TObjectPtr<UInventorySystemComponent> OwningInventorySystemComponent;
};
