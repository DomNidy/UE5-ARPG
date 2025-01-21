// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemInstance.h"
#include "Inventory.h"


#define CHECK_QUANTITY_VALID(); check(Quantity <= MaxQuantity && Quantity >= 0);

UItemInstance::UItemInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Quantity = 0;
	MaxQuantity = 1;
}

int32 UItemInstance::GetQuantity() const
{
	return Quantity;
}

const UItemData* UItemInstance::GetItemData() const
{
	return ItemData;
}

UItemInstance* UItemInstance::CreateItemInstance(UItemData* ItemData, UInventory* Inventory)
{
	check(Inventory);
	return nullptr;
}

UInventory* UItemInstance::GetOwningInventory() const
{
	return nullptr;
}

void UItemInstance::OnMaxQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}

void UItemInstance::OnQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}
