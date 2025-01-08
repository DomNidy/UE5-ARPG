// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ARPG/Abilities/ARPGAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ARPG/Input/ARPGInputConfig.h"
#include "ARPGCharacter.generated.h"

UCLASS(Blueprintable)
class AARPGCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AARPGCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	


protected:
	/** ASC for the player's character. Managed by the player state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UARPGAbilitySystemComponent* AbilitySystemComponent;

	/** Input config data asset. This should probably be moved elsewhere */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UARPGInputConfig* InputConfig;


	/** Callback functions executed when ability inputs are pressed/released */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};

