// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGPlayerState.h"
#include <MVVMGameSubsystem.h>
#include "ARPGCharacter.h"

AARPGPlayerState::AARPGPlayerState()
{
	SetNetUpdateFrequency(100.f);

	// ISC
	InventorySystemComponent = CreateDefaultSubobject<UInventorySystemComponent>(TEXT("InventorySystemComponent"));
	InventorySystemComponent->SetIsReplicated(true);

	// ASC 
	AbilitySystemComponent = CreateDefaultSubobject<UARPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UARPGHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

void AARPGPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AARPGPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	InitPlayerViewModels();
}

void AARPGPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitAbilitySystem();
}

UAbilitySystemComponent* AARPGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UInventorySystemComponent* AARPGPlayerState::GetInventorySystemComponent() const
{
	return InventorySystemComponent;
}

void AARPGPlayerState::InitAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("No ASC"));
		return;
	}

	// Setup handlers for when core gameplay attributes change (do this on client only)
	if (GetNetMode() != NM_DedicatedServer)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UARPGHealthAttributeSet::GetHealthAttribute()).AddUObject(this, &AARPGPlayerState::HandleCoreAttributeValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UARPGHealthAttributeSet::GetHealthMaxAttribute()).AddUObject(this, &AARPGPlayerState::HandleCoreAttributeValueChanged);
	}

	// Grant all ability sets to the player
	if (HasAuthority())
	{
		FARPGAbilitySet_GrantedHandles Handles;
		for (const auto& AbilitySet : AbilitySets)
		{
			UARPGAbilitySet* SetToGrant = AbilitySet.Get();
			if (!IsValid(AbilitySet))
			{
				UE_LOG(LogTemp, Warning, TEXT("AbilitySet is null"));
				continue;
			}

			check(SetToGrant);
			SetToGrant->GiveToAbilitySystem(AbilitySystemComponent, Handles, this);
		}
	}
}


// This function is only bound on the client side. See InitAbilitySystem
void AARPGPlayerState::HandleCoreAttributeValueChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerStatsViewModel)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot handle attribute change event with no player stats view model"));
		return;
	}

	// Update the view model with new attribute values
	if (Data.Attribute == UARPGHealthAttributeSet::GetHealthAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("Core attribute changed: Health! %f"), Data.NewValue);
		PlayerStatsViewModel->SetHealth(Data.NewValue);
	}
	else if (Data.Attribute == UARPGHealthAttributeSet::GetHealthMaxAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("Core attribute changed: HealthMax! %f"), Data.NewValue);
		PlayerStatsViewModel->SetHealthMax(Data.NewValue);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Core attribute changed, but the attribute that changed was not handled."));
	}
}

void AARPGPlayerState::InitPlayerViewModels()
{

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Create the view model for the player
	UARPGViewModelPlayerStats* NewPlayerStatsViewModel = NewObject<UARPGViewModelPlayerStats>(this, TEXT("PlayerStatsViewModel"));
	if (!NewPlayerStatsViewModel)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create PlayerStatsViewModel for PlayerState."));
		return;
	}

	PlayerStatsViewModel = NewPlayerStatsViewModel;
}

UARPGViewModelPlayerStats* AARPGPlayerState::GetPlayerStatsViewModel() const
{
	return PlayerStatsViewModel;
}
