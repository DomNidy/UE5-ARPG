// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventorySystemGlobals.generated.h"

/**
 * Global configuration properties for how the inventory system should function.
 */
UCLASS()
class ARPG_API UInventorySystemGlobals : public UObject
{
	GENERATED_BODY()

	static UInventorySystemGlobals& Get()
	{
		if (InventorySystemGlobals == nullptr)
		{
			// Should we really use GEngine as outer? Would UWorld make more sense?
			InventorySystemGlobals = NewObject<UInventorySystemGlobals>(GEngine);
		}

		return *InventorySystemGlobals;
	}

private:
	static UInventorySystemGlobals* InventorySystemGlobals;
};