// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSPlayerController.h"
#include "../../../RealtimeWorld/Chunker_PMC.h"
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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interact|Action")
	void traceAction(FString type);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Generation|Init")
		void Initialize(AWorldGrid* map, UChunker_PMC* chunk, FVector offset);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Generation|Init")
		void ServerToClient(AWorldGrid* map, UChunker_PMC* chunk, FVector offset);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Generation|Init")
		void Interact(UChunker_PMC* chunk, int32 faceIndex, float strength, bool isAdd);

	UFUNCTION(BlueprintCallable, Category = "Generation|Box") // Server, Reliable, WithValidation
		void AppendBoxMesh(UChunker_PMC* chunk, FVector BoxRadius, FTransform BoxTransform);
};
