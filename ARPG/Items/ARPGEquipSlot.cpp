// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGEquipSlot.h"

// Sets default values for this component's properties
UARPGEquipSlot::UARPGEquipSlot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UARPGEquipSlot::BeginPlay()
{
	Super::BeginPlay();
}

void UARPGEquipSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UARPGEquipSlot::TryEquip(TScriptInterface<IARPGEquipableGear> Equipment, FARPGEquipError& OutEquipError)
{
	if (!VerifyEquipmentTagsCompatible(Equipment->GetEquipmentTags()))
	{
		OutEquipError.ErrorText = FText::FromString("Equipment tags are not supported!");
		return false;
	}

	AActor* OwnerActor = GetOwner();
	check(OwnerActor);

	UE_LOG(LogTemp, Log, TEXT("Got owner actor in equip slot: %s"), *OwnerActor->GetName());


	return true;
}

bool UARPGEquipSlot::VerifyEquipmentTagsCompatible(const FGameplayTagContainer& EquipmentTags) const
{
	return SupportedEquipmentTags.HasAll(EquipmentTags);
}

