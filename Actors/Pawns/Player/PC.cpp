// Fill out your copyright notice in the Description page of Project Settings.


#include "PC.h"

// Sets default values
//APC::APC()
//{
//	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//}

// Called when the game starts or when spawned
void APC::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//FHitResult hitResult;
	//bool isHit = GetHitResultAtScreenPosition(FVector2D(300, 240)/*gameMgr->center*/, ECollisionChannel::ECC_WorldStatic, false, hitResult);
	//if (isHit) {
	//	if (GetPawn<APC>()) GetPawn<APC>()->setHitRay(&hitResult, isPlace);
	//}
}

