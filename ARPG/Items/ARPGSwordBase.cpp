// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGSwordBase.h"
#include "Logging/StructuredLog.h"
#include "ARPG/Core/ARPGNativeGameplayTags.h"

AARPGSwordBase::AARPGSwordBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponHitBox"));

	WeaponHitBox->SetRelativeLocation(FVector(0.f, 0.f, 62.0f));
	WeaponHitBox->SetBoxExtent(FVector(15.f, 6.f, 40.f));

	WeaponHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponHitBox->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
}


void AARPGSwordBase::OnEquippedToSlot(UARPGEquipSlot* Slot)
{
	UE_LOGFMT(LogTemp, Log, "SWORD EQUIPPED TO SLOT: {0}", Slot->GetName());
}

FGameplayTagContainer AARPGSwordBase::GetEquipmentTags() const
{
	// only equipable in slots that support 1h melee weapons
	return FGameplayTagContainer(Equipment_Melee_1H);
}

void AARPGSwordBase::BeginPlay()
{
	Super::BeginPlay();
}

void AARPGSwordBase::PrepareForAttack()
{
	WeaponHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AARPGSwordBase::FinishForAttack()
{
	WeaponHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

