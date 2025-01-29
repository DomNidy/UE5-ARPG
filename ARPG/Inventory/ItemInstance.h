// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemData.h"
#include "ItemInstance.generated.h"

class UInventory;
struct FInventorySlot;
struct FInventorySlotList;

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
	UItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ----------------------------------------------------------------------------------------------------------------
	//	Networking
	// ----------------------------------------------------------------------------------------------------------------
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ----------------------------------------------------------------------------------------------------------------
	//	Item creation
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Create an item instance and initialize it with the provided UItemData data asset
	 *
	 * Notes:
	 *	- Call is ignored if not on dedicated server or standalone
	 *  - The resulting item has no owning Inventory.
	 *
	 * @param WorldContext The outer of the newly created ItemInstance object
	 * @param BaseItemData The UItemData this item should be initialized with
	 * @return Pointer to the new item
	 */
	static UItemInstance* CreateItemInstance(UObject* Outer, TObjectPtr<UItemData> BaseItemData);

protected:
	/**
	 * @brief Initializes an item with the provided UItemData
	 * @param BaseItemData The UItemData this item should be initialized with
	 *
	 * This should be overriden by subclasses to intialize properties specific to that subclass
	 *
	 * Notes:
	 *	- BaseItemData's properties are copied by value into the ItemInstance.
	 */
	virtual void InitializeItemInstance(TObjectPtr<UItemData> BaseItemData);

public:
	// ----------------------------------------------------------------------------------------------------------------
	//	Item ownership
	// ----------------------------------------------------------------------------------------------------------------
	/**
	 * @brief Returns the owning UInventory.
	 */
	UInventory* GetOwningInventory() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Getters for item properties
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Item|Item Properties")
	virtual const FGuid& GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintCallable, Category = "Item|Item Properties")
	virtual const FGameplayTag& GetItemTypeTag() const { return ItemTypeTag; }

	UFUNCTION(BlueprintCallable, Category = "Item|Item Properties")
	virtual const FText& GetItemDisplayName() const { return ItemDisplayName; }

	UFUNCTION(BlueprintCallable, Category = "Item|Item Properties")
	virtual const FText& GetItemDescription() const { return ItemDescription; }

	UFUNCTION(BlueprintCallable, Category = "Item|Item Properties")
	virtual const UTexture2D* GetItemIcon() const { return ItemIcon; }

	// ----------------------------------------------------------------------------------------------------------------
	//	Getters for item instance state
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Item|Item Instance State")
	virtual int32 GetQuantity() const;

	UFUNCTION(BlueprintCallable, Category = "Item|Item Instance State")
	virtual int32 GetMaxQuantity() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Misc/debug getters
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual FString GetDebugString() const;

private:
	friend UInventory;

	friend FInventorySlotList;

	UPROPERTY(Replicated)
	UInventory* OwningInventory;

protected:
	// Unique ID
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Item Properties|Classification")
	FGuid ItemId;

	// Used by the filtering system and other systems to classify and query for items
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Item Properties|Classification")
	FGameplayTag ItemTypeTag;

	// Localized name for display
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Item Properties|Basic")
	FText ItemDisplayName;

	// Localized description
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Item Properties|Basic")
	FText ItemDescription;

	// Icon displayed in UI inventory windows
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Item Properties|Visual")
	UTexture2D* ItemIcon;

protected:
	// ----------------------------------------------------------------------------------------------------------------
	//	Quantity
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION()
	virtual void SetQuantity(int NewQuantity);

	UFUNCTION()
	virtual void SetMaxQuantity(int NewMaxQuantity);

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_MaxQuantity, EditDefaultsOnly, BlueprintReadWrite, Category = "Item|Quantity")
	int32 MaxQuantity;

	UFUNCTION()
	virtual void OnMaxQuantityChanged();

	UFUNCTION()
	virtual void OnRep_MaxQuantity();

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Quantity, EditDefaultsOnly, BlueprintReadWrite, Category = "Item|Quantity")
	int32 Quantity;

	UFUNCTION()
	virtual void OnQuantityChanged();

	UFUNCTION()
	virtual void OnRep_Quantity();
};
