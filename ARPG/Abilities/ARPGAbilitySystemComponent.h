// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ARPGAbilitySystemComponent.generated.h"

/**
 *
 */
UCLASS()
class ARPG_API UARPGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UARPGAbilitySystemComponent();

	/**
	 * @brief Call this when a player character triggers an ability input.
	 *		  This method finds the AbilitySpec associated with the input tag
	 *		  and stores its spec handle for later processing.
	 *
	 * @param AbilityInputTag the input tag of the ability that was pressed
	 */
	virtual void AbilityInputTagPressed(FGameplayTag AbilityInputTag);

	/**
	 * @brief Call this when a player character releases an ability input.
	 *		  This method finds the AbilitySpec associated with the input tag
	 *		  and stores its spec handle for later processing.
	 *
	 * @param AbilityInputTag the input tag of the ability that was released
	 */
	virtual void AbilityInputTagReleased(FGameplayTag AbilityInputTag);

	/**
	 * @brief Call this to process all accumulated ability-related inputs.
	 *		  This handles activating abilities, and forwards the ability-related input events to the server.
	 * 
	 * @param DeltaTime is the delta since the last tick, not the last time inputs were processed
	 * @param bGamePaused is the game paused?
	 */
	virtual void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

private:
	/** Array of ability specs that had their input pressed this frame and are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** Array of ability specs that have their input held down are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** Array of ability specs that have had their input released and are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/**
	 * @brief Clear all ability input awaiting processing
	 */
	virtual void ClearAbilityInput();
};
