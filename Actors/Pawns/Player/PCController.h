// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSPlayerController.h"
#include "../../../RealtimeWorld/WorldGrid.h"
#include "PCController.generated.h"

/**
 *
 */
UCLASS()
class TERRSHALOM_API APCController : public AALSPlayerController
{
	GENERATED_BODY()
public:
	const int16 MAP_CHUNKS = 16;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interact|Action")
	void traceAction(FString type);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Generation|Init")
	void Initialize(AWorldGrid *map, AWorldBuilder *chunk, FVector offset);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Generation|Init")
	void ServerToClient(AWorldGrid *map, AWorldBuilder *chunk, FVector offset);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Generation|Init")
	void Interact(AWorldBuilder *chunk, int32 faceIndex, float strength, bool isAdd);

	 UFUNCTION(BlueprintCallable, Category = "Generation|Box") // Server, Reliable, WithValidation
	 void AppendBoxMesh(AWorldBuilder *chunk, FVector BoxRadius, FTransform BoxTransform, FVector offset);
};
