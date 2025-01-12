// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "CoreMinimal.h"
#include "UARPGAbilityTask_PlayMontageAndWaitForEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FARPGPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * Reference: https://github.com/vahabahmadvand/ActionRPG_UE53/blob/main/Source/ActionRPG/Public/Abilities/RPGAbilityTask_PlayMontageAndWaitForEvent.h
 */
UCLASS()
class ARPG_API UARPGAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask {
public:
	GENERATED_BODY()

	UARPGAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/**
	 * @brief The montage completely finished playing
	 */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageAndWaitForEventDelegate OnCompleted;

	/**
	 * @brief The montage started blending out
	 */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageAndWaitForEventDelegate OnBlendOut;

	/**
	 * @brief The montage was interrupted
	 */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageAndWaitForEventDelegate OnInterrupted;

	/**
	 * @brief The ability task was explicitly cancelled by another ability
	 */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageAndWaitForEventDelegate OnCancelled;

	/**
	 * @brief One of the triggering gameplay events happened
	 */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageAndWaitForEventDelegate EventReceived;

	/**
	 * @brief Start playing an animation montage and wait for it to end.
	 *	If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
	 *
	 * @param TaskInstanceName Set to override the name of this task, for later querying
	 * @param EventTags Any gameplay events matching this tag will activate the OnEventReceived callback. If empty, all events will trigger callback
	 * @param MontageToPlay  The montage to play on the character (which is the avatar actor of the owning ability)
	 * @param Rate  Change to play the montage faster or slower
	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
	 * @return
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks|ARPG",
		meta = (DisplayName = "PlayMontageAndWaitForEvent", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
	static UARPGAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility, FName TaskInstanceName, FGameplayTagContainer EventTags, UAnimMontage* MontageToPlay, float Rate = 1.f, FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true, float AnimRootMotionTranslationScale = 1.f, float StartTimeSeconds = 0.f, bool bAllowInterruptAfterBlendOut = false);

	/** Return the ASC of the ability's owner */
	class UARPGAbilitySystemComponent* GetTargetASC() const;

protected:
	UPROPERTY()
	UAnimMontage* MontageToPlay;

	/**
	 * @brief Container of gameplay tags which we will listen for while playing the animation montage
	 */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/**
	 * @brief Playback rate of animation montage
	 */
	UPROPERTY()
	float Rate;

	/** Section to start the montage from */
	UPROPERTY()
	FName StartSection;

	/** Modifies how root motion movement to apply */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** Should montage be aborted when the ability ends? */
	UPROPERTY()
	bool bStopWhenAbilityEnds;

	/** Stops currently playing montage and unbinds montage-related delegates, returns true if a montage was stopped, false if not. */
	bool StopPlayingMontage();

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled();
	void OnMontageEnded(UAnimMontage*, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
