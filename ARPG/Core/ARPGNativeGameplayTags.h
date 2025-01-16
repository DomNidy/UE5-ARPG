

#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

/**
 * Singleton containing native gameplay tags
 */

UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_MeleeBasic);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Roll);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Block_AbilityInput)

/**
 * Tags for equipment types
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Melee_1H)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Melee_2H)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Ring)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Trinket)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Helmet)


struct FARPGNativeGameplayTags
{
	static const FARPGNativeGameplayTags& Get() { return GameplayTags; }
private:
	static FARPGNativeGameplayTags GameplayTags;
};