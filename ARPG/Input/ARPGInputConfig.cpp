// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGInputConfig.h"


const UInputAction* UARPGInputConfig::FindInputActionForInputTag(const FGameplayTag& InputTag) const
{

	for (const FTaggedInputAction& TaggedInputAction : TaggedInputActions)
	{
		if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}

	return nullptr;
}
