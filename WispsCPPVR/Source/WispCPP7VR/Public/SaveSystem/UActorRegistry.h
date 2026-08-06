// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UActorRegistry.generated.h"

/**
 * 3. The Registry (UActorRegistry)
Class Type: Inherits from UGameInstanceSubsystem

Location: Public/Core/ (or SaveSystem/)

Purpose: The "Address Book." It keeps a live Map of every tree/butterfly in the garden. Without this, the SaveManager would have to "search" the whole world every time you save, which is slow and buggy.
 */
UCLASS()
class WISPCPP7VR_API UUActorRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
};
