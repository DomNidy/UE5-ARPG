// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemData.h"
#include "ItemInstance.generated.h"

class UInventory;
struct FInventorySlot;

/**
 * Represents a specific instance of an item in the game world, owned by an inventory.
 * This is the runtime representation of an item.
 *
 * This should point to a UItemData, which is a data object that stores the underlying data and functionality for items.
 * All we're concerned about here is managing the items in the context of the inventory system, not gameplay code.
 *
 * When an item is modified at runtime (e.g., it gets upgraded), then and only then should we create a new UItem for it (if an identical
 * UItem does not already exist). These UItems should be added to some kind of registry.
 *
 * An ItemInstance needs to support:
 *
 *		- Being "owned" by a specific inventory
 *
 *		- "Stacks" of items (quantity, e.g., 38 gold, 2 health potions)
 *
 *		- Splitting items off from a stack, creating a new ItemInstance with them
 *
 *      - Changes to the MaxQuantity at runtime and handle cases where the max quantity shrinks below the current item quantity.
 */
UCLASS(Blueprintable, BlueprintType)
class ARPG_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UItemInstance(const FObjectInitializer& ObjectInitializer);

	virtual int32 GetQuantity() const;
	virtual const UItemData* GetItemData() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Item creation
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Create an item instance with the specified ItemData and add it to the Inventory
	 *
	 * This method call will be ignored if not ran on server.
	 *
	 * Process Flow:
	 *	1. Creates a new item instance from the specified ItemData
	 *  2. Temporarily stores the item in the staging inventory (a game-wide holding area)
	 *  3. Attempts to transfer the item from staging to the target inventory
	 *  4. If transfer fails, removes the item from staging inventory
	 * 
	 * The staging inventory is a special, game-wide inventory. It is used to guarantee all items always belong to 
	 * an inventory, as the system relies on this assumption. Transfer to the target inventory may fail due to various 
	 * reasons, such as receive item hooks rejecting the transfer.
	 *
	 * @param ItemData The item to create
	 * @param Inventory The inventory the item should be added to
	 * @return Pointer to the new item
	 */
	static UItemInstance* CreateItemInstance(UItemData* ItemData, UInventory* Inventory);

protected:
	// ----------------------------------------------------------------------------------------------------------------
	//	Item ownership: Items must be owned by an Inventory, which in turn must be owned by an InventorySystemComponent
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Returns the owning UInventory.
	 */
	UInventory* GetOwningInventory() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Quantity
	// ----------------------------------------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadWrite, Category = "Item|Quantity")
	int32 MaxQuantity;

	UPROPERTY(BlueprintReadWrite, Category = "Item|Quantity")
	int32 Quantity;

	UFUNCTION()
	virtual void OnMaxQuantityChanged();


	UFUNCTION()
	virtual void OnQuantityChanged();

private:
	// Underlying item data. Defines what the item is and can do in gameplay code.
	const UItemData* ItemData;
};
