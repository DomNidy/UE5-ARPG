// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemData.h"

FString UItemData::GetDebugString() const
{
    FString DebugString;

    DebugString += FString::Printf(TEXT("Item: %s\n"), *ItemDisplayName.ToString());
    DebugString += FString::Printf(TEXT("ID: %s\n"), *ItemId.ToString());

    // Add tags if any exist
    if (ItemTypeTags.Num() > 0)
    {
        DebugString += TEXT("Tags: ");
        FString TagString;
        for (const FGameplayTag& Tag : ItemTypeTags)
        {
            if (!TagString.IsEmpty())
            {
                TagString += TEXT(", ");
            }
            TagString += Tag.ToString();
        }
        DebugString += TagString + TEXT("\n");
    }

    // Add description if it exists
    if (!ItemDescription.IsEmpty())
    {
        DebugString += FString::Printf(TEXT("Description: %s\n"), *ItemDescription.ToString());
    }

    // Add icon information
    DebugString += FString::Printf(TEXT("Has Icon: %s"), Icon != nullptr ? TEXT("Yes") : TEXT("No"));

    return DebugString;
}

FString UEquipmentData::GetDebugString() const
{
    // First get the base class debug string
    FString DebugString = Super::GetDebugString();

    // Add equipment-specific information
    DebugString += TEXT("\n--- Equipment Specific Data ---\n");

    // Add mesh information
    DebugString += FString::Printf(TEXT("Mesh: %s\n"),
        Mesh.IsValid() ? *Mesh.ToSoftObjectPath().ToString() : TEXT("None"));

    // Add ability set information
    DebugString += FString::Printf(TEXT("Ability Set: %s"),
        AbilitySet != nullptr ? *AbilitySet->GetName() : TEXT("None"));

    return DebugString;
}
