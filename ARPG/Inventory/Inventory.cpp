// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory.h"
#include "Net/UnrealNetwork.h"
#include "InventoryLogMacros.h"
#include "InventorySystemComponent.h"

UInventory::UInventory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

AActor* UInventory::GetOwningActor() const
{
	return nullptr;
}

void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventory, Slots);
	DOREPLIFETIME(UInventory, OwningInventorySystemComponent);
}


UInventorySystemComponent* UInventory::GetOwningInventorySystemComponent() const
{
	return OwningInventorySystemComponent;
}

UInventorySystemComponent* UInventory::GetOwningInventorySystemComponentChecked() const
{
	check(OwningInventorySystemComponent != nullptr);
	return OwningInventorySystemComponent;
}

bool UInventory::IsValidInventory() const
{
	if (GetOwningInventorySystemComponent() == nullptr)
	{
		return false;
	}

	return true;
}

FInventorySlot& UInventory::GetSlot(int Index)
{
	check(Slots.IsValidIndex(Index));

	return Slots[Index];
}

void UInventory::TryReceiveItem(UItemInstance* Item)
{

	if (GetOwningInventorySystemComponent()->GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		INVENTORY_LOG_WARNING(TEXT("UInventory::TryReceiveItem was called on client. This should only be called on server."));
		return;
	}

	checkf(Item, TEXT("UInventory::TryReceiveItem called with a null Item"));
	checkf(IsValid(Item), TEXT("UInventory::TryReceiveItem called with an invalid Item (pending kill)"));
	checkf(Item->OwningInventory == nullptr, TEXT("UInventory::TryReceiveItem called on item that already has an owner. Use transfer methods instead."));

	// Lock all slots
	FScopeLock Lock(&InventoryItemsLock);

	// Early exit if no space available
	if (!HasEmptySlotForItemType(Item->ItemTypeTag))
	{
		INVENTORY_LOG_WARNING(TEXT("Inventory %s tried to receive an item, but no empty slot that could accept the item was found. Item name: %s"), *GetName(), *Item->GetName());
		return;
	}

	PreItemReceived(Item);

	// Get pointer to the slot we want to add the item to
	FInventorySlot* TargetSlot = nullptr;
	for (FInventorySlot& Slot : Slots)
	{
		// if the slot is empty and doesn't block this item type, use this one
		if (Slot.DoesPermitItemType(Item->ItemTypeTag) && Slot.Item == nullptr)
		{
			TargetSlot = &Slot;
			break;
		}
	}

	checkf(TargetSlot && TargetSlot->Item == nullptr,
		TEXT("Failed to find valid slot despite prior availability check"));

	// Transfer item ownership
	Item->OwningInventory = this;
	TargetSlot->Item = Item;

	// Update the outer of the item to this inventory we do this because
	// items created by the UItemInstance::CreateItemInstance method are 
	// initially created with the transient package as their outer UObject
	Item->Rename(nullptr, this);

	PostItemReceived(Item);

	checkf(TargetSlot->Item == Item && Item->OwningInventory == this, TEXT("Inventory/item state mismatch after transfer."));

	INVENTORY_LOG(Log, TEXT("Inventory %s successfully received new item instance %s"), *GetOwningInventorySystemComponent()->GetOwner()->GetName(), *Item->GetItemDisplayName().ToString());

	return;
}

bool UInventory::HasEmptySlotForItemType(FGameplayTag ItemTypeTag) const
{
	for (const FInventorySlot& Slot : Slots)
	{
		// if the slot is empty and doesn't block this item type
		if (Slot.DoesPermitItemType(ItemTypeTag) && Slot.Item == nullptr)
		{
			return true;
		}
	}

	return false;
}


void UInventory::PreItemReceived(const UItemInstance* Item) const
{
}

void UInventory::PostItemReceived(const UItemInstance* Item) const
{
	OnInventoryChanged.Broadcast();
}


FString FInventorySlot::GetDebugString() const
{
	FString DebugString;

	// Start with basic slot information
	DebugString = TEXT("FInventorySlot:\n");

	// Add item instance information if it exists
	if (IsValid(Item))
	{
		// Get the item instance debug string and indent it
		TArray<FString> ItemLines;
		Item->GetDebugString().ParseIntoArrayLines(ItemLines);
		for (const FString& Line : ItemLines)
		{
			DebugString += FString::Printf(TEXT("    %s\n"), *Line);
		}
	}
	else
	{
		DebugString += TEXT("    Empty Slot\n");
	}

	// Add blocked item types if any exist
	if (BlockItemTypes.Num() > 0)
	{
		DebugString += TEXT("Blocked Types: ");
		FString TagString;
		for (const FGameplayTag& Tag : BlockItemTypes)
		{
			if (!TagString.IsEmpty())
			{
				TagString += TEXT(", ");
			}
			TagString += Tag.ToString();
		}
		DebugString += TagString;
	}
	else
	{
		DebugString += TEXT("No Blocked Types");
	}

	return DebugString;
}

bool FInventorySlot::DoesPermitItemType(FGameplayTag ItemTypeTag) const
{
	if (BlockItemTypes.HasTagExact(ItemTypeTag))
	{
		return false;
	}

	return true;
}

