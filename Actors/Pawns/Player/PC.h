// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSCharacter.h"
#include "PC.generated.h"

/**
 * 
 */
UCLASS()
class TERRSHALOM_API APC : public AALSCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	//APC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
