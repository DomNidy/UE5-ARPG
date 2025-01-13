// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGAnimNotifyStateWeaponTrace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Logging/StructuredLog.h"
#include "DrawDebugHelpers.h"

UARPGAnimNotifyStateWeaponTrace::UARPGAnimNotifyStateWeaponTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	WeaponTraceStartSocket(TEXT("BottomTrace")),
	WeaponTraceEndSocket(TEXT("TopTrace")),
	WeaponTraceBoxHalfExtent(FVector(40.f, 40.f, 90.f)),
	bIgnoreSelf(true)
{
	WeaponTraceShape = FCollisionShape::MakeBox(WeaponTraceBoxHalfExtent);
}

void UARPGAnimNotifyStateWeaponTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	SCOPE_CYCLE_COUNTER(STAT_ARPGAnimNotifyStateWeaponTrace_NotifyBegin);
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Get the character owning the mesh component
	Character = Cast<AARPGCharacter>(MeshComp->GetOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Character is null in UARPGAnimNotifyStateWeaponTrace::NotifyBegin"));
		return;
	}

	// Only proceed if the character has authority or is the autonomous proxy (local player)
	if (Character->GetLocalRole() != ROLE_Authority && Character->GetLocalRole() != ROLE_AutonomousProxy)
	{
		return;
	}

	// Get the weapon mesh from the character
	WeaponMesh = Character->GetWeaponMesh();
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get weapon mesh in UARPGAnimNotifyStateWeaponTrace::NotifyBegin"));
		return;
	}

	// Initialize collision query parameters
	CollisionQueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace), /* bTraceComplex = */ false);

	// Ignore the character performing the trace to prevent self-hits
	if (bIgnoreSelf)
	{
		CollisionQueryParams.AddIgnoredActor(Character);
	}

	// Initialize collision object query parameters
	CollisionObjectQueryParams = FCollisionObjectQueryParams();

	// Add specified object types to query; default to Pawn if none are specified
	if (CollisionObjectTypesToQuery.Num() > 0)
	{
		for (const auto& ObjectType : CollisionObjectTypesToQuery)
		{
			CollisionObjectQueryParams.AddObjectTypesToQuery(ObjectType);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No object types specified for collision query. Defaulting to ECC_Pawn."));
		CollisionObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	}

}

// Repeatedly check for overlaps with enemy characters with a box trace around the character's sword
void UARPGAnimNotifyStateWeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	SCOPE_CYCLE_COUNTER(STAT_ARPGAnimNotifyStateWeaponTrace_NotifyTick);
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!Character || !WeaponMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Character or WeaponMesh is null in UARPGAnimNotifyStateWeaponTrace::NotifyTick"));
		return;
	}

	// Only proceed if the character has authority or is the autonomous proxy (local player)
	if (Character->GetLocalRole() != ROLE_Authority && Character->GetLocalRole() != ROLE_AutonomousProxy)
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World context is null in UARPGAnimNotifyStateWeaponTrace::NotifyTick"));
		return;
	}

	// Define the start and end locations of the trace
	FVector StartLocation = WeaponMesh->GetSocketLocation(WeaponTraceStartSocket);
	FVector EndLocation = WeaponMesh->GetSocketLocation(WeaponTraceEndSocket);
	FQuat TraceRotation = WeaponMesh->GetComponentQuat();

	// Perform the sweep trace
	TArray<FHitResult> HitResults;
	bool bHit = World->SweepMultiByObjectType(
		HitResults,
		StartLocation,
		EndLocation,
		TraceRotation,
		CollisionObjectQueryParams,
		WeaponTraceShape,
		CollisionQueryParams
	);

	// Draw the trace for debugging purposes
	DrawDebugBox(
		World,
		StartLocation,
		WeaponTraceBoxHalfExtent,
		TraceRotation,
		FColor::Red,
		false,
		0.3f,
		0,
		1.0f
	);

	// Process each hit result
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// Check if the hit actor is another character
		if (AARPGCharacter* HitCharacter = Cast<AARPGCharacter>(HitActor))
		{
			UE_LOG(LogTemp, Log, TEXT("Hit character: %s"), *HitCharacter->GetName());

			// Draw a debug box around the hit character
			DrawDebugBox(
				World,
				HitCharacter->GetActorLocation(),
				FVector(
					HitCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
					HitCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
					HitCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
				),
				FColor::Green,
				false,
				0.5f,
				0,
				1.0f
			);

			// If the hit character has an ability system component, trigger the OnWeaponTraceHitActor event
			if (Cast<UARPGAbilitySystemComponent>(HitCharacter->GetAbilitySystemComponent()))
			{
				OnWeaponTraceHitActor(Character, HitCharacter);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Hit non-character actor: %s"), *HitActor->GetName());
		}
	}
}

void UARPGAnimNotifyStateWeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	SCOPE_CYCLE_COUNTER(STAT_ARPGAnimNotifyStateWeaponTrace_NotifyEnd);
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Clear references to ensure they are not used after the notify ends
	Character = nullptr;
	WeaponMesh = nullptr;

	UE_LOG(LogTemp, Log, TEXT("UARPGAnimNotifyStateWeaponTrace::NotifyEnd"));
}
