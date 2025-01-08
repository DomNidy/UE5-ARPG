// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGEnhancedInputComponent.h"
#include "ARPG/Core/ARPGCharacter.h"


void UARPGEnhancedInputComponent::BindActionByTag(const UARPGInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UObject* Object, FName FuncName)
{
	check(InputConfig);

	if (const UInputAction* IA = InputConfig->FindInputActionForInputTag(InputTag))
	{
		BindAction(IA, TriggerEvent, Object, FuncName);
	}
}


