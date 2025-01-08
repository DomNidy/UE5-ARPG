// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARPGCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "ARPGNativeGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Logging/StructuredLog.h"
#include "ARPGPlayerState.h"
#include "MVVMGameSubsystem.h"
#include "ARPG/Input/ARPGEnhancedInputComponent.h"

AARPGCharacter::AARPGCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// When the character pawn is possessed, the ASC will be overwritten with the ASC stored in player state
	AbilitySystemComponent = CreateDefaultSubobject<UARPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	SetNetUpdateFrequency(100.f);

}

void AARPGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

UAbilitySystemComponent* AARPGCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AARPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(InputConfig);

	UARPGEnhancedInputComponent* EnhancedInputComponent = CastChecked<UARPGEnhancedInputComponent>(PlayerInputComponent);

	TArray<uint32> BindHandles;
	EnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, BindHandles);
}

// Server only: called when the hero is possessed by a controller (could be player or ai)
void AARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AARPGPlayerState* PS = GetPlayerState<AARPGPlayerState>();
	if (PS)
	{
		// Update property to use the ASC of the player state 
		AbilitySystemComponent = Cast<UARPGAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	}
}

// Client only: Called on clients when the player state property of this pawn is updated
void AARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AARPGPlayerState* PS = GetPlayerState<AARPGPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = Cast<UARPGAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	}
}

void AARPGCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Log, TEXT("------ Input_AbilityInputTagPressed: %s"), *InputTag.GetTagName().ToString());
	if (AbilitySystemComponent)
	{

		if (AbilitySystemComponent->HasMatchingGameplayTag(Status_Block_AbilityInput))
		{
			return;
		}

		AbilitySystemComponent->AbilityInputTagPressed(InputTag);
	}
}

void AARPGCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Log, TEXT("------ Input_AbilityInputTagReleased: %s"), *InputTag.GetTagName().ToString());

	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(Status_Block_AbilityInput))
		{
			return;
		}

		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}

