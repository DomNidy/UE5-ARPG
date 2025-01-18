// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipSlot.h"

// Sets default values for this component's properties
UEquipSlot::UEquipSlot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

//bool UEquipSlot::TryEquip(TScriptInterface<IARPGEquippableGear> Equipment)
//{
//	if (!VerifyEquipmentTagsCompatible(Equipment->GetEquipmentTags()))
//	{
//		return false;
//	}
//
//	AActor* OwnerActor = GetOwner();
//	check(OwnerActor);
//
//	UE_LOG(LogTemp, Log, TEXT("Got owner actor in equip slot: %s"), *OwnerActor->GetName());
//
//
//	return true;
//}

bool UEquipSlot::VerifyEquipmentTagsCompatible(const FGameplayTagContainer& EquipmentTags) const
{
	return SupportedEquipmentTags.HasAll(EquipmentTags);
}

