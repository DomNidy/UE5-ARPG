

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ARPG/Abilities/ARPGAbilitySystemComponent.h"
#include "ARPG/Abilities/ARPGAbilitySet.h"
#include "ARPG/Abilities/ARPGHealthAttributeSet.h"
#include "ARPGEnemyCharacter.generated.h"

UCLASS()
class ARPG_API AARPGEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AARPGEnemyCharacter();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** ASC for this enemy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UARPGAbilitySystemComponent* AbilitySystemComponent;

	/** Ability sets to grant on game start */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TObjectPtr<UARPGAbilitySet>> AbilitySets;

	/** Core attribute sets used by all entities that can do combat */
	UPROPERTY()
	TObjectPtr<const UARPGHealthAttributeSet> HealthAttributeSet;

	/** Grants ability sets to the enemy and performs other necessary initialization */
	void GrantInitialAbilitySets();

	/** Function that handles changes to core attributes and updates UI */
	virtual void HandleCoreAttributeValueChanged(const FOnAttributeChangeData& Data);
};
