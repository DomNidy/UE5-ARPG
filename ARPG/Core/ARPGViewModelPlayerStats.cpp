// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGViewModelPlayerStats.h"

UARPGViewModelPlayerStats::UARPGViewModelPlayerStats()
{
}

float UARPGViewModelPlayerStats::GetHealth() const
{
	return Health;
}

void UARPGViewModelPlayerStats::SetHealth(const float& NewHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, NewHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercentage);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthTextDisplay);
	}
}

float UARPGViewModelPlayerStats::GetHealthMax() const
{
	return HealthMax;
}

void UARPGViewModelPlayerStats::SetHealthMax(const float& NewHealthMax)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(HealthMax, NewHealthMax))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercentage);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthTextDisplay);
	}
}

float UARPGViewModelPlayerStats::GetHealthPercentage() const
{
	return (HealthMax != 0.f) ? (Health / HealthMax) : 0.f;
}

FText UARPGViewModelPlayerStats::GetHealthTextDisplay() const
{
	FTextFormat HealthDisplayTextFormat = FTextFormat::FromString(TEXT("{0}/{1}"));

	int32 RoundedHealth = FMath::RoundToInt(Health);
	int32 RoundedHealthMax = FMath::RoundToInt(HealthMax);

	return FText::Format(HealthDisplayTextFormat, FText::AsNumber(RoundedHealth), FText::AsNumber(RoundedHealthMax));
}
