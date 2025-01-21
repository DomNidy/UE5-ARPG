


#include "ARPGEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AARPGEnemyCharacter::AARPGEnemyCharacter()
{
	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Configure ASC
	AbilitySystemComponent = CreateDefaultSubobject<UARPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	SetNetUpdateFrequency(100.f);

	HealthAttributeSet = CreateDefaultSubobject<UARPGHealthAttributeSet>(TEXT("HealthAttributeSet"));

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* AARPGEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AARPGEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	check(AbilitySystemComponent);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

}

void AARPGEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetNetMode() != NM_DedicatedServer && AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Adding delegates for enemy"));
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute()).AddUObject(this, &AARPGEnemyCharacter::HandleCoreAttributeValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthMaxAttribute()).AddUObject(this, &AARPGEnemyCharacter::HandleCoreAttributeValueChanged);
	}
}

// Called when the game starts or when spawned
void AARPGEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GrantInitialAbilitySets();
	}
}

void AARPGEnemyCharacter::GrantInitialAbilitySets()
{
	// Grant ability sets
	FARPGAbilitySet_GrantedHandles Handles;
	for (const auto& AbilitySet : AbilitySets)
	{
		UARPGAbilitySet* SetToGrant = AbilitySet.Get();
		if (!IsValid(SetToGrant))
		{
			continue;
		}

		SetToGrant->GiveToAbilitySystem(AbilitySystemComponent, Handles, this);
	}
}

void AARPGEnemyCharacter::HandleCoreAttributeValueChanged(const FOnAttributeChangeData& Data)
{
	// Update the view model with new attribute values
	if (Data.Attribute == UARPGHealthAttributeSet::GetHealthAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy Core attribute changed: Health! %f"), Data.NewValue);
		// TODO: Update the health bar widget for this enemy
	}
	else if (Data.Attribute == UARPGHealthAttributeSet::GetHealthMaxAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy Core attribute changed: HealthMax! %f"), Data.NewValue);
		// TODO: Update the health bar widget for this enemy
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy Core attribute changed, but the attribute that changed was not handled."));
	}
}
