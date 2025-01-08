// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags.h"
#include "InputAction.h"
#include "ARPGInputConfig.generated.h"

/**
 * Correlates an InputAction with an input tag (GameplayTag).
 *
 * FTaggedInputAction should be used for actions NOT linked to gameplay abilities
 */
USTRUCT(BlueprintType)
struct FTaggedInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/**
 * Correlates an InputAction with an input tag (GameplayTag).
 *
 * The input tag should match the input tag of an ability in a UARPGAbilitySet
 * and can be used to activate the corresponding ability.
 */
USTRUCT(BlueprintType)
struct FTaggedAbilityAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 *
 */
UCLASS()
class ARPG_API UARPGInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * Returns the first input action associated with an input tag
	 *
	 * Note: This will only search for matching input actions in the TaggedInputActions array.
	 *		InputActions associated with an FTaggedAbilityAction will never be returned.
	 */
	const UInputAction* FindInputActionForInputTag(const FGameplayTag& InputTag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTaggedInputAction> TaggedInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTaggedAbilityAction> TaggedAbilityActions;
};
