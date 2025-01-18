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

	FVector StartLocation = WeaponMesh->GetSocketLocation(WeaponTraceStartSocket);
	UE_LOGFMT(LogTemp, Log, "START LOC: {0}", StartLocation.ToString());
	// Draw box at the starting point of trace
	DrawDebugBox(
		Character->GetWorld(),
		StartLocation,
		FVector(20.f, 20.f, 20.f),
		WeaponMesh->GetSocketQuaternion(WeaponTraceEndSocket),
		FColor::Cyan,
		false,
		5.0f
	);
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
	FVector MidVec = EndLocation - StartLocation;
	// get up vector of world
	FQuat TraceRotation = WeaponMesh->GetSocketQuaternion(WeaponTraceEndSocket);

	UE_LOGFMT(LogTemp, Log, "Start location: {0}, end location: {1}, trace rotation: {2}, end-start: {3}", StartLocation.ToString(), EndLocation.ToString(), TraceRotation.ToString(), MidVec.ToString());

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
	// Draw path of the trace
	DrawDebugLine(
		World,               // The world context
		StartLocation,       // Starting point of the trace
		EndLocation,         // Ending point of the trace
		FColor::Red,         // Line color (Red, for example)
		false,               // Persistent (false for temporary, true for persistent)
		2.0f,                // Duration (2 seconds for temporary debug line)
		0,                   // Depth priority (0 for default)
		1.0f                 // Line thickness
	);


	// Draw box at the starting point of trace
	DrawDebugBox(
		World,
		StartLocation,
		FVector(1.f, 1.f, 1.f),
		TraceRotation,
		FColor::Yellow,
		false,
		2.0f
	);

	DrawDebugBox(
		World,
		EndLocation,
		FVector(1.f, 1.f, 1.f),
		TraceRotation,
		FColor::Yellow,
		false,
		2.0f
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

			DebugDrawBoxAroundCharacter(HitCharacter, FColor::Green, 0.3f);

			// If the hit character has an ability system component, trigger the OnWeaponTraceHitActor event
			if (Cast<UARPGAbilitySystemComponent>(HitCharacter->GetAbilitySystemComponent()))
			{

				// If we should NOT hit the same actor multiple times and this actor hasn't been hit yet
				if (!bHitSameActorMultipleTimes && !AlreadyHitActors.Contains(HitCharacter))
				{
					AlreadyHitActors.Add(HitCharacter);
					CollisionQueryParams.AddIgnoredActor(HitCharacter);
					OnWeaponTraceHitActor(Character, HitCharacter);
				}
				else if (bHitSameActorMultipleTimes)
				{
					OnWeaponTraceHitActor(Character, HitCharacter);
				}

			}
		}
	}
}

void UARPGAnimNotifyStateWeaponTrace::DebugDrawBoxAroundCharacter(ACharacter* TargetCharacter, FColor Color, float Duration)
{
	if (!TargetCharacter)
	{
		return;
	}
	UWorld* World = TargetCharacter->GetWorld();
	if (!World)
	{
		return;
	}
	DrawDebugBox(
		World,
		TargetCharacter->GetActorLocation(),
		FVector(
			TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		),
		Color,
		false,
		Duration,
		0,
		1.0f
	);
}

void UARPGAnimNotifyStateWeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	SCOPE_CYCLE_COUNTER(STAT_ARPGAnimNotifyStateWeaponTrace_NotifyEnd);
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (WeaponMesh)
	{
		FVector StartLocation = WeaponMesh->GetSocketLocation(WeaponTraceStartSocket);
		UE_LOGFMT(LogTemp, Log, "END LOC: {0}", StartLocation.ToString());

		DrawDebugBox(
			Character->GetWorld(),
			StartLocation,
			FVector(20.f, 20.f, 20.f),
			WeaponMesh->GetSocketQuaternion(WeaponTraceEndSocket),
			FColor::Purple,
			1,
			5.0f
		);
	}

	// Clear references to ensure they are not used after the notify ends (notify states seem to retain state/be reused across multiple executions?)
	Character = nullptr;
	WeaponMesh = nullptr;
	AlreadyHitActors.Empty();
}
