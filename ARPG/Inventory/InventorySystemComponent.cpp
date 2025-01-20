// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Logging/StructuredLog.h"

void UInventorySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// TODO: Handle replication of inventories
	/*FDoRepLifetimeParams Params;
	Params.Condition = ELifetimeCondition::COND_OwnerOnly;*/

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Sets default values for this component's properties
UInventorySystemComponent::UInventorySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UInventorySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Example of how to give an inventory
	/*UInventory* Inventory = NewObject<UInventory>(this);
	GiveInventory(Inventory, FName("DefaultInventory"));
	DebugDumpInventories();*/
}


void UInventorySystemComponent::GiveInventory(UInventory* Inventory, const FInventoryPermissionSet& PermissionSet, FName Name)
{
	check(Inventory != nullptr);
	check(Name.IsValid());

	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		INVENTORY_LOG(Error, TEXT("GiveInventory called on client! Cannot grant UInventory %s with inventory name: %s"), *Inventory->GetName(), *Name.ToString());
		return;
	}

	INVENTORY_LOG(Log, TEXT("GiveInventory called on server! Granting UInventory %s with inventory name: %s"), *Inventory->GetName(), *Name.ToString());

	if (Inventories.Contains(Name))
	{
		INVENTORY_LOG(Error, TEXT("GiveInventory failed to grant an inventory. UInventory with name %s already exists.'"), *Name.ToString());
		return;
	}

	Inventories.Add({ Name, Inventory });
}

UInventory* UInventorySystemComponent::GetInventory(FName Name)
{
	check(Name.IsValid());

	return *Inventories.Find(Name);
}

FString UInventorySystemComponent::GetDebugString() const
{
	FString DebugString = FString::Printf(TEXT("Inventories: %d"), Inventories.Num());

	for (const auto& Pair : Inventories)
	{
		DebugString += FString::Printf(TEXT("\n- %s"),
			*Pair.Key.ToString());
	}

	return DebugString;
}


void UInventorySystemComponent::DebugDumpInventories() const
{
	INVENTORY_LOG(Log, TEXT("Dumping inventories..."));
	INVENTORY_LOG(Log, TEXT("%s"), *GetDebugString());
}