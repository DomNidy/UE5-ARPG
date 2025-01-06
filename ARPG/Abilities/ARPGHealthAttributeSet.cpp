// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGHealthAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Logging/StructuredLog.h"
#include "ARPG/Core/ARPGViewModelPlayerStats.h"
#include "MVVMGameSubsystem.h"

UARPGHealthAttributeSet::UARPGHealthAttributeSet()
{

}

// Client only - This is not invoked when playing in standalone mode
void UARPGHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GetOwningAbilitySystemComponent()->SetBaseAttributeValueFromReplication(GetHealthAttribute(), GetHealth(), OldHealth.GetCurrentValue());
	UE_LOG(LogTemp, Log, TEXT("On rep health: %f, server?: %s"), GetHealth(), GetOwningActor()->GetNetMode() == NM_DedicatedServer ? TEXT("True") : TEXT("False"));

}

// Client only - This is not invoked when playing in standalone mode
void UARPGHealthAttributeSet::OnRep_HealthMax(const FGameplayAttributeData& OldHealthMax)
{
	GetOwningAbilitySystemComponent()->SetBaseAttributeValueFromReplication(GetHealthMaxAttribute(), GetHealthMax(), OldHealthMax.GetCurrentValue());
	UE_LOG(LogTemp, Log, TEXT("On rep max health: %f, server?: %s"), GetHealthMax(), GetOwningActor()->GetNetMode() == NM_DedicatedServer ? TEXT("True") : TEXT("False"));
}

void UARPGHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UARPGHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealingAttribute())
	{
		const float HealingDone = NewValue;

		if (HealingDone > 0.0f)
		{
			SetHealing(0.0f);

			const float NewHealth = FMath::Clamp(GetHealth() + HealingDone, 0.0f, GetHealthMax());

			SetHealth(NewHealth);
		}
	}
	else if (Attribute == GetDamageAttribute())
	{

		const float DamageDone = NewValue;

		if (DamageDone > 0.0f)
		{
			SetDamage(0.0f);

			const float NewHealth = FMath::Clamp(GetHealth() - DamageDone, 0.0f, GetHealthMax());

			SetHealth(NewHealth);
		}
	}
}

void UARPGHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UARPGHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARPGHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARPGHealthAttributeSet, HealthMax, COND_None, REPNOTIFY_Always);
}
