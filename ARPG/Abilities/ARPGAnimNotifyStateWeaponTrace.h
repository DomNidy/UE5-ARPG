// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ARPG/Core/ARPGCharacter.h"
#include "ARPGAnimNotifyStateWeaponTrace.generated.h"

DECLARE_STATS_GROUP(TEXT("ARPGAnimNotifications"), STATGROUP_ARPGAnimNotifications, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Weapon Trace Notify Tick"), STAT_ARPGAnimNotifyStateWeaponTrace_NotifyTick, STATGROUP_ARPGAnimNotifications);
DECLARE_CYCLE_STAT(TEXT("Weapon Trace Notify Begin"), STAT_ARPGAnimNotifyStateWeaponTrace_NotifyBegin, STATGROUP_ARPGAnimNotifications);
DECLARE_CYCLE_STAT(TEXT("Weapon Trace Notify End"), STAT_ARPGAnimNotifyStateWeaponTrace_NotifyEnd, STATGROUP_ARPGAnimNotifications);
/**
 * During an animation montage, this notify state will perform a box trace from the weapon mesh's 
 * start socket to the end socket.
 * 
 * The trace will occur every single tick.
 */
UCLASS()
class ARPG_API UARPGAnimNotifyStateWeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UARPGAnimNotifyStateWeaponTrace(const FObjectInitializer& ObjectInitializer);

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/**
	 * @brief The socket on the weapon mesh where the trace starts from
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	FName WeaponTraceStartSocket;

	/**
	 * @brief The socket on the weapon mesh where the trace ends
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	FName WeaponTraceEndSocket;

	/**
	 * @brief The half extent of the box shape that will be traced.
	 *	This shape will be traced from the start socket to the end socket of the weapon.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	FVector WeaponTraceBoxHalfExtent;

	/**
	 * @brief The object types that we will trace for (look for collisions with)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	TArray<TEnumAsByte<ECollisionChannel>> CollisionObjectTypesToQuery;

	/**
	 * @brief Should the trace ignore collisions with the character executing the animation montage
	 *	Yes by default
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	bool bIgnoreSelf = true;


	/**
	 * @brief If true, then a single swing can trigger multiple hits on the same actor in a single swing
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	bool bHitSameActorMultipleTimes = false;
	

protected:

	/**
	 * @brief Blueprint implementable event that is called when the weapon trace hits an actor
	 * @param InstigatorActor The actor that initiated the trace (the character doing the melee attack)
	 * @param HitActor The actor that was hit by the trace
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon Trace")
	void OnWeaponTraceHitActor(AActor* InstigatorActor, AActor* HitActor) const;

private:
	/**
	 * @brief The character executing the animation montage
	 */
	TObjectPtr<AARPGCharacter> Character;

	/**
	 * @brief The weapon mesh of the character
	 */
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	/**
	 * @brief Struct that contains list of object types the trace will look for when performing a collision query
	 *	Created using the ObjectTypesToQuery array.
	 */
	FCollisionObjectQueryParams CollisionObjectQueryParams;

	/**
	 * @brief Further parameters for the collision query, such as ignored actors
	 */
	FCollisionQueryParams CollisionQueryParams;

	/**
	 * @brief The shape of the trace that will be performed
	 */
	FCollisionShape WeaponTraceShape;


	// Set used to store actors that have already been hit by the trace
	TSet<AActor*> AlreadyHitActors;
};
