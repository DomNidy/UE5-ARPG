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


FString UItemInstance::GetDebugString() const
{
	FString DebugString;

	// Header with class name
	DebugString += TEXT("UItemInstance:\n");

	// Basic instance information
	DebugString += FString::Printf(TEXT("- Quantity: %d/%d\n"), GetQuantity(), GetMaxQuantity());

	// Owner information
	UInventory* OwningInv = GetOwningInventory();
	DebugString += FString::Printf(TEXT("- Owning Inventory: %s\n"),
		OwningInv ? *OwningInv->GetName() : TEXT("None"));

	// Add underlying ItemData information
	if (const UItemData* Data = GetItemData())
	{
		DebugString += TEXT("\nItem Data:\n");
		// Indent the ItemData debug string for better readability
		TArray<FString> ItemDataLines;
		Data->GetDebugString().ParseIntoArrayLines(ItemDataLines);
		for (const FString& Line : ItemDataLines)
		{
			DebugString += FString::Printf(TEXT("    %s\n"), *Line);
		}

		// If this is equipment data, add that information too
		if (const UEquipmentData* EquipData = Cast<UEquipmentData>(Data))
		{
			DebugString += TEXT("\nEquipment Data:\n");
			TArray<FString> EquipDataLines;
			EquipData->GetDebugString().ParseIntoArrayLines(EquipDataLines);
			for (const FString& Line : EquipDataLines)
			{
				DebugString += FString::Printf(TEXT("    %s\n"), *Line);
			}
		}
	}
	else
	{
		DebugString += TEXT("\nNo Item Data Available");
	}

	return DebugString;
}