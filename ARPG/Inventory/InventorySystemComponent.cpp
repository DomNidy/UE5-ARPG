// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Logging/StructuredLog.h"

void UInventorySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// TODO: Handle replication of inventories
	/*FDoRepLifetimeParams Params;
	Params.Condition = ELifetimeCondition::COND_OwnerOnly;*/

	DOREPLIFETIME_CONDITION(UInventorySystemComponent, Inventories, COND_OwnerOnly);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Sets default values for this component's properties
UInventorySystemComponent::UInventorySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UInventorySystemComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInventorySystemComponent::GiveInventory(UInventory* Inventory, const FInventoryPermissionSet& PermissionSet)
{
	check(Inventory != nullptr);

	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		INVENTORY_LOG(Error, TEXT("GiveInventory called on client! Cannot grant UInventory %s"), *Inventory->GetName());
		return;
	}

	INVENTORY_LOG(Log, TEXT("GiveInventory called on server! Granting UInventory %s"), *Inventory->GetName());

	FScopeLock Lock(&InventoryListLock);
	Inventories.Add(FInventoryGrant(Inventory, PermissionSet));
}

UInventory* UInventorySystemComponent::CreateAndGiveInventory(TSubclassOf<UInventory> InventoryClass, const FInventoryPermissionSet& PermissionSet)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		INVENTORY_LOG(Error, TEXT("CreateAndGiveInventory must be called on server! Called with role: %d"), GetOwnerRole());
		return nullptr;
	}

	if (InventoryClass)
	{
		// Acquire inventory list lock
		FScopeLock Lock(&InventoryListLock);

		// Outer is this ISC
		UInventory* Inventory = NewObject<UInventory>(this, InventoryClass);

		Inventories.Add(FInventoryGrant(Inventory, PermissionSet));

		return Inventory;
	}

	return nullptr;
}

FInventoryGrant* UInventorySystemComponent::GetInventoryGrant(FGuid Guid)
{
	check(Guid.IsValid());

	FInventoryGrant* Inventory = Inventories.FindByPredicate([&](FInventoryGrant Grant)
		{
			return Grant.GrantGuid == Guid;
		});

	return Inventory;
}

FString UInventorySystemComponent::GetDebugString() const
{
	FString DebugString = FString::Printf(TEXT("Inventories: %d"), Inventories.Num());

	for (const auto& InventoryGrant : Inventories)
	{
		DebugString += FString::Printf(TEXT("\n- grant guid: %s"),
			*InventoryGrant.GrantGuid.ToString());
	}

	return DebugString;
}


void UInventorySystemComponent::DebugDumpInventories() const
{
	INVENTORY_LOG(Log, TEXT("Dumping inventories..."));

	FString DebugString = GetDebugString();
	if (!DebugString.IsEmpty())
	{
		INVENTORY_LOG(Log, TEXT("%s"), *GetDebugString());
	}
	else
	{
		INVENTORY_LOG(Log, TEXT("No inventories to dump..."));
	}
}