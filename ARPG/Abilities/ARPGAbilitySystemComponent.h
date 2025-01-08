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


	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;


	virtual void AbilityInputTagPressed(FGameplayTag InputTag);
	virtual void AbilityInputTagReleased(FGameplayTag InputTag);

	virtual void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

private:
	/** Array of ability specs that had their input pressed this frame and are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** Array of ability specs that have their input held down are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** Array of ability specs that have had their input released and are waiting to be processed */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/**
	 * @brief Array of abilities that we will call TryActivateAbility on
	 * This is used internally by ProcessAbilityInput
	 */
	

	/**
	 * @brief Clear all ability input awaiting processing
	 */
	virtual void ClearAbilityInput();
};
