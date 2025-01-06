// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGAttributeSet.h"
#include "ARPGAbilityHelpers.h"
#include "ARPGHealthAttributeSet.generated.h"

/**
 * Attribute set that manages the health of a character in the world.
 * 
 */
UCLASS()
class ARPG_API UARPGHealthAttributeSet : public UARPGAttributeSet
{
	GENERATED_BODY()
	
public:
	UARPGHealthAttributeSet();

	// Accessor functions for attributes
	ATTRIBUTE_ACCESSORS(UARPGHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UARPGHealthAttributeSet, HealthMax);

	// Accessor functions for meta-attributes (server-side only)
	ATTRIBUTE_ACCESSORS(UARPGHealthAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(UARPGHealthAttributeSet, Healing);

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_HealthMax(const FGameplayAttributeData& OldHealthMax);


	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Used to specify which properties this Actor should replicate (manually overriding this for fine tuned control over attribute replication) */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthMax, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HealthMax;

	/**
	 * Meta attribute: server-side only
	 *
	 * Stores damage that the owner of this attribute set should RECEIVE
	 * This is not the amount they deal.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Damage;


	/**
	 * Meta attribute: server-side only
	 *
	 * Stores healing that the owner of this attribute set should RECEIVE
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Healing", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Healing;
};
