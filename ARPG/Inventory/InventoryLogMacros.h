// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Log, All);

#define INVENTORY_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogInventorySystem, Verbosity, Format, ##__VA_ARGS__); \
}

#define INVENTORY_LOG_VERBOSE(Format, ...)   INVENTORY_LOG(Verbose, Format, ##__VA_ARGS__)
#define INVENTORY_LOG_INFO(Format, ...)      INVENTORY_LOG(Log, Format, ##__VA_ARGS__)
#define INVENTORY_LOG_WARNING(Format, ...)   INVENTORY_LOG(Warning, Format, ##__VA_ARGS__)
#define INVENTORY_LOG_ERROR(Format, ...)     INVENTORY_LOG(Error, Format, ##__VA_ARGS__)

#define INVENTORY_LOG_SCREEN(Format, ...) \
{ \
    INVENTORY_LOG(Log, Format, ##__VA_ARGS__); \
    if (GEngine) \
    { \
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(Format, ##__VA_ARGS__)); \
    } \
}