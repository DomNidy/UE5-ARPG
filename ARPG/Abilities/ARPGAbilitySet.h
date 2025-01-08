// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Logging/StructuredLog.h"
#include "InputAction.h"

#include "GameplayEffect.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"

#include "ARPGAbility.h"
#include "ARPGAbilitySystemComponent.h"

#include "ARPGAbilitySet.generated.h"

class UARPGAbilitySystemComponent;
class UGameplayEffect;
struct FGameplayAbilitySpecHandle;

/**
 * FARPGAbilitySet_GameplayAbility
 *
 *	Represents a gameplay ability in the context of an FARPGAbilitySet.
 *	This struct is a wrapper around a gameplay ability with additional data
 *  needed to grant that gameplay ability (e.g., ability level).
 *
 *  Notably, the InputAction for an ability can be configured here. We should
 *  probably move the input action binding elsewhere in the future.
 */
USTRUCT(BlueprintType)
struct FARPGAbilitySet_GameplayAbility
{
	GENERATED_BODY()
public:
	// Class of the ability to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UARPGAbility> Ability;

	// Level to grant the ability at
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel{ 1 };

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

/**
 * FARPGAbilitySet_GameplayEffect
 *
 *	Represents a gameplay effect in the context of an FARPGAbilitySet.
 *	This effect will be applied when the FARPGAbilitySet is granted.
 */
USTRUCT(BlueprintType)
struct FARPGAbilitySet_GameplayEffect
{
	GENERATED_BODY()
public:
	// Gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	// Level of the effect to grant
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel{ 1.f };
};

/**
 * FARPGAbilitySet_AttributeSet
 *
 *	Represents an attribute set in the context of an FARPGAbilitySet.
 *	This attribute set will be granted when the FARPGAbilitySet is granted.
 */
USTRUCT(BlueprintType)
struct FARPGAbilitySet_AttributeSet
{
	GENERATED_BODY()
public:
	// Attribute set to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};


/**
 * FARPGAbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set
 *
 *	The `GiveToAbilitySystem` method of UARPGAbilitySet, accepts an out-pointer parameter
 *  to an instance of this struct. While granting the abilities, effects, and attribute sets,
 *  to the ASC, each returned handle will be added to the arrays stored here using the
 *  AddAbilitySpecHandle, AddGameplayEffectHandle, and AddAttributeSet methods respectively.
 *
 *	These handles can be used to remove anything that was granted.
 */
USTRUCT(BlueprintType)
struct FARPGAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeAbilityFromAbilitySystem(UARPGAbilitySystemComponent* ASC);

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * UARPGAbilitySet
 *
 *	Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 *
 *	Allows the creation of sets of abilities that can be quickly assigned to multiple characters.
 *	For instance, a "Fire Mage" ability set may contain a fire ball, meteor shower, etc.
 */
UCLASS(BlueprintType, Const)
class UARPGAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:

	UARPGAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GiveToAbilitySystem(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject = nullptr) const;

	/**
	 * Helper method that returns reference to the array of gameplay abilities in this set
	 *	useful to retrieve the input actions for each ability
	 */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	const TArray<FARPGAbilitySet_GameplayAbility>& GetGameplayAbilities() const;


protected:
	// Grant all abilities in this set to the provided ASC
	void GrantGameplayAbilities(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject) const;

	// Grant (apply) all gameplay effects in this set to the provided ASC
	void GrantGameplayEffects(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles) const;

	// Grant all attribute sets in this set to the provided ASC
	void GrantAttributeSets(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles) const;

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (TitleProperty = Ability))
	TArray<FARPGAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect))
	TArray<FARPGAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta = (TitleProperty = AttributeSet))
	TArray<FARPGAbilitySet_AttributeSet> GrantedAttributes;
};