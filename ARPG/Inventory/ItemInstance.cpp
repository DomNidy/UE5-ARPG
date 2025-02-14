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
	OwningInventory = nullptr;
}

int32 UItemInstance::GetQuantity() const
{
	return Quantity;
}

int32 UItemInstance::GetMaxQuantity() const
{
	return MaxQuantity;
}

UItemInstance* UItemInstance::CreateItemInstance(UObject* Outer, TObjectPtr<UItemData> BaseItemData)
{
	checkf(BaseItemData && IsValid(BaseItemData), TEXT("UItemInstance::CreateItemInstance called with invalid BaseItemData"));
	checkf(Outer, TEXT("UItemInstance::CreateItemInstance called with null Outer!"));
	checkf(Outer->GetWorld(), TEXT("UItemInstance::CreateItemInstance called with Outer, but the Outer has no UWorld!"));

	// Only allow items to be created on dedicated server or standalone 
	if (Outer->GetWorld()->GetNetMode() != ENetMode::NM_DedicatedServer && Outer->GetWorld()->GetNetMode() != ENetMode::NM_Standalone)
	{
		INVENTORY_LOG_WARNING(TEXT("UItemInstance::CreateItemInstance called on machine with invalid net mode. Only should be called on NM_DedicatedServer or NM_Standalone."));
		return nullptr;
	}

	UItemInstance* NewItemInstance = NewObject<UItemInstance>(Outer);

	NewItemInstance->InitializeItemInstance(BaseItemData);

	return NewItemInstance;
}

void UItemInstance::InitializeItemInstance(TObjectPtr<UItemData> BaseItemData)
{
	checkf(BaseItemData && IsValid(BaseItemData), TEXT("UItemInstance::InitializeItemInstance called with invalid BaseItemData"));
	ItemDisplayName = BaseItemData->ItemDisplayName;
	ItemDescription = BaseItemData->ItemDescription;
	ItemTypeTag = BaseItemData->ItemTypeTag;
	ItemId = BaseItemData->ItemId;
	ItemIcon = BaseItemData->ItemIcon;
}

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	INVENTORY_LOG(Log, TEXT("UItemInstance::GetLifetimeReplicatedProps called. On server?: %s"),
		GetWorld()
		? GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer ? TEXT("True") : TEXT("False")
		: TEXT("No world"));

	DOREPLIFETIME(UItemInstance, Quantity);
	DOREPLIFETIME(UItemInstance, MaxQuantity);
	DOREPLIFETIME(UItemInstance, OwningInventory);

	DOREPLIFETIME(UItemInstance, ItemDisplayName);
	DOREPLIFETIME(UItemInstance, ItemDescription);
	DOREPLIFETIME(UItemInstance, ItemId);
	DOREPLIFETIME(UItemInstance, ItemIcon);
	DOREPLIFETIME(UItemInstance, ItemTypeTag);

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
	return OwningInventory;
}

void UItemInstance::OnMaxQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}

void UItemInstance::OnRep_MaxQuantity()
{
	INVENTORY_LOG_SCREEN(TEXT("Max quantity changed to: %d"), MaxQuantity);
}

void UItemInstance::OnQuantityChanged()
{
	CHECK_QUANTITY_VALID();
}

void UItemInstance::OnRep_Quantity()
{
	INVENTORY_LOG_SCREEN(TEXT("Quantity changed to: %d"), Quantity);
}


FString UItemInstance::GetDebugString() const
{
	FString DebugString;

	DebugString += TEXT("UItemInstance:\n");

	// information about this instance
	DebugString += FString::Printf(TEXT("- Quantity: %d/%d\n"), GetQuantity(), GetMaxQuantity());

	// what inventory owns this item instance?
	UInventory* OwningInv = GetOwningInventory();
	DebugString += FString::Printf(TEXT("- Owning Inventory: %s\n"),
		OwningInv ? *OwningInv->GetName() : TEXT("None"));

	// information about this instance's item properties
	DebugString += TEXT("\nItem Data:\n");

	DebugString += FString::Printf(TEXT("- Item: %s\n"), *ItemDisplayName.ToString());
	DebugString += FString::Printf(TEXT("- ID: %s\n"), *ItemId.ToString());
	DebugString += FString::Printf(TEXT("- Item Type Tag: %s\n"), *ItemTypeTag.ToString());

	// Add description if it exists
	if (!ItemDescription.IsEmpty())
	{
		DebugString += FString::Printf(TEXT("Description: %s\n"), *ItemDescription.ToString());
	}

	// Add icon information
	DebugString += FString::Printf(TEXT("Has Icon: %s"), ItemIcon != nullptr ? TEXT("Yes") : TEXT("No"));

	return DebugString;
}