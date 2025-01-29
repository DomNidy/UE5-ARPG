// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemInstance.h"
#include "InventoryLogMacros.h"
#include "Inventory.generated.h"

class UInventorySystemComponent;

/**
 * A struct that can hold an ItemInstance
 */
USTRUCT(BlueprintType)
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventorySlot() {}

	//~ Begin FFastArraySerializerItem contract
	void PreReplicatedRemove(const FInventorySlotList& InArraySerializer);
	void PostReplicatedAdd(const FInventorySlotList& InArraySerializer);
	void PostReplicatedChange(const FInventorySlotList& InArraySerializer);
	//~ FFastArraySerializerItem contract

	FString GetDebugString() const;
public:
	/**
	 * @brief Check if this slot permits (does not block) items with the ItemTypeTag
	 * @return True if permitted, false if not.
	 */
	bool DoesPermitItemType(FGameplayTag ItemTypeTag) const;

	/**
	 * @brief Is this slot empty? (Not item instance held by it)
	 */
	bool IsSlotEmpty() const { return Item == nullptr; }

	bool operator==(const FInventorySlot& Other) const
	{
		return (BlockItemTypes == Other.BlockItemTypes && Item == Other.Item);
	}

	bool operator!=(const FInventorySlot& Other) const
	{
		return !(*this == Other);
	}
private:
	friend FInventorySlotList;
	friend UInventory;
private:

	/**
	 * @brief ItemInstance currently inside this slot
	 */
	UPROPERTY()
	TObjectPtr<UItemInstance> Item = nullptr;

	/**
	 * @brief What item "types" should not be allowed inside this slot
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Filters")
	FGameplayTagContainer BlockItemTypes = FGameplayTagContainer();
};

/**
 * A list of FInventorySlots.
 *
 * Inventories use these instead of managing individual slots.
 *
 * Handles efficiently replicating slot contents, etc.
 */
USTRUCT(BlueprintType)
struct FInventorySlotList : public FFastArraySerializer
{
	GENERATED_BODY()

	FInventorySlotList() : OwningInventory(nullptr)
	{
		//Items.Empty();
	}

	FInventorySlotList(UInventory* InOwnerInventory) : OwningInventory(InOwnerInventory)
	{
		//Items.Empty();
	}

	TArray<UItemInstance*> GetAllItems() const;
	const TArray<FInventorySlot>& GetAllSlots() const;
private:
	friend UInventory;
public:
	//~ Begin FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~ EndFFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlot, FInventorySlotList>(Items, DeltaParams, *this);
	}
public:
	// ----------------------------------------------------------------------------------------------------------------
	//	Slot management
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Does this slot list have an empty item slot that can hold an item of
	 * of ItemType. (i.e., it's not blocked by slot item filters).
	 */
	bool HasEmptySlotForItemType(FGameplayTag ItemTypeTag) const;
private:
	/**
	 * @brief Adds an empty slot to the slot list
	 */
	void AddEmptySlot(FInventorySlot Slot);
private:
	/**
	 * The array of slots.
	 *
	 * Note: FFastArraySerializer documentation states that this variable **must**
	 * be named "Items". So just leave it named like this.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Slots")
	TArray<FInventorySlot> Items;

	UPROPERTY(NotReplicated)
	UInventory* OwningInventory;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotList> : public TStructOpsTypeTraitsBase2<FInventorySlotList>
{
	enum { WithNetDeltaSerializer = true };
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
 *     Ownership of an Inventory is held by a single InventorySystemComponent.
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

	//~Begin UObject Interface
	virtual void PostInitProperties() override;
	//~Begin UObject End

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
	//	Delegates / Events
	// ----------------------------------------------------------------------------------------------------------------
	/** Parameterless delegate that fires whenever the inventory changes (e.g., item added/removed, capacity changes, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

private:
	/**
	 * @brief Slots of the inventory, which may or may not contain an ItemInstance.
	 */
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Inventory")
	FInventorySlotList SlotList;

#pragma region ReceivingItems
public:
	/**
	 * @brief Attempts to add an ItemInstance to this inventory
	 * @param Item item instance we're trying to add
	 *
	 * This method is ignored if called on client.
	 *
	 * Note: This should only be called on items that are not owned by an inventory
	 * will throw an error if called on an item that already has an owner.
	 */
	void TryReceiveItem(UItemInstance* Item);

protected:
	friend class UInventorySystemComponent;


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
	virtual void PreItemReceived(const UItemInstance* Item) const;

	/**
	 * @brief Executed after an item is successfully added (received) to this inventory.
	 * @param Item The item that we received.
	 * @param SourceInventory The inventory this item came from
	 *
	 * When this code runs, we now have ownership over the Item.
	 */
	virtual void PostItemReceived(const UItemInstance* Item) const;
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
