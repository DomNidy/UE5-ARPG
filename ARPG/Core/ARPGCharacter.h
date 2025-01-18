// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ARPG/Abilities/ARPGAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ARPG/Input/ARPGInputConfig.h"
#include "ARPGViewModelPlayerStats.h"
#include "Components/WidgetComponent.h"
#include "ARPGCharacter.generated.h"


UCLASS(Blueprintable)
class AARPGCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AARPGCharacter();

	//~ Begin ACharacter
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End ACharacter

	//~ Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	//~ End AActor

	//~ Being APawn
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	//~ End APawn

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

public:

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }


	/** Returns the player stats viewmodel for this character */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel")
	UARPGViewModelPlayerStats* GetPlayerStatsViewModel() const { return PlayerStatsViewModel; }


	/**
	 * @brief Returns the mesh for the currently equipped weapon if one exists
	 * TODO: Get the mesh from the equipment system
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const { return nullptr; }


protected:

	/** ASC for the player's character. Managed by the player state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UARPGAbilitySystemComponent* AbilitySystemComponent;

	/** Widget component for the player healthbar */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthbarWidgetComponent;

	/** Input config data asset. This should probably be moved elsewhere */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UARPGInputConfig* InputConfig;

	/** Callback functions executed when ability inputs are pressed/released */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerStatsViewModelUpdated(UARPGViewModelPlayerStats* NewPlayerStatsViewModel);

	/**
	 * @brief Allows blueprint to know when the ability system component has been updated/changed
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilitySystemComponentUpdated(UARPGAbilitySystemComponent* NewAbilitySystemComponent);

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditDefaultsOnly, Category = "Viewmodel")
	UARPGViewModelPlayerStats* PlayerStatsViewModel;
};

