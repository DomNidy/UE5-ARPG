// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory.h"

UInventory::UInventory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

AActor* UInventory::GetOwningActor() const
{
	return nullptr;
}

UInventorySystemComponent* UInventory::GetOwningInventorySystemComponent() const
{
	return nullptr;
}

UInventorySystemComponent* UInventory::GetOwningInventorySystemComponentChecked() const
{
	return nullptr;
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

