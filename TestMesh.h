// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestMesh.generated.h"

UCLASS()
class TERRSHALOM_API ATestMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestMesh();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//	class URealtimeMeshComponent* RealtimeMesh;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* baseMesh;

};
