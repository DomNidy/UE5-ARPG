// Fill out your copyright notice in the Description page of Project Settings.

#include "ARPGAbilityTask_PlayMontageAndWaitForEvent.h"
#include "GameFramework/Character.h"
#include "ARPGAbilitySystemComponent.h"

UARPGAbilityTask_PlayMontageAndWaitForEvent::
UARPGAbilityTask_PlayMontageAndWaitForEvent(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

// 1. We are gonna need to start playing animation montage
// 2. We are gonna need to bind to montage-related delegates to integrate animation with the ASC
// 3. We are gonna need to add a delegate to our ASC to listen for gameplay events, and then forward those events with our custom delegate
void UARPGAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
	if (!Ability)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability is null in Activate()"));
		return;
	}

	bool bIsLocallyControlled = Ability->IsLocallyControlled();
	
	UARPGAbilitySystemComponent* ASC = GetTargetASC();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UARPGAbilityTask_PlayMontageAndWaitForEvent call failed. ASC was null ptr"));
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("UARPGAbilityTask_PlayMontageAndWaitForEvent call failed. AnimInstance was nullptr"));
		return;
	}

	// Bind to event callback
	EventHandle = ASC->AddGameplayEventTagContainerDelegate(
		EventTags,
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
			this, &UARPGAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent)
	);

	// Start playing the animation montage
	bool bPlayedMontage = ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f;
	
	// If we failed to play the montage, log out error and broadcast cancel delegate
	if (!bPlayedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("URPGAbilityTask_PlayMontageAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
		return;
	}

	// Check if the task should continue
	if (!ShouldBroadcastAbilityTaskDelegates())
	{
		return;
	}

	// Bind to ability cancelled delegate
	CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UARPGAbilityTask_PlayMontageAndWaitForEvent::OnAbilityCancelled);

	// Bind to anim montage blend out delegate
	BlendingOutDelegate.BindUObject(this, &UARPGAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

	// Bind to anim montage end delegate
	MontageEndedDelegate.BindUObject(this, &UARPGAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

	// Update root motion properties if necessary
	// Actor authority changes how actor updates are simulated inbetween updates (from authority).
	// When the local role of the character is ROLE_AutonomousProxy, 
	//	that means the character is possessed by the local player controller
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && (Character->GetLocalRole() == ENetRole::ROLE_Authority ||
		(Character->GetLocalRole() == ENetRole::ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
	{
		Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
	}

	SetWaitingOnAvatar();

	return;
}

void UARPGAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
	UAbilitySystemComponent* ASC = GetTargetASC();

	check(ASC);

	OnAbilityCancelled();
	Super::ExternalCancel();
}


void UARPGAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	UARPGAbilitySystemComponent* ASC = GetTargetASC();
	if (ASC)
	{
		ASC->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

UARPGAbilityTask_PlayMontageAndWaitForEvent*
UARPGAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	FGameplayTagContainer EventTags,
	UAnimMontage* MontageToPlay,
	float Rate,
	FName StartSection,
	bool bStopWhenAbilityEnds,
	float AnimRootMotionTranslationScale,
	float StartTimeSeconds,
	bool bAllowInterruptAfterBlendOut)
{
	UARPGAbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<UARPGAbilityTask_PlayMontageAndWaitForEvent>(
		OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

	return MyObj;
}

UARPGAbilitySystemComponent*
UARPGAbilityTask_PlayMontageAndWaitForEvent::GetTargetASC() const
{
	return Cast<UARPGAbilitySystemComponent>(AbilitySystemComponent);
}

bool UARPGAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UAbilitySystemComponent* ASC = GetTargetASC();

	if (ASC && Ability)
	{
		if (AbilitySystemComponent->GetAnimatingAbility() == Ability
			&& AbilitySystemComponent->GetCurrentMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			AbilitySystemComponent->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

void UARPGAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent->ClearAnimatingAbility(Ability);

			// Reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}

		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UARPGAbilityTask_PlayMontageAndWaitForEvent::OnAbilityCancelled()
{
	if (StopPlayingMontage())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UARPGAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage*,
	bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void UARPGAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(
	FGameplayTag EventTag,
	const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload; // copy data so we don't worry about it being freed unexpectedly after broadcasting
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}

FString
UARPGAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}