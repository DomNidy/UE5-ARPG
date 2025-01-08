// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGAbilitySystemComponent.h"
#include "ARPGAbility.h"
#include "Logging/StructuredLog.h"
#include "ARPG/Core/ARPGNativeGameplayTags.h"

UARPGAbilitySystemComponent::UARPGAbilitySystemComponent()
{
}

void UARPGAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	UE_LOGFMT(LogTemp, Log, "AbilitySpecInputPressed - {0}", Spec.GetDebugString());
	if (Spec.IsActive())
	{
		if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
				return;
		}
		else
		{
			for (const UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances())
			{
				// TODO: Is using the prediction key like this incorrect? Notice we are using the same one for current and original prediction key
				FPredictionKey AbilityPredictionKey = AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey();

				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, AbilityPredictionKey, AbilityPredictionKey);
			}
		}
	}
}

void UARPGAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
				return;
		}
		else
		{
			for (const UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances())
			{
				// TODO: Is using the prediction key like this incorrect? Notice we are using the same one for current and original prediction key
				FPredictionKey AbilityPredictionKey = AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey();

				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, AbilityPredictionKey, AbilityPredictionKey);
			}
		}
	}
}
void UARPGAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{

	if (InputTag.IsValid())
	{

		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{

			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UARPGAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	UE_LOGFMT(LogTemp, Log, "AbilityInputTagReleased - {0}", InputTag.GetTagName());

	if (InputTag.IsValid())
	{

		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{

			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				UE_LOG(LogTemp, Log, TEXT("Removing the handles!"));
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}

}

void UARPGAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// Get the network role of the ASC's owner on the LOCAL MACHINE (local machine = the where this code was executed at)
	// For example, if the ASC is owned by the PlayerState, and this is executed on the client, since the player state is server authoratative
	// we should expect this to not return an authoritative role. But when this runs on the server, it should.
	TEnumAsByte<ENetRole> RoleOfOwnerLocally = GetOwnerRole();
	FText RoleOfOwnerLocallyText;
	UEnum::GetDisplayValueAsText(RoleOfOwnerLocally, RoleOfOwnerLocallyText);

	// Look at the local owner actor, and check what network mode the local UWorld is running under
	// When executed on a client that, we should expected NM_Client
	// When executed on a server, we can get any of NM_ListenServer, NM_DedicatedServer, or NM_Standalone
	ENetMode WorldNetModeLocally = GetOwnerActor()->GetWorld()->GetNetMode();

	UE_LOGFMT(LogTemp, Log, "ASC Process Input:");
	UE_LOGFMT(LogTemp, Log, "\t Local Role of ASC Owner: {0}", RoleOfOwnerLocallyText.ToString());
	UE_LOGFMT(LogTemp, Log, "\t Local UWorld NetMode: {0} (Standalone: {1}, Dedicated Server: {2}, Client: {3})", uint32(WorldNetModeLocally), uint32(ENetMode::NM_Standalone), uint32(ENetMode::NM_DedicatedServer), uint32(ENetMode::NM_Client));

	if (HasMatchingGameplayTag(Status_Block_AbilityInput))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// Process all abilities that activate when the input is held
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			// If the ability isn't active
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{

				// Check if this ability should activate when input is held
				const UARPGAbility* AbilityCDO = CastChecked<UARPGAbility>(AbilitySpec->Ability);

				if (AbilityCDO->GetActivationPolicy() == EARPGAbilityActivationPolicy::WhileInputActive)
				{
					UE_LOGFMT(LogTemp, Log, "InputHeldSpecHandles - Added handle for ability {0}", AbilityCDO->GetName());
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
	for (FGameplayAbilitySpecHandle AbilitySpecHandle : AbilitiesToActivate)
	{
		if (AbilitySpecHandle.IsValid())
		{
			TryActivateAbility(AbilitySpecHandle);
		}
	}

	// Process all abilities that had their input released this frame
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputReleased(*AbilitySpec);
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
