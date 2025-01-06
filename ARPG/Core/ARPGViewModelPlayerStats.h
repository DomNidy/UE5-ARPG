// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ARPGViewModelPlayerStats.generated.h"

/**
 *
 */
UCLASS()
class ARPG_API UARPGViewModelPlayerStats : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UARPGViewModelPlayerStats();

	// ------------------------------
	// Health variables
	// ------------------------------
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealth() const;

	UFUNCTION(BlueprintCallable)
	void SetHealth(const float& NewHealth);

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealthMax() const;

	UFUNCTION(BlueprintCallable)
	void SetHealthMax(const float& NewHealthMax);

	// ------------------------------
	// Data transformations
	// ------------------------------
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealthPercentage() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetHealthTextDisplay() const;

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float Health = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float HealthMax = 0.0f;
};
