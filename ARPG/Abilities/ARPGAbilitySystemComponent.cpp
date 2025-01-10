// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGAbilitySystemComponent.h"
#include "ARPGAbility.h"
#include "Logging/StructuredLog.h"
#include "ARPG/Core/ARPGNativeGameplayTags.h"

UARPGAbilitySystemComponent::UARPGAbilitySystemComponent()
{
}


void UARPGAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag AbilityInputTag)
{

	if (AbilityInputTag.IsValid())
	{

		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(AbilityInputTag))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UARPGAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag AbilityInputTag)
{
	if (AbilityInputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{

			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(AbilityInputTag))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void LogASCLocalAuthority(UARPGAbilitySystemComponent* ASC)
{
	// Get the network role of the ASC's owner on the LOCAL MACHINE (local machine = the where this code was executed at)
	// For example, if the ASC is owned by the PlayerState, and this is executed on the client, since the player state is server authoratative
	// we should expect this to not return an authoritative role. But when this runs on the server, it should.
	TEnumAsByte<ENetRole> RoleOfOwnerLocally = ASC->GetOwnerRole();
	FText RoleOfOwnerLocallyText;
	UEnum::GetDisplayValueAsText(RoleOfOwnerLocally, RoleOfOwnerLocallyText);

	// Look at the local owner actor, and check what network mode the local UWorld is running under
	// When executed on a client that, we should expected NM_Client
	// When executed on a server, we can get any of NM_ListenServer, NM_DedicatedServer, or NM_Standalone
	ENetMode WorldNetModeLocally = ASC->GetOwner()->GetWorld()->GetNetMode();

	UE_LOGFMT(LogTemp, Log, "ASC Networking/Authority related info:");
	UE_LOGFMT(LogTemp, Log, "\t Local Role of ASC Owner: {0}", RoleOfOwnerLocallyText.ToString());
	UE_LOGFMT(LogTemp, Log, "\t Local UWorld NetMode: {0} (Standalone: {1}, Dedicated Server: {2}, Client: {3})", uint32(WorldNetModeLocally), uint32(ENetMode::NM_Standalone), uint32(ENetMode::NM_DedicatedServer), uint32(ENetMode::NM_Client));
}

void UARPGAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// This method should only be run on the client
	check(GetNetMode() != NM_DedicatedServer)

	if (HasMatchingGameplayTag(Status_Block_AbilityInput))
	{
		ClearAbilityInput();
		return;
	}

	// Array of ability specs that are "queued" to be activated.
	// At the end of this function, we will call TryActivateAbility on all specs in this array
	// TODO: We could probably implement an abstraction here instead of using an array for clarity.
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// Process all abilities that activate when the input is held
	// The main purpose of this:
	//	 1. We need to find the ability spec associated with the spec handle
	//   2. We need to make sure the spec points to a valid ability, and that it's not active already
	//   3. Need to make sure the activation policy allows the ability to be activated while it's input is held down
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
	
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UARPGAbility* AbilityCDO = CastChecked<UARPGAbility>(AbilitySpec->Ability);

				if (AbilityCDO->GetActivationPolicy() == EARPGAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// Process all abilities that had their input pressed this frame
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					UE_LOGFMT(LogTemp, Log, "Already active...");
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					// Check if this ability should activate when input is triggered
					const UARPGAbility* AbilityCDO = CastChecked<UARPGAbility>(AbilitySpec->Ability);

					if (AbilityCDO->GetActivationPolicy() == EARPGAbilityActivationPolicy::OnInputPressed)
					{
						UE_LOGFMT(LogTemp, Log, "InputPressedSpecHandles - Added handle for ability {0}", AbilityCDO->GetName());
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}

			}
		}
	}

	// Try to activate all abilities
	while (AbilitiesToActivate.Num() > 0)
	{
		FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitiesToActivate.Pop(EAllowShrinking::No);

		if (AbilitySpecHandle.IsValid())
		{
			TryActivateAbility(AbilitySpecHandle);
		}
	}

	// Shrink array only after we've popped everything
	AbilitiesToActivate.Shrink();

	// Process all abilities that had their input released this frame
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : InputReleasedSpecHandles)
	{
		UE_LOGFMT(LogTemp, Log, "\t This ability spec had it's input released this frame: {0}", AbilitySpecHandle.ToString());

		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			UE_LOGFMT(LogTemp, Log, "\t Found ability spec from the handle, it's ability is: {0}", AbilitySpec->GetDebugString());

			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					UE_LOGFMT(LogTemp, Log, "\t It was active, so we're calling AbilitySpecInputReleased on it");

					AbilitySpecInputReleased(*AbilitySpec);
				}
				else
				{
					UE_LOGFMT(LogTemp, Log, "\t Ability was already inactive, no need to call AbilitySpecInputReleased on it");
				}
			}
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}


void UARPGAbilitySystemComponent::ClearAbilityInput()
{
	InputHeldSpecHandles.Reset();
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}
