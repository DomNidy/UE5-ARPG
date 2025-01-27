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
	if (OwningInventorySystemComponent && OwningInventorySystemComponent->GetOwner())
	{
		return OwningInventorySystemComponent->GetOwner();
	}

	return nullptr;
}

void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventory, SlotList);
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

	// Early exit if no space available
	if (!SlotList.HasEmptySlotForItemType(Item->ItemTypeTag))
	{
		INVENTORY_LOG_WARNING(TEXT("Inventory %s tried to receive an item, but no empty slot that could accept the item was found. Item name: %s"), *GetName(), *Item->GetName());
		return;
	}

	PreItemReceived(Item);

	// Get pointer to the slot we want to add the item to
	FInventorySlot* TargetSlot = nullptr;
	for (FInventorySlot& Slot : SlotList.Entries)
	{
		// if the slot is empty and doesn't block this item type, use this one
		if (Slot.DoesPermitItemType(Item->ItemTypeTag) && Slot.IsSlotEmpty())
		{
			TargetSlot = &Slot;
			break;
		}
	}

	checkf(TargetSlot && TargetSlot->IsSlotEmpty(),
		TEXT("Failed to find valid slot despite prior availability check"));

	// Transfer item ownership
	SlotList.PutItemIntoSlot(*TargetSlot, Item);

	// Update the outer of the item to the owning actor. We do this because
	// items created by the UItemInstance::CreateItemInstance method are 
	// initially created with the transient package as their outer UObject
	Item->Rename(nullptr, GetOwningActor());

	PostItemReceived(Item);

	INVENTORY_LOG(Log, TEXT("Inventory %s successfully received new item instance %s"), *GetOwningInventorySystemComponent()->GetOwner()->GetName(), *Item->GetItemDisplayName().ToString());

	return;
}

void UInventory::PreItemReceived(const UItemInstance* Item) const
{
}

void UInventory::PostItemReceived(const UItemInstance* Item) const
{
	OnInventoryChanged.Broadcast();
}



bool FInventorySlot::DoesPermitItemType(FGameplayTag ItemTypeTag) const
{
	if (BlockItemTypes.HasTagExact(ItemTypeTag))
	{
		return false;
	}

	return true;
}

TArray<UItemInstance*> FInventorySlotList::GetAllItems() const
{
	TArray<UItemInstance*> ItemPtrs;

	// This should create a new array containing all UItemInstances that are present in 
	// the slots of this SlotList.
	for (const FInventorySlot& Slot : Entries)
	{
		if (Slot.Item != nullptr)
		{
			ItemPtrs.Add(Slot.Item);
		}
	}
	return ItemPtrs;
}

const TArray<FInventorySlot>& FInventorySlotList::GetAllSlots() const
{
	return Entries;
}

void FInventorySlotList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInventorySlotList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FInventorySlotList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

bool FInventorySlotList::HasEmptySlotForItemType(FGameplayTag ItemTypeTag) const
{
	for (const FInventorySlot& Slot : Entries)
	{
		// if the slot is empty and doesn't block this item type
		if (Slot.DoesPermitItemType(ItemTypeTag) && Slot.Item == nullptr)
		{
			return true;
		}

	}
	return false;
}

void FInventorySlotList::PutItemIntoSlot(FInventorySlot& TargetSlot, UItemInstance* InItemInstance)
{
	// Make sure this slot actually exists in the Entries array before proceeding
	int32 SlotIndex = Entries.Find(TargetSlot);
	checkf(SlotIndex != INDEX_NONE, TEXT("FInventorySlotList::PutItemIntoSlot called with a target slot that did not exist in Entries"));

	// Validate slot is not empty & does not block item type
	checkf(TargetSlot.IsSlotEmpty(), TEXT("FInventorySlotList::PutItemIntoSlot called but the target slot was not empty"));
	checkf(TargetSlot.DoesPermitItemType(InItemInstance->GetItemTypeTag()), TEXT("FInventorySlotList::PutItemIntoSlot called but the target slot does not permit items of type %s"), *InItemInstance->GetItemTypeTag().ToString());

	TargetSlot.Item = InItemInstance; // Update slot to point to the item instance
	InItemInstance->OwningInventory = OwningInventory; // Update item instance to be owned by the inventory

	// Mark the slot as dirty since we updated its contents
	MarkItemDirty(TargetSlot);
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