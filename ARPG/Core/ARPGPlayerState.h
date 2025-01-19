// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "ARPG/Abilities/ARPGAbilitySystemComponent.h"
#include "ARPG/Abilities/ARPGHealthAttributeSet.h"
#include "ARPG/Abilities/ARPGAbilitySet.h"
#include "ARPG/Core/ARPGViewModelPlayerStats.h"
#include "ARPG/Inventory/InventorySystemComponent.h"
#include "ARPGPlayerState.generated.h"


const FString PlayerStatsViewModelContextName = TEXT("PlayerStatsViewModel");

/**
 *
 */
UCLASS()
class ARPG_API AARPGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AARPGPlayerState();

	virtual void BeginPlay() override;

	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	/** Ability sets to grant to the player on game start */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TObjectPtr<UARPGAbilitySet>> AbilitySets;

	/** Fetch player stats viewmodel for this player */
	UFUNCTION(BlueprintCallable)
	UARPGViewModelPlayerStats* GetPlayerStatsViewModel() const;

	/** Function that handles changes to core attributes and updates UI */
	virtual void HandleCoreAttributeValueChanged(const FOnAttributeChangeData& Data);

protected:
	/** This player's inventory system component */
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UInventorySystemComponent* InventorySystemComponent;


	/** The ASC for a player lives on player state AND the player character */
	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	TObjectPtr<UARPGAbilitySystemComponent> AbilitySystemComponent;

	/** Core attribute sets used by all entities that can do combat */
	UPROPERTY()
	TObjectPtr<const UARPGHealthAttributeSet> HealthAttributeSet;

	/** Player stats viewmodel */
	UPROPERTY()
	UARPGViewModelPlayerStats* PlayerStatsViewModel;

	/** Create view models that the player will need for UI and add them to the global view model collection */
	void InitPlayerViewModels();

	/** Grants ability sets to the player and performs other necessary initialization */
	void InitAbilitySystem();

};
