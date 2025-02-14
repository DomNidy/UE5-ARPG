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


	INVENTORY_LOG(Log, TEXT("UInventory::GetLifetimeReplicatedProps called. On server?: %s"),
		GetWorld()
		? GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer ? TEXT("True") : TEXT("False")
		: TEXT("No world"));

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

void UInventory::PostInitProperties()
{
	Super::PostInitProperties();

	INVENTORY_LOG(Log, TEXT("UInventory::PostInitProperties called on inventory: %s"), *GetFullGroupName(false));

	// Update the SlotList's owning inventory for instances (not the CDO)
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SlotList.OwningInventory = this;
		SlotList.MarkArrayDirty();
		INVENTORY_LOG(Log, TEXT("\tRegistered SlotList for this inventory."));
	}
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

	INVENTORY_LOG(Log, TEXT(" UInventory::TryReceiveItem called (on server)!"));

	checkf(IsValid(Item), TEXT("UInventory::TryReceiveItem called with an invalid Item (potentially pending kill)"));
	checkf(IsValidInventory(), TEXT("UInventory::TryReceiveItem called, but the UInventory was invalid (had OwningInventorySystemComponent)"));
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
	for (FInventorySlot& Slot : SlotList.Items)
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
	TargetSlot->Item = Item; // Update slot to point to the item instance
	Item->OwningInventory = this; // Update item instance to be owned by the inventory

	/** This sets the outer of the ItemInstance to the ISC, meaning that the ItemInstance is now a subobject of the ISC. */
	Item->Rename(nullptr, OwningInventorySystemComponent);

	// Mark the slot as dirty since we updated its contents
	SlotList.MarkItemDirty(*TargetSlot);

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
	for (const FInventorySlot& Slot : Items)
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
	return Items;
}


void FInventorySlotList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	TArray<FString> IndexStrings;
	for (int32 Index : RemovedIndices)
	{
		IndexStrings.Add(FString::FromInt(Index));
	}
	FString IndicesString = FString::Printf(TEXT("[%s]"), *FString::Join(IndexStrings, TEXT(", ")));

	INVENTORY_LOG(Log, TEXT("FInventorySlotList::PreReplicatedRemove, removed indices = %s, owning inv: %s, has authority: %s"),
		*IndicesString,
		OwningInventory ? *OwningInventory->GetName() : TEXT("No owning inventory"),
		OwningInventory ?
		(OwningInventory->GetOwningActor()->HasAuthority() ? TEXT("True") : TEXT("False")) : TEXT("N/a"));
}

void FInventorySlotList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	TArray<FString> IndexStrings;
	for (int32 Index : AddedIndices)
	{
		IndexStrings.Add(FString::FromInt(Index));
	}
	FString IndicesString = FString::Printf(TEXT("[%s]"), *FString::Join(IndexStrings, TEXT(", ")));

	INVENTORY_LOG(Log, TEXT("FInventorySlotList::PostReplicatedAdd, added indices = %s, owning inv: %s, has authority: %s"),
		*IndicesString,
		OwningInventory ? *OwningInventory->GetName() : TEXT("No owning inventory"),
		OwningInventory ?
		(OwningInventory->GetOwningActor()->HasAuthority() ? TEXT("True") : TEXT("False")) : TEXT("N/a"));
}

void FInventorySlotList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	TArray<FString> IndexStrings;
	for (int32 Index : ChangedIndices)
	{
		IndexStrings.Add(FString::FromInt(Index));
	}
	FString IndicesString = FString::Printf(TEXT("[%s]"), *FString::Join(IndexStrings, TEXT(", ")));

	INVENTORY_LOG(Log, TEXT("FInventorySlotList::PostReplicatedChange, changed indices = %s, owning inv: %s, has authority: %s"),
		*IndicesString,
		OwningInventory ? *OwningInventory->GetName() : TEXT("No owning inventory"),
		OwningInventory ?
		(OwningInventory->GetOwningActor()->HasAuthority() ? TEXT("True") : TEXT("False")) : TEXT("N/a"));
}

bool FInventorySlotList::HasEmptySlotForItemType(FGameplayTag ItemTypeTag) const
{
	for (const FInventorySlot& Slot : Items)
	{
		// if the slot is empty and doesn't block this item type
		if (Slot.DoesPermitItemType(ItemTypeTag) && Slot.Item == nullptr)
		{
			return true;
		}

	}
	return false;
}



void FInventorySlotList::AddEmptySlot(FInventorySlot Slot)
{
	Items.Add(Slot);
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

void FInventorySlot::PreReplicatedRemove(const FInventorySlotList& InArraySerializer)
{
	INVENTORY_LOG(Log, TEXT("FInventorySlot::PreReplicatedRemove, item instance in slot: %s"),
		Item ? *Item->GetName() : TEXT("No item"))

}

void FInventorySlot::PostReplicatedAdd(const FInventorySlotList& InArraySerializer)
{
	INVENTORY_LOG(Log, TEXT("FInventorySlot::PostReplicatedAdd, item instance in slot: %s"),
		Item ? *Item->GetName() : TEXT("No item"))
}

void FInventorySlot::PostReplicatedChange(const FInventorySlotList& InArraySerializer)
{
	INVENTORY_LOG(Log, TEXT("FInventorySlot::PostReplicatedChange, item instance in slot: %s"),
		Item ? *Item->GetName() : TEXT("No item"))
}