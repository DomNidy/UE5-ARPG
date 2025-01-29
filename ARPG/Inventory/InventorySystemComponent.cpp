// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "InventoryLogMacros.h"
#include "Logging/StructuredLog.h"


// Sets default values for this component's properties
UInventorySystemComponent::UInventorySystemComponent()
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = true;
}

bool UInventorySystemComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UInventory* Inventory : Inventories)
	{
		// Without this inventories won't replicate to client
		WroteSomething |= Channel->ReplicateSubobject(Inventory, *Bunch, *RepFlags);
	}

	return WroteSomething;
}

void UInventorySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UInventorySystemComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UInventorySystemComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register all existing UInventories
	if (IsUsingRegisteredSubObjectList())
	{

		for (const auto& Inventory : Inventories)
		{
			if (IsValid(Inventory))
			{
				check(GetOwner());
				AddReplicatedSubObject(Inventory);
			}
		}
	}
}


void UInventorySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventorySystemComponent, Inventories);
	DOREPLIFETIME(UInventorySystemComponent, InventoryGrants);
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

	InventoryGrants.Add(FInventoryGrant(PermissionSet));
	Inventories.Add(Inventory);
}



UInventory* UInventorySystemComponent::CreateAndGiveInventory(TSubclassOf<UInventory> InventoryClass, const FInventoryPermissionSet& PermissionSet)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		INVENTORY_LOG(Error, TEXT("CreateAndGiveInventory must be called on server! Called with role: %d"), GetOwnerRole());
		return nullptr;
	}

	if (!InventoryClass)
	{
		INVENTORY_LOG_ERROR(TEXT("CreateAndGiveInventory called with a null inventory class!"));
		return nullptr;
	}

	// Acquire inventory list lock
	FScopeLock Lock(&InventoryListLock);

	// Outer object should be player state
	UInventory* Inventory = NewObject<UInventory>(GetOwner(), InventoryClass);

	if (Inventory)
	{
		Inventory->OwningInventorySystemComponent = this;
		Inventories.Add(Inventory);
		InventoryGrants.Add(FInventoryGrant(PermissionSet));

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(Inventory);
		}

		return Inventory;
	}
	else
	{
		return nullptr;
	}

	return nullptr;
}

FInventoryGrant* UInventorySystemComponent::GetInventoryGrant(FGuid Guid)
{
	check(Guid.IsValid());

	FInventoryGrant* Inventory = InventoryGrants.FindByPredicate([&](FInventoryGrant Grant)
		{
			return Grant.GrantGuid == Guid;
		});

	return Inventory;
}


void UInventorySystemComponent::OnRep_InventoryGrants()
{
	check(!GetOwner()->HasAuthority());

	INVENTORY_LOG(Log, TEXT("[CLIENT] OnRep_InventoryGrants ran"));
}

void UInventorySystemComponent::OnRep_Inventories()
{
	check(!GetOwner()->HasAuthority());

	INVENTORY_LOG(Log, TEXT("[CLIENT] OnRep_Inventories ran"));

	for (const auto& Inventory : Inventories)
	{
		if (Inventory)
		{
			INVENTORY_LOG(Log, TEXT("\tGot inv: %s"), *Inventory->GetName());
		}
		else
		{
			INVENTORY_LOG_ERROR(TEXT("\tOnRep_Inventories had a null inventory"));
		}
	}
}

FString UInventorySystemComponent::GetDebugString() const
{
	const FString Separator = TEXT("========================================");
	const FString SubSeparator = TEXT("----------------------------------------");

	FString DebugString = FString::Printf(
		TEXT("\n%s\n")
		TEXT("INVENTORY SYSTEM STATUS\n")
		TEXT("Total Inventories: %d\n")
		TEXT("%s\n"),
		*Separator,
		Inventories.Num(),
		*SubSeparator
	);

	// Details for each inventory grant
	for (int32 Index = 0; Index < InventoryGrants.Num(); Index++)
	{
		const auto& InventoryGrant = InventoryGrants[Index];
		DebugString += FString::Printf(
			TEXT("\n[Inventory Grant %d]\n")
			TEXT("│ Can Take Items Out    : %s\n")
			TEXT("│ Can Put Items In      : %s\n")
			TEXT("│ Grant GUID            : %s\n")
			TEXT("%s\n"),
			Index + 1,
			InventoryGrant.InventoryPermissionSet.bAllowTakeItemsOut ? TEXT("✓ Yes") : TEXT("✗ No"),
			InventoryGrant.InventoryPermissionSet.bAllowPutItemsIn ? TEXT("✓ Yes") : TEXT("✗ No"),
			*InventoryGrant.GrantGuid.ToString(),
			*SubSeparator
		);
	}

	// Details for each inventory
	for (int32 InvIndex = 0; InvIndex < Inventories.Num(); InvIndex++)
	{
		const UInventory* Inventory = Inventories[InvIndex];

		DebugString += FString::Printf(
			TEXT("\n[Inventory %d]\n")
			TEXT("│ Is Valid              : %s\n"),
			InvIndex + 1,
			IsValid(Inventory) ? TEXT("✓ Yes") : TEXT("✗ No")
		);

		if (!IsValid(Inventory))
		{
			continue;
		}

		const TArray<FInventorySlot>& Slots = Inventory->SlotList.GetAllSlots();

		DebugString += FString::Printf(TEXT("│ Num Slots             : %d\n"), Slots.Num());

		// Add slot information if inventory is valid
		DebugString += TEXT("│\n│ Slot Contents:\n");

		// Iterate through all slots
		for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
		{
			const auto& Slot = Slots[SlotIndex];

			DebugString += FString::Printf(TEXT("│\n│ [Slot %d]\n"), SlotIndex);

			// Get the slot's debug string and indent it
			TArray<FString> SlotDebugLines;
			Slot.GetDebugString().ParseIntoArrayLines(SlotDebugLines);
			for (const FString& Line : SlotDebugLines)
			{
				DebugString += FString::Printf(TEXT("│     %s\n"), *Line);
			}
		}

		DebugString += FString::Printf(TEXT("%s\n"), *SubSeparator);
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