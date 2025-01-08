// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ARPGAbility.generated.h"


UENUM()
enum class EARPGAbilityActivationPolicy : uint8 {
	OnInputPressed UMETA(DisplayName = "Ability is activated when it's input is pressed/triggered"),
	OnInputReleased UMETA(DisplayName = "Ability is activated when it's input is released"),
	WhileInputActive UMETA(DisplayName = "Ability is repeatedly activate while it's input is held down")
};

/**
 *
 */
UCLASS()
class ARPG_API UARPGAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UARPGAbility();

	EARPGAbilityActivationPolicy GetActivationPolicy() const;

private:
	UPROPERTY(EditDefaultsOnly)
	EARPGAbilityActivationPolicy ActivationPolicy;
};
