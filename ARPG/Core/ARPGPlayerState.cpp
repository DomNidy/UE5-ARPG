// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGPlayerState.h"
#include <MVVMGameSubsystem.h>

AARPGPlayerState::AARPGPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UARPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	SetNetUpdateFrequency(2.f);

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
	UARPGViewModelPlayerStats* PlayerStatsViewModel = GetPlayerStatsViewModel();
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
	UARPGViewModelPlayerStats* PlayerStatsViewModel = NewObject<UARPGViewModelPlayerStats>(this, TEXT("PlayerStatsViewModel"));
	if (!PlayerStatsViewModel)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create PlayerStatsViewModel for PlayerState."));
		return;
	}

	// Retrieve the MVVM subsystem
	UMVVMGameSubsystem* ViewModelSubsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	if (!ensure(ViewModelSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("MVVMGameSubsystem is missing in the GameInstance."));
		return;
	}

	UMVVMViewModelCollectionObject* GlobalViewModelCollection = ViewModelSubsystem->GetViewModelCollection();
	if (!ensure(GlobalViewModelCollection))
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalViewModelCollection is missing in MVVMGameSubsystem."));
		return;
	}

	// Set up the view model context
	FMVVMViewModelContext ViewModelContext;
	ViewModelContext.ContextClass = UARPGViewModelPlayerStats::StaticClass();
	ViewModelContext.ContextName = FName(PlayerStatsViewModelContextName);

	// Add the view model to the collection
	if (!GlobalViewModelCollection->AddViewModelInstance(ViewModelContext, PlayerStatsViewModel))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add PlayerStatsViewModel to the GlobalViewModelCollection."));
	}
}

UARPGViewModelPlayerStats* AARPGPlayerState::GetPlayerStatsViewModel() const
{

	UMVVMGameSubsystem* ViewModelSubsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	check(ViewModelSubsystem);

	UMVVMViewModelCollectionObject* GlobalViewModelCollection = ViewModelSubsystem->GetViewModelCollection();
	check(GlobalViewModelCollection);

	// Get the local player's viewmodel
	FMVVMViewModelContext ViewModelContext;
	ViewModelContext.ContextName = FName(PlayerStatsViewModelContextName);
	ViewModelContext.ContextClass = UARPGViewModelPlayerStats::StaticClass();

	UARPGViewModelPlayerStats* LocalPlayerStatsViewModel = Cast<UARPGViewModelPlayerStats>(GlobalViewModelCollection->FindViewModelInstance(ViewModelContext));
	if (!LocalPlayerStatsViewModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find local player stats viewmodel"));
		return nullptr;
	}

	return LocalPlayerStatsViewModel;
}
