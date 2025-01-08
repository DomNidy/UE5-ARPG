// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ARPGInputConfig.h"
#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTags.h"
#include "ARPGEnhancedInputComponent.generated.h"

/**
 *
 */
UCLASS()
class ARPG_API UARPGEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:

	/**
	 * @brief Binds an input action to a specific gameplay tag, triggering a function when the input action is performed.
	 *
	 * @param InputConfig The input configuration containing the action mappings.
	 * @param InputTag The gameplay tag associated with the input action.
	 * @param TriggerEvent The event that triggers the action (e.g., pressed, released).
	 * @param Object The object instance that will execute the function.
	 * @param Func The function to be executed when the input action is performed.
	 *
	 * @example
	 * UARPGInputConfig* MyInputConfig = ...; // Obtain the input configuration
	 * APlayerCharacter* Player = ...; // Obtain the player character
	 *
	 * InputComponent->BindActionByTag<MyPlayerCharacter>(
	 *     MyInputConfig,
	 *     FGameplayTag::RequestGameplayTag("Action.Jump"),
	 *     ETriggerEvent::Triggered,
	 *     Player,
	 *     &APlayerCharacter::Jump
	 * );
	 */
	void BindActionByTag(const UARPGInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UObject* Object, FName Func);

	/**
	 * @brief Binds multiple ability actions based on the provided input configuration, handling both pressed and released events.
	 *
	 * @param InputConfig The input configuration containing the ability input action and input tag mappings.
	 * @param Object The object instance that will execute the PressedFunc and ReleasedFunc whenever any ability action is triggered.
	 * @param PressedFunc Function to execute when an ability's input action is pressed.
	 * @param ReleasedFunc Function to execute when an ability's input action is released.
	 * @param BindHandles Array we will store action binding handles in.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UARPGInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc,
		TArray<uint32>& BindHandles)
	{
		check(InputConfig);
		for (const FTaggedAbilityAction& Action : InputConfig->TaggedAbilityActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (PressedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
				}

				if (ReleasedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				}
			}
		}
	}
};
