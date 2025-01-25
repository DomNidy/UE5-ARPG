// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory.h"
#include "Net/UnrealNetwork.h"

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

bool UInventory::TryReceiveItem(const UItemInstance* Item, const UInventory* SourceInventory)
{
	return false;
}

void UInventory::PreItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const
{
}

void UInventory::PostItemReceived(const UItemInstance* Item, const UInventory* SourceInventory) const
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