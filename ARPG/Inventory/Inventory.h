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
};

/**
 * An object where ItemInstances are stored. Any and all ItemInstances are owned by an inventory,
 * and any and all Inventories are owned by a single InventorySystemComponent. Other InventorySystemComponents
 * can be granted varying levels of access to an inventory they do not own by adding them as Participants.
 *
 * Ownership & Participants:
 *
 *     Ownership of an Inventory is held by a single InventorySystemComponent. When an InventorySystemComponent
 *	   owns an Inventory, the inventory will always be replicated to them, and they will always have authority to
 *	   perform actions like sending and receiving items to and from the Inventory. The server will still need to
 *	   validate an owner's actions to prevent them from cheating.
 *
 *	   Participants of an Inventory are other InventorySystemComponents that should have some level of access to the
 *	   Inventory. Inventory data will be replicated to participants, and some participants may have different levels of
 *	   permissions. E.g., a shared guild chest might allow the guild leader to take items out and put them in, but other
 *	   guild members can only put items in.
 *
 *	   Important: While an owner always has the authority to perform actions on their inventories, these actions
 *	   can still fail. For example, if an ISC tries to send an item to another ISC's inventory, that ISC or
 *	   Inventory may still block the item from being sent. Additionally the current GameState or GameMode may cause
 *	   actions to be blocked, for example, if a player is deemed to be in combat they shouldn't be able to change their gear
 *     (which means moving an item to their equipment inventory), and if the current GameMode is "SharedPlayerCity" they
 *	   shouldn't be able to drop items into the world (which means moving an item into the world inventory).
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
UCLASS(Blueprintable)
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
	//	Slots
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Causes runtime error if an invalid index is passed
	 * @return Reference to the FInventorySlot at the specified index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot& GetSlot(int Index);

protected:
	/**
	 * @brief Slots of the inventory, which may or may not contain an ItemInstance.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> Slots;

	/**
	 * @brief Executed before an item is added (received) to this inventory.
	 * @param Item The item that we're receiving
	 * @param SourceInventory The inventory that we're receiving this item from.
	 *
	 * When this code runs, the SourceInventory still has ownership over the Item.
	 */
	virtual void PreItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const;

	/**
	 * @brief Executed after an item is added (received) to this inventory.
	 * @param Item The item that we received.
	 * @param SourceInventory The inventory this item came from
	 *
	 * When this code runs, we now have ownership over the Item.
	 */
	virtual void PostItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const;
};
