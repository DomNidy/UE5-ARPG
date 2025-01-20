// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInstance.h"


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
