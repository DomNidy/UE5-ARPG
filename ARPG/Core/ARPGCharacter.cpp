// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARPGCharacter.h"
#include "ARPGViewModelPlayerStats.h"
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
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// --- Character Movement ---
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// --- Camera Boom ---
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// --- Camera ---
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// --- ASC Initialization ---
	AbilitySystemComponent = CreateDefaultSubobject<UARPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	SetNetUpdateFrequency(100.f);

	// --- Healthbar Widget ---
	HealthbarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthbarWidgetComponent"));
	HealthbarWidgetComponent->SetupAttachment(RootComponent);
	HealthbarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthbarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthbarWidgetComponent->SetDrawAtDesiredSize(true);
	HealthbarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<UUserWidget> HealthbarWidgetClassFinder(TEXT("/Script/UMG.WidgetBlueprintGeneratedClass'/Game/UI/W_PlayerCharacterFloatingHealthbar.W_PlayerCharacterFloatingHealthbar_C'"));
	if (HealthbarWidgetClassFinder.Succeeded())
	{
		HealthbarWidgetComponent->SetWidgetClass(HealthbarWidgetClassFinder.Class);
	}
}

void AARPGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Randomly log inventories on client
	if (!HasAuthority() && FMath::Rand() % 78 == 0)
	{
		AARPGPlayerState* PS = GetPlayerState<AARPGPlayerState>();

		if (PS)
		{
			if (UInventorySystemComponent* ISC = PS->GetInventorySystemComponent())
			{
				UE_LOG(LogTemp, Log, TEXT("[Client] We found an ISC on AARPGCharacter::OnRep_PlayerState! Logging."));
				ISC->DebugDumpInventories();
			}
		}
	}
}

void AARPGCharacter::BeginPlay()
{
	Super::BeginPlay();

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

// Server only: called when the character is possessed by a controller (could be player or ai)
void AARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AARPGPlayerState* PS = GetPlayerState<AARPGPlayerState>();
	if (PS)
	{
		// Update property to use the ASC of the player state 
		AbilitySystemComponent = Cast<UARPGAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}

}

// Client only: Called on clients when the player state property of this pawn is updated
void AARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AARPGPlayerState* PS = GetPlayerState<AARPGPlayerState>();
	if (PS)
	{
		// Initialize ASC locally
		AbilitySystemComponent = Cast<UARPGAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		OnAbilitySystemComponentUpdated(AbilitySystemComponent);

		if (UInventorySystemComponent* ISC = PS->GetInventorySystemComponent())
		{
			UE_LOG(LogTemp, Log, TEXT("[CLIENT] We found an ISC on AARPGCharacter::OnRep_PlayerState! Dumping it."));
			ISC->DebugDumpInventories();
		}

		// Initialize player stats viewmodel
		if (PS->GetPlayerStatsViewModel())
		{
			PlayerStatsViewModel = PS->GetPlayerStatsViewModel();
			OnPlayerStatsViewModelUpdated(PlayerStatsViewModel);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OnRep Player stats viewmodel not found"));
		}
	}
}

void AARPGCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
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
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(Status_Block_AbilityInput))
		{
			return;
		}

		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}

