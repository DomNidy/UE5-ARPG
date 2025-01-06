// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGAbilitySet.h"

UARPGAbilitySet::UARPGAbilitySet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UARPGAbilitySet::GiveToAbilitySystem(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject) const
{
	if (!ASC)
	{
		UE_LOGFMT(LogTemp, Log, "Tried to grant ability set, but the provided ASC was null.");
		return;
	}

	if (!ASC->IsOwnerActorAuthoritative())
	{
		UE_LOGFMT(LogTemp, Log, "Owner actor of the ASC was not authoritative, not granting the ability set.");
		return;
	}

	GrantAttributeSets(ASC, OutGrantedHandles);
	GrantGameplayEffects(ASC, OutGrantedHandles);
	GrantGameplayAbilities(ASC, OutGrantedHandles);
}

const TArray<FARPGAbilitySet_GameplayAbility>& UARPGAbilitySet::GetGameplayAbilities() const
{
	return GrantedGameplayAbilities;
}


void FARPGAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	AbilitySpecHandles.Add(Handle);
}

void FARPGAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	GameplayEffectHandles.Add(Handle);
}

void FARPGAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FARPGAbilitySet_GrantedHandles::TakeAbilityFromAbilitySystem(UARPGAbilitySystemComponent* ASC)
{
}


#if WITH_EDITOR
void UARPGAbilitySet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!PropertyChangedEvent.Property)
	{
		UE_LOGFMT(LogTemp, Log, "UARPGAbilitySet post edit change property handler ran, but property was null");
		return;
	}

	// Get name of property and ensure it exists in the ability set class, return if it doesn't
	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	if (PropertyName != GET_MEMBER_NAME_CHECKED(UARPGAbilitySet, GrantedGameplayAbilities))
	{
		UE_LOGFMT(LogTemp, Log, "UARPGAbilitySet post edit change property handler ran, but property name {0} was not equal to the checked property name", PropertyName);
		return;
	}

	if (GrantedGameplayAbilities.IsEmpty())
	{
		UE_LOGFMT(LogTemp, Log, "UARPGAbilitySet post edit change property handler ran, but granted gameplay abilities array is empty");
		return;
	}

	// Create copy of the granted abilities array, and clear current array in the ability set
	TArray<FARPGAbilitySet_GameplayAbility> AbilitiesCopy = GrantedGameplayAbilities;
	GrantedGameplayAbilities.Reset();
	GrantedGameplayAbilities.Reserve(AbilitiesCopy.Num());

	// Iterate over each gameplay ability in the copy
	for (int32 Index = 0; Index < AbilitiesCopy.Num(); ++Index)
	{
		FARPGAbilitySet_GameplayAbility& Ability = AbilitiesCopy[Index];

		// Assign input ID and add it back to the granted abilities array
		Ability.InputID = Index;
		GrantedGameplayAbilities.Add(Ability);
		UE_LOGFMT(LogTemp, Log, "Automatically assigned input id {0} to a gameplay ability in ability set {1}", Ability.InputID, GetName());
	}
}
#endif

void UARPGAbilitySet::GrantGameplayAbilities(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles) const
{
	check(ASC);

	// Add gameplay abilities
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FARPGAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOGFMT(LogTemp, Log, "Tried to grant a gameplay ability from an ability set, but the pointer to the underlying gameplay ability is invalid, skipping this one");
			continue;
		}

		// Get CDO of the ability
		UGameplayAbility* NewAbility = AbilityToGrant.Ability.GetDefaultObject();

		FGameplayAbilitySpec AbilitySpec(NewAbility, AbilityToGrant.AbilityLevel, AbilityToGrant.InputID);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);


		OutGrantedHandles.AddAbilitySpecHandle(AbilitySpecHandle);
	}
}

void UARPGAbilitySet::GrantGameplayEffects(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles) const
{
	check(ASC);

	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FARPGAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOGFMT(LogTemp, Log, "Tried to grant a gameplay effect from an ability set, but the pointer to the underlying gameplay effect is invalid, skipping this one.");
			continue;
		}

		// Get CDO for the gameplay effect
		UGameplayEffect* Effect = EffectToGrant.GameplayEffect.GetDefaultObject();
		if (!IsValid(Effect))
		{
			UE_LOGFMT(LogTemp, Log, "Tried to grant a gameplay effect, but the underlying CDO for the gameplay effect's class is invalid, skipping this one.");
			continue;
		}

		FGameplayEffectContextHandle EffectContextHandle;
		EffectContextHandle.AddInstigator(ASC->GetOwnerActor(), ASC->GetAvatarActor());

		FGameplayEffectSpec EffectSpec = FGameplayEffectSpec(EffectToGrant.GameplayEffect.GetDefaultObject(), EffectContextHandle, EffectToGrant.EffectLevel);

		FActiveGameplayEffectHandle ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(EffectSpec);

		UE_LOGFMT(LogTemp, Log, "Applied gameplay effect {0} to an ASC.", Effect->GetName());

		OutGrantedHandles.AddGameplayEffectHandle(ActiveEffectHandle);
	}

}
void UARPGAbilitySet::GrantAttributeSets(UARPGAbilitySystemComponent* ASC, FARPGAbilitySet_GrantedHandles& OutGrantedHandles) const
{
	check(ASC);

	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FARPGAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOGFMT(LogTemp, Log, "Tried to grant an invalid attribute set, skipping it.");
			continue;
		}

		bool SetAlreadyExists = IsValid(ASC->GetAttributeSet(SetToGrant.AttributeSet));
		if (SetAlreadyExists)
		{
			UE_LOGFMT(LogTemp, Log, "Tried to grant an attribute set to an ASC, but the ASC already has an attribute set of the same UClass. Skipping it.");
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet);
		ASC->AddAttributeSetSubobject(NewSet);

		UE_LOGFMT(LogTemp, Log, "Granted attribute set {0} to an ASC.", NewSet->GetName());

		OutGrantedHandles.AddAttributeSet(NewSet);
	}
}