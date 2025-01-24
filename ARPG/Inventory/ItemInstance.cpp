// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Inventory.h"
#include "InventoryLogMacros.h"

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

int32 UItemInstance::GetMaxQuantity() const
{
	return MaxQuantity;
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

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemInstance, Quantity);
	DOREPLIFETIME(UItemInstance, MaxQuantity);
}

void UItemInstance::SetQuantity(int NewQuantity)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		INVENTORY_LOG_ERROR(TEXT("SetQuantity called on client"));
		return;
	}


	Quantity = NewQuantity;
	INVENTORY_LOG(Log, TEXT("SERVER CHANGED QUANTITY: %d"), Quantity);
}

void UItemInstance::SetMaxQuantity(int NewMaxQuantity)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		INVENTORY_LOG_ERROR(TEXT("SetMaxQuantity called on client"));
		return;
	}


	MaxQuantity = NewMaxQuantity;
	INVENTORY_LOG(Log, TEXT("SERVER CHANGED MAX QUANTITY: %d"), MaxQuantity);
}

UInventory* UItemInstance::GetOwningInventory() const
{
	return nullptr;
}

void UItemInstance::OnMaxQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}

void UItemInstance::OnRep_MaxQuantity()
{
	INVENTORY_LOG(Log, TEXT("Max quantity changed to: %d"), MaxQuantity);
}

void UItemInstance::OnQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}

void UItemInstance::OnRep_Quantity()
{
	INVENTORY_LOG(Log, TEXT("Quantity changed to: %d"), Quantity);
}
